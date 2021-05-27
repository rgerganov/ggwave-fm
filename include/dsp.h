#ifndef __DSP_H__
#define __DSP_H__

#include <vector>
#include <deque>
#include <complex>
#include "ringbuffer.hpp"

const int BUFSIZE = 4096;

typedef jnk0le::Ringbuffer<std::complex<float>, BUFSIZE*2> Ringbuffer_t;

// lowpass FIR filter
std::vector<float> lowpass(double gain, double sampling_freq, double cutoff_freq, double transition_width);

// FM modulator
float fmmod(const float *input, size_t input_size, Ringbuffer_t &output, float sensitivity, float last_phase);

// used only for tests
void naive_interpolate(const std::vector<std::complex<float>> &input,
                       std::vector<std::complex<float>> &output,
                       int interpolation,
                       const std::vector<float> &taps);

class FIRInterpolator
{
public:
    FIRInterpolator(int interpolation, const std::vector<float> &taps);
    int interpolate(Ringbuffer_t &input, std::vector<std::complex<float>> &output);

private:
    // these are the taps for the polyphase filers
    std::vector<std::vector<float>> xtaps;
};
#endif
