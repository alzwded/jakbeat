jakbeat
=======

Miniature sample-based drum sequencer

How does it work?
=================

You give it input in the format described in [NOTES.md](NOTES.md) and it chugs out a wave file. `test.drm` is such an example files.

Samples must be `.wav` files in either f32le or s16le format.

If you want to run the `test.drm` example, get some kick and snare samples from somewhere and drop them in the root directory as `kick.wav` and `snare.wav`. Then, build `jakbeat` and run `jakbeat < test.drm`. You should have a `test.wav` file which sounds like a groove.

Building
========

There are platform/compiler specific makefiles. See below.

To activate debug, `setenv JAKBEAT_OPTS debug` before running make/nmake

Win32
-----

The build was tested with MSVC 12.0 (that would be Visual Studio 2013).

The project depends on lemon (included in the sources) and SDL 2.0 which you need to get (put it in \vendor).

Then, just `nmake` in the root directory and you should end up with `jakbeat.exe` which reads input from STDIN.

Linux
-----

There's [this makefile](Makefile.gcc) that can be used to build on linux.

Tested with gcc 6.3, but should work with any gcc that supports `--std=gnu++14`.

It depends on libSDL2 and its headers which are expected to be in installed in the standard paths. In its current state, the headers REALLY need to be in `/usr/include`, otherwise you need to manually patch the makefile. (mostly because the silly win32 package doesn't include and SDL2 subdirectory in the include dir...)

To build with GNU make do a `make -f Makefile.gcc` and to clean `make -f Makefile.gcc clean`.
