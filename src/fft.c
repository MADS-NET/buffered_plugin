/******************************************************************************\
 _____ _____ _____         _   _ _ _ _   _
|  ___|  ___|_   _|  _   _| |_(_) (_) |_(_) ___  ___
| |_  | |_    | |   | | | | __| | | | __| |/ _ \/ __|
|  _| |  _|   | |   | |_| | |_| | | | |_| |  __/\__ \
|_|   |_|     |_|    \__,_|\__|_|_|_|\__|_|\___||___/

==============================================================================
 File:         fft.c
 Timestamp:    2008-Jan-30
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

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "fft.h"

const data_t PI2 = 2 * M_PI;

typedef struct fft_data {
  int processed;
  data_t freq;    // sampling frequency in seconds
  index_t n;      // sample size: power of 2
  data_t *x, *y;  // input data
  data_t *t;      // times
  data_t *f;      // freqs
  data_t mean[2];
  data_t sd[2];
  index_t head;
  // Peaksearch
  data_t   nsigma;      // number of nsigma defining the threshold
  index_t  win_size;    // moving windows size
  data_t   stdev;       // search_peaks() will put here the overal st.dev.
  index_t *peaks;       // array of found peaks
  index_t  n_peaks;     // number of found peaks
  char    *output_file; // debug output file (not used if NULL)
} fft_data_t;

void hamming(data_t x[], data_t bias, index_t n) {
  index_t i;
  const data_t alpha = 0.46;
  for (i = 0; i < n; i++) {
    x[i] -= bias;
    x[i] *= alpha - (1 - alpha) * cos(PI2 * i / (n - 1));
  }
}

void hann(data_t x[], data_t bias, index_t n) {
  index_t i;
  for (i = 0; i < n; i++) {
    x[i] -= bias;
    x[i] *= 0.5 * (1 - cos(PI2 * i / (n - 1)));
  }
}

void blackmann(data_t x[], data_t bias, index_t n) {
  index_t i;
  const data_t alpha = 0.16;
  const data_t a0 = (1 - alpha) / 2;
  const data_t a1 = 0.5;
  const data_t a2 = alpha / 2;
  for (i = 0; i < n; i++) {
    x[i] -= bias;
    x[i] *= a0 - a1 * cos(PI2 * i / (n - 1)) + a2 * cos(2 * PI2 * i / (n - 1));
  }
}

fft_data_t *fft_init(index_t power, data_t freq) {
  index_t i;
  index_t n = pow(2, power);
  fft_data_t *data = (fft_data_t *)malloc(sizeof(fft_data_t));
  if (data == NULL) {
    perror("fft_data malloc error");
    exit(EXIT_FAILURE);
  }
  memset(data, 0, sizeof(fft_data_t));
  data->freq = freq;
  data->n = n;
  data->x = (data_t *)malloc(n * sizeof(data_t));
  data->y = (data_t *)malloc(n * sizeof(data_t));
  data->t = (data_t *)malloc(n * sizeof(data_t));
  data->f = (data_t *)malloc(n * sizeof(data_t));
  data->peaks = (index_t *)malloc(INITIAL_N_PEAKS * sizeof(index_t));
  data->output_file = NULL;
  if (data->x == NULL || data->y == NULL || data->t == NULL ||
      data->f == NULL || data->peaks == NULL) {
    perror("data malloc error");
    exit(EXIT_FAILURE);
  }
  for (i = 0; i < n; i++) {
    data->t[i] = i / data->freq;
    data->f[i] = data->freq / n * i;
  }
  fft_reset(data);
  assert(data != NULL);
  return data;
}

void fft_reset(fft_data_t *data) {
  memset(data->x, 0, data->n * sizeof(data_t));
  memset(data->y, 0, data->n * sizeof(data_t));
  memset(data->mean, 0, 2 * sizeof(data_t));
  data->processed = 0; // FFT NOT DONE YET!
  memset(data->sd, 0, 2 * sizeof(data_t));
  data->head = 0;
}

index_t *fft_realloc_peaks(fft_data_t *d, size_t n) {
  d->peaks = (index_t*) realloc(fft_peaks(d), n * sizeof(index_t));
  return d->peaks;
}

void fft_free(fft_data_t *d) {
  assert(d != NULL);
  // free(d->x);
  // free(d->y);
  // free(d->t);
  free(d->f);
  free(d->peaks);
  if (d->output_file)
    free(d->output_file);
  free(d);
}

static void fft(data_t x[], data_t y[], index_t n) {
  int m = (int)(log(n) / log(2));
  int i, j, k, n1, n2;
  data_t c, s, e, a, t1, t2;

  j = 0; /* bit-reverse */
  n2 = n / 2;
  for (i = 1; i < n - 1; i++) {
    n1 = n2;
    while (j >= n1) {
      j = j - n1;
      n1 = n1 / 2;
    }
    j = j + n1;

    if (i < j) {
      t1 = x[i];
      x[i] = x[j];
      x[j] = t1;
      t1 = y[i];
      y[i] = y[j];
      y[j] = t1;
    }
  }

  n1 = 0; /* FFT */
  n2 = 1;

  for (i = 0; i < m; i++) {
    n1 = n2;
    n2 = n2 + n2;
    e = -PI2 / n2;
    a = 0.0;

    for (j = 0; j < n1; j++) {
      c = cos(a);
      s = sin(a);
      a = a + e;

      for (k = j; k < n; k = k + n2) {
        t1 = c * x[k + n1] - s * y[k + n1];
        t2 = s * x[k + n1] + c * y[k + n1];
        x[k + n1] = x[k] - t1;
        y[k + n1] = y[k] - t2;
        x[k] = x[k] + t1;
        y[k] = y[k] + t2;
      }
    }
  }
  return;
}

