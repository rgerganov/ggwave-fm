#include <ggwave.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <cstring>
#include <climits>
#include <vector>
#include <cmath>
#include "dsp.h"
#include "ringbuffer.hpp"

const int SAMPLE_RATE = 48000;

typedef enum {
    IQ_SAMPLE_FORMAT_S8,
    IQ_SAMPLE_FORMAT_F32,
} IQSampleFormat;

void usage()
{
    fprintf(stderr, "Usage: ggwave-fm -m <message> [-p <protocol>] [-o <output>] [-f <format>]\n"
            "   -m message      - message to be transmitted\n"
            "   -p protocol     - ggwave protocol: normal(default), fast, fastest"
            "   -o output       - output file (default stdout)\n"
            "   -f format       - output sample format: f32(default), s8(HackRF)\n");
    exit(1);
}

std::vector<float> encode(const char *msg, ggwave_TxProtocolId tx_proto)
{
    ggwave_Parameters params = ggwave_getDefaultParameters();
    params.payloadLength = -1;
    params.sampleRateOut = SAMPLE_RATE;
    params.sampleFormatOut = GGWAVE_SAMPLE_FORMAT_F32;
    ggwave_Instance inst = ggwave_init(params);
    int n = ggwave_encode(inst, msg, strlen(msg), tx_proto, 25, NULL, 2);
    std::vector<float> waveform(n);
    ggwave_encode(inst, msg, strlen(msg), tx_proto, 25, (char*)waveform.data(), 0);
    ggwave_free(inst);
    return waveform;
}

std::vector<int8_t> f32_to_s8(const std::vector<std::complex<float>> &input)
{
    std::vector<int8_t> result(input.size() * 2);
    for (int i = 0; i < (int) input.size(); i++) {
        result[i*2] = input[i].real() * SCHAR_MAX;
        result[i*2+1] = input[i].imag() * SCHAR_MAX;
    }
    return result;
}

// FM modulation + interpolation (x50)
// output sample rate: 48000 * 50 = 2400000
int modulate(const std::vector<float> &waveform, const char *output, IQSampleFormat iq_sf)
{
    float max_deviation = 5000; // 5kHz deviation
    float sensitivity = 2 * M_PI * max_deviation / (float)SAMPLE_RATE;
    float factor = 50.0;
    float fractional_bw = 0.4;
    float halfband = 0.5;
    float trans_width = halfband - fractional_bw;
    float mid_transition_band = halfband - trans_width / 2.0;
    std::vector<float> taps = lowpass(factor, factor, mid_transition_band, trans_width);

    FILE *fout = stdout;
    if (output) {
        fout = fopen(output, "wb");
        if (fout == NULL) {
            fprintf(stderr, "Error creating output file '%s'\n", output);
            return 1;
        }
    }

    Ringbuffer_t mod_buf;
    FIRInterpolator interp(factor, taps);
    float last_phase = 0;
    int offset = 0;
    while (offset < (int)waveform.size()) {
        int input_size = std::min(BUFSIZE, (int)waveform.size() - offset);
        last_phase = fmmod(waveform.data()+offset, input_size, mod_buf, sensitivity, last_phase);

        std::vector<std::complex<float>> interp_buf;
        int processed = interp.interpolate(mod_buf, interp_buf);
        if (!processed) {
            break;
        }
        mod_buf.remove(processed);
        if (iq_sf == IQ_SAMPLE_FORMAT_S8) {
            auto samples_s8 = f32_to_s8(interp_buf);
            fwrite(samples_s8.data(), sizeof(int8_t), samples_s8.size(), fout);
        } else {
            fwrite(interp_buf.data(), sizeof(std::complex<float>), interp_buf.size(), fout);
        }
        offset += input_size;
    }
    fclose(fout);
    return 0;
}

int main(int argc, char *argv[])
{
    int opt;
    char *msg = NULL;
    char *output = NULL;
    ggwave_TxProtocolId tx_proto = GGWAVE_TX_PROTOCOL_DT_NORMAL;
    IQSampleFormat iq_sf = IQ_SAMPLE_FORMAT_F32;

    while ((opt = getopt(argc, argv, "m:p:o:f:")) != -1) {
        switch (opt) {
            case 'm':
                msg = strdup(optarg);
                break;
            case 'p':
                if (strcmp(optarg, "normal") == 0) {
                    tx_proto = GGWAVE_TX_PROTOCOL_DT_NORMAL;
                } else if (strcmp(optarg, "fast") == 0) {
                    tx_proto = GGWAVE_TX_PROTOCOL_DT_FAST;
                } else if (strcmp(optarg, "fastest") == 0) {
                    tx_proto = GGWAVE_TX_PROTOCOL_DT_FASTEST;
                } else {
                    fprintf(stderr, "Incorrect protocol: %s\n", optarg);
                    return 1;
                }
                break;
            case 'o':
                output = strdup(optarg);
                break;
            case 'f':
                if (strcmp(optarg, "s8") == 0) {
                    iq_sf = IQ_SAMPLE_FORMAT_S8;
                } else if (strcmp(optarg, "f32") == 0) {
                    iq_sf = IQ_SAMPLE_FORMAT_F32;
                } else {
                    fprintf(stderr, "Incorrect sample format: %s\n", optarg);
                    return 1;
                }
                break;
            default:
                usage();
                break;
        }
    }
    if (!msg) {
        fprintf(stderr, "No message specified\n");
        return 1;
    }

    auto waveform = encode(msg, tx_proto);
    return modulate(waveform, output, iq_sf);
}
