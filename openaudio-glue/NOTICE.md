# Notice

OpenAudio Glue is part of the OpenAudio BeBoB MAFW project. The project's licence (PolyForm Noncommercial 1.0.0, see the
repository `LICENSE`) applies to this folder, except for the third-party code below, which keeps its own licence.

## Airwindows (MIT)

The transformer, tube, desk-colour and tape stages in `dsp/` are ports of Airwindows plug-ins by Chris Johnson:
**Interstage, Tube2, Channel9, ToTape9**. Source: <https://github.com/airwindows/airwindows>. Their source headers read
"Copyright (c) Airwindows, Airwindows uses the MIT license", and the repository licence is MIT.

```
MIT License

Copyright (c) 2018 Chris Johnson

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation
files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy,
modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software
is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE
WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
```

## RBJ Audio EQ Cookbook

The EQ filter formulas follow Robert Bristow-Johnson's "Cookbook formulae for audio EQ biquad filter coefficients".

## M-Audio

The FireWire 1814, FireWire Audiophile and FireWire 410 are M-Audio products. Glue only works with the OpenAudio driver and
M-Audio FireWire hardware; see the repository `NOTICE.md`.
