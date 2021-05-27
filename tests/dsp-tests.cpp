#include <cassert>
#include "dsp.h"

void gen_waveform(std::vector<std::complex<float>> &waveform)
{
    for (int i = 0 ; i < (int) waveform.size() ; i++) {
        waveform[i] = std::complex<float>(i*0.2, i*0.3);
    }
}

void test_interpolate()
{
    int n = 6000;
    std::vector<std::complex<float>> waveform(n);
    gen_waveform(waveform);

    float factor = 50.0;
    float fractional_bw = 0.4;
    float halfband = 0.5;
    float trans_width = halfband - fractional_bw;
    float mid_transition_band = halfband - trans_width / 2.0;
    std::vector<float> taps = lowpass(factor, factor, mid_transition_band, trans_width);
    printf("Taps size: %lu\n", taps.size());

    Ringbuffer_t mod_buf;
    std::vector<std::complex<float>> output1;
    FIRInterpolator interp(factor, taps);

    int offset = 0;
    while (offset < (int)waveform.size()) {
        int input_size = std::min(BUFSIZE, (int)waveform.size() - offset);
        int x = mod_buf.writeBuff(waveform.data()+offset, input_size);
        assert(x == input_size);
        int processed = interp.interpolate(mod_buf, output1);
        if (!processed) {
            break;
        }
        mod_buf.remove(processed);
        printf("offset=%d processed=%d\n", offset, processed);
        offset += input_size;
    }

    std::vector<std::complex<float>> output2;
    naive_interpolate(waveform, output2, factor, taps);

    printf("n*factor=%d output1.size=%ld output2.size=%ld\n", n*(int)factor, output1.size(), output2.size());
    float eps = 0.000001;
    for (int i = 0 ; i < (int)output2.size() ; i++) {
        assert(fabs(output1[i].real()-output2[i].real()) < eps && fabs(output1[i].imag()-output2[i].imag()) < eps);
    }
}

int main()
{
    test_interpolate();
    return 0;
}
