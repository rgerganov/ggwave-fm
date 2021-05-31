# ggwave-fm

Transmit [ggwave](https://github.com/ggerganov/ggwave) encoded messages with an SDR and receive them with an FM radio.

Here is a demo with HackRF transmitting messages on 145.650MHz. Messages are received with Baofeng FM radio and decoded with the [Waver](https://play.google.com/store/apps/details?id=com.ggerganov.Waver) mobile app:

[![Demo](https://img.youtube.com/vi/pO4SuNroBA0/0.jpg)](https://www.youtube.com/watch?v=pO4SuNroBA0)

## Building
Build and install [ggwave](https://github.com/ggerganov/ggwave) first. Then:
```
$ export LDFLAGS=-L<GGWAVE_LIBDIR>
$ make
```
## Usage
The program takes a text message (specified with `-m`) and outputs a complex sampled IQ file which is suitable for transmission with an SDR. The output sample format is specified with `-f` and can be either 32bit float or 8bit signed integer. The output sample rate is 2400000.

For example to transmit "Hello, world" on 145.650MHz with HackRF:
```
$ ggwave-fm -m 'Hello, world' -o hello.s8 -f s8
$ hackrf_transfer -s 2400000 -f 145650000 -t hello.s8 -a 1 -x 20
```
You can also pipe the output of `ggwave-fm` to `hackrf_transfer`:
```
ggwave-fm -m 'Hello, world' -f s8 | hackrf_transfer -s 2400000 -f 145650000 -t - -a 1 -x 20
```

The `-p` option specifies the ggwave protocol: normal (default), fast, fastest

## License

`ggwave` is licensed under MIT.

Some parts of `dsp.cpp` are borrowed from GNU Radio which is licensed under GPL version 3.

`ringbuffer.hpp` is licensed under CC0 License.
