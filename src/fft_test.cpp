#include <iostream>
#include <cmath>
#include "fft.h"
#include "peaksearch.h"

using namespace std;

int main() {
  size_t exp = 10;
  size_t n = std::pow(2, exp);
  double freq = 1000.0;
  fft_data_t *fft = fft_init(exp, freq);
  fft_set_win_size(fft, 10);
  fft_set_nsigma(fft, 2);
  // if you set an output file, then the analysis will be saved (useful in debug)
  fft_set_output_file(fft, "fft.txt");

  double t, dt = 1.0 / freq;
  // fill signal
  for (int i = 0; i < n; i++) {
    t = i * dt;
    fft_add_point(fft, 2 * std::sin(t*128*2*M_PI) + 0.8 * std::sin(t*200*2*M_PI), 0);
  }

  fft_apply_window_and_bias(fft, hann);
  fft_calc_spectrum(fft);
  cout << "Found " << fft_search_peaks(fft, 10) << " peaks" << endl;

  for (int i = 0; i < fft_npeaks(fft); i++) {
    cout << "peak " << i << " at index " << fft_peaks(fft)[i]
         << " freq " << fft_f(fft)[fft_peaks(fft)[i]]
         << " value " << fft_x(fft)[fft_peaks(fft)[i]]
         << endl;
  }

  fft_free(fft);

  return 0;
}