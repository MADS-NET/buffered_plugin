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

#ifndef PEAKSEARCH_H
#define PEAKSEARCH_H

#define FULL    0
#define PARTIAL 1

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct statistics statistics_t;

static data_t mean(fft_data * const d, index_t start, index_t end);

static void compute_stats(fft_data * const d, index_t start, char type, statistics_t * const stat);

index_t fft_search_peaks(fft_data * const d, index_t max_peaks);


#ifdef __cplusplus
}
#endif
#endif