// operate an in-place transform from rectangilar to polar coords
void to_polar(data_t *const x, data_t *const y) {
  assert(x != NULL && y != NULL);
  data_t m, p;
  m = sqrt(pow(*x, 2) + pow(*y, 2));
  p = atan2(*y, *x);
  *x = m;
  *y = p;
}

// Polar version: returns modulus and phase
static void polar_fft(fft_data_t *const d) {
  int i;
  fft(d->x, d->y, d->n);
  for (i = 0; i < d->n; i++) {
    to_polar(&d->x[i], &d->y[i]);
  }
  d->processed = 1;
}

void fft_apply_window(fft_data_t *const d, fft_windowing win) {
  win(d->x, 0.0, d->n);
  win(d->y, 0.0, d->n);
}

void fft_apply_window_and_bias(fft_data_t *const d, fft_windowing win) {
  win(d->x, d->mean[0], d->n);
  win(d->y, d->mean[1], d->n);
}

index_t fft_calc_spectrum(fft_data_t *const d) {
  index_t n = d->n / 2;
  if (d->processed == 0)
    polar_fft(d);
  return n;
}

int fft_add_point(fft_data_t *const d, data_t x, data_t y) {
  const index_t n = d->head + 1;
  d->x[n] = x;
  d->y[n] = y;
  if (n <= 1) { // recursion formula: first element (base-1)
    d->mean[0] = x;
    d->mean[1] = y;
    d->sd[0] = 0;
    d->sd[1] = 0;
  } else {
    const data_t n1 = n - 1;
    const data_t n2 = n - 2;
    const data_t nr = 1.0 / n;
    const data_t n1r = 1.0 / n1;
    const data_t nn1 = n / n1;
    d->mean[0] = nr * (n1 * d->mean[0] + x);
    d->mean[1] = nr * (n1 * d->mean[1] + y);
    d->sd[0] = sqrt(n1r *
                    (n2 * pow(d->sd[0], 2) + nn1 * pow(d->mean[0] - x, 2)));
    d->sd[1] = sqrt(n1r *
                    (n2 * pow(d->sd[1], 2) + nn1 * pow(d->mean[1] - y, 2)));
  }
  d->head++;
  if (d->head >= d->n)
    return 0;
  else
    return 1;
}


data_t *fft_x(const fft_data_t *fft) { return fft->x; }
data_t *fft_y(const fft_data_t *fft) { return fft->y; }
data_t *fft_t(const fft_data_t *fft) { return fft->t; }
data_t *fft_f(const fft_data_t *fft) { return fft->f; }
index_t fft_n(const fft_data_t *fft) { return fft->n; }
index_t fft_win_size(const fft_data_t *fft) { return fft->win_size; }
void fft_set_win_size(fft_data_t *fft, index_t w) { fft->win_size = w; }
index_t fft_npeaks(const fft_data_t *fft) { return fft->n_peaks; }
void fft_set_npeaks(fft_data_t *fft, index_t n) { fft->n_peaks = n;}
index_t *fft_peaks(const fft_data_t *fft) { return fft->peaks; }
char *fft_output_file(const fft_data_t *fft) { return fft->output_file; }
void fft_set_output_file(fft_data_t *fft, const char *file) {
  fft->output_file = (char *)calloc(strlen(file) + 1, sizeof(char));
  strncpy(fft->output_file, file, strlen(file));
}
data_t fft_stdev(const fft_data_t *fft) { return fft->stdev; }
void fft_set_stdev(fft_data_t *fft, data_t sd) { fft->stdev = sd; }
data_t fft_nsigma(const fft_data_t *fft) { return fft->nsigma; }
void fft_set_nsigma(fft_data_t *fft, data_t s) { fft->nsigma = s; }
