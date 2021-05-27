# ggwave-fm

Transmit [ggwave](https://github.com/ggerganov/ggwave) encoded messages with an SDR and receive them with an FM radio.

## Building
Build and install [ggwave](https://github.com/ggerganov/ggwave) first. Then:
```
$ export LDFLAGS=<GGWAVE_LIBDIR>
$ make
```
## Usage
Transmit "Hello, world" on 145.650MHz using `hackrf_transfer`:
```
$ ggwave-fm -m 'Hello, world' -o hello.s8 -f s8
$ hackrf_transfer -s 2400000 -f 145650000 -t hello.s8 -a 1 -x 20
```
You can also pipe the output of `ggwave-fm` to `hackrf_transfer`:
```
ggwave-fm -m 'Hello, world' -f s8 | hackrf_transfer -s 2400000 -f 145650000 -t - -a 1 -x 20
```

## License

`ggwave` is licensed under MIT.

Some parts of `dsp.cpp` are borrowed from GNU Radio which is licensed under GPL version 3.

`ringbuffer.hpp` is licensed under CC0 License.