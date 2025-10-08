/******************************************************************************\
 _____ _____ _____         _   _ _ _ _   _
|  ___|  ___|_   _|  _   _| |_(_) (_) |_(_) ___  ___
| |_  | |_    | |   | | | | __| | | | __| |/ _ \/ __|
|  _| |  _|   | |   | |_| | |_| | | | |_| |  __/\__ \
|_|   |_|     |_|    \__,_|\__|_|_|_|\__|_|\___||___/

==============================================================================
 File:         fft.h
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

#include <math.h>
#include <stdlib.h>
#include <stdint.h>

#ifndef FFT_H
#define FFT_H

// Whenever the output array gets full, it is expanded by this quantity:
#define INITIAL_N_PEAKS 5 // initial allocation
#define CHUNK_SIZE 10     // chunks of re-allocation

#ifdef __cplusplus
extern "C"
{
#endif

// Types for data and indexes:
typedef double   data_t;
typedef uint16_t index_t;

// signature for windowing function
typedef void (*fft_windowing)(data_t x[], data_t bias, index_t n);

// Object Structure
typedef struct fft_data fft_data_t;

// Initializer&de-initializer
fft_data_t *fft_init(index_t radix, data_t freq);
void fft_free(fft_data_t * d);
void fft_reset(fft_data_t *d);
index_t *fft_realloc_peaks(fft_data_t *d, size_t n);

// Append data point and update statistics
// returns 0 when data are full, 1 otherwise
int fft_add_point(fft_data_t * const d, data_t x, data_t y);

// Basic windowing (Hamming)
void hamming(data_t x[], data_t bias, index_t n);
void hann(data_t x[], data_t bias, index_t n);
void blackmann(data_t x[], data_t bias, index_t n);
void fft_apply_window(fft_data_t * const d, fft_windowing w);
void fft_apply_window_and_bias(fft_data_t * const d, fft_windowing w);

// Calculate spectrum (in place: initial data are lost)
// NOTE: returns polar coordinates
index_t fft_calc_spectrum(fft_data_t * const d);
// Run peak search algorithm
index_t fft_search_peaks(fft_data_t * const d, index_t max_peaks);

// Utilities
// operate an in-place transform from rectangilar to polar coords
void to_polar(data_t * const x, data_t * const y);


// Accessors

data_t *fft_x(const fft_data_t *fft);
data_t *fft_y(const fft_data_t *fft);
data_t *fft_t(const fft_data_t *fft);
data_t *fft_f(const fft_data_t *fft);
index_t fft_n(const fft_data_t *fft);
index_t fft_win_size(const fft_data_t *fft);
void fft_set_win_size(fft_data_t *fft, index_t w);
index_t fft_npeaks(const fft_data_t *fft);
void fft_set_npeaks(fft_data_t *fft, index_t n);
index_t *fft_peaks(const fft_data_t *fft);
char *fft_output_file(const fft_data_t *fft);
void fft_set_output_file(fft_data_t *fft, const char *file);
data_t fft_stdev(const fft_data_t *fft);
void fft_set_stdev(fft_data_t *fft, double sd);
data_t fft_nsigma(const fft_data_t *fft);
void fft_set_nsigma(fft_data_t *fft, data_t s);


#ifdef __cplusplus
}
#endif
#endif
