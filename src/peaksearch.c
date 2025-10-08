/******************************************************************************\
 _____ _____ _____         _   _ _ _ _   _
|  ___|  ___|_   _|  _   _| |_(_) (_) |_(_) ___  ___
| |_  | |_    | |   | | | | __| | | | __| |/ _ \/ __|
|  _| |  _|   | |   | |_| | |_| | | | |_| |  __/\__ \
|_|   |_|     |_|    \__,_|\__|_|_|_|\__|_|\___||___/

==============================================================================
 File:         peaksearch.c
 Timestamp:    2008-Jan-22
 Author:       Paolo Bosetti <paolo.bosetti@unitn.it>
 Organization: Universiy of Trento - https://unitn.it
 LICENSE:      Copyright (c) 2008 Paolo Bosetti

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without
 restriction, including without limitation the rights to use,
 copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the
 Software is furnished to do so, subject to the following
 conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 OTHER DEALINGS IN THE SOFTWARE.
\******************************************************************************/

#include "fft.h"
#include <stdio.h>
#define FULL    0
#define PARTIAL 1

typedef struct statistics {
  data_t  mean;
  data_t  sd;
  data_t  max;
  index_t max_idx;
} statistics_t;

static data_t mean(fft_data_t * const d, index_t start, index_t end) {
  data_t sum = 0.;
  index_t i, c;
  for (i = start, c = 0; i < end; i++, c++) {
    sum += fft_x(d)[i];
  }
  return (sum / c );
}

static void compute_stats(fft_data_t * const d, index_t start, char type, statistics_t * const stat) {
  data_t acc = 0.;
  index_t i, c, end;
  switch (type) {
    case FULL:
      end = fft_n(d);
      break;
    case PARTIAL:
      end = start + fft_win_size(d) <= fft_n(d) ? start + fft_win_size(d) : fft_n(d);
      break;
  }
  stat->mean = mean(d, start, end);
  stat->max  = 0.;
  for (i = start, c = 0; i < end; i++, c++) {
    acc += pow((fft_x(d)[i] - stat->mean), 2);
    if (fft_x(d)[i] > stat->max) {
      stat->max     = fft_x(d)[i];
      stat->max_idx = i;
    }
  }
  stat->sd = sqrt(acc / (c - 1));
}

index_t fft_search_peaks(fft_data_t * const d, index_t max_peaks) {
  index_t count   = 0;
  FILE *of = NULL;
  // index_t peaks_s = CHUNK_SIZE;
  statistics_t stat;

  if (fft_realloc_peaks(d, max_peaks) == NULL) {
    fprintf(stderr, "Memory allocation error in peaksearch\n");
    return 0;
  }
  if (fft_output_file(d)) {
    of = fopen(fft_output_file(d), "w");
    fprintf(stderr, "Writing data to %s\n", fft_output_file(d));
  }
  if (of) fprintf(of, "i\tfft\tw\trw\tc\n");

  compute_stats(d, 0, FULL, &stat);
  fft_set_stdev(d, stat.sd);

  index_t i;
  char in_cluster = 0;

  data_t max = 0.;
  // run only on the first half of the FFT (it is symmetric!)
  for(i = 0; i < ((fft_n(d) / 2) - fft_win_size(d)); i++) {
    compute_stats(d, i, PARTIAL, &stat);
    if (stat.sd > fft_nsigma(d) * fft_stdev(d)) {
      if (stat.max > max && fft_peaks(d)[count-1] != stat.max_idx) {
        fft_peaks(d)[count] = stat.max_idx;
        max = stat.max;
      }
      in_cluster = 1;
    }
    else {
      if(in_cluster == 1) count++;
      if(count > max_peaks) {
        break;
        // peaks_s += CHUNK_SIZE;
        // fft_peaks(d) = (index_t*) realloc(fft_peaks(d), peaks_s * sizeof(index_t));
      }
      max = 0.;
      in_cluster = 0;
    }
    if (of) fprintf(of, "%d\t%f\t%f\t%f\t%d\n", i, fft_x(d)[i], stat.sd, stat.sd / fft_stdev(d), in_cluster);
  }
  if (of) fclose(of);
  fft_set_npeaks(d, count);
  return count;
}
