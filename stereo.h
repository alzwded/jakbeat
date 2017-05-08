/*
Copyright (c) 2016-2017, Vlad Meșco
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef STEREO_H
#define STEREO_H

#include <string>

struct IValue;

typedef struct {
    float data[2];
} stereo_sample_t;

typedef void* stereo_state_t;

typedef stereo_state_t (*stereo_plugin_init_fn)(IValue* params);
typedef stereo_sample_t (*stereo_plugin_fn)(stereo_state_t state, float mono_sample);
typedef void (*stereo_plugin_dispose_fn)(stereo_state_t);

struct StereoInstance;

StereoInstance* NewStereoInstance(std::wstring name, IValue* params);

struct StereoInstance {
    stereo_sample_t operator()(float sample) { return fn(state, sample); }
    ~StereoInstance() { dispose(state); }

private:
    stereo_state_t state;
    stereo_plugin_fn fn;
    stereo_plugin_dispose_fn dispose;

private:
    StereoInstance(stereo_state_t state_,
            stereo_plugin_fn fn_,
            stereo_plugin_dispose_fn dispose_)
        : dispose(dispose_)
          , fn(fn_)
          , state(state_)
    {}

    friend StereoInstance* NewStereoInstance(std::wstring, IValue*);
};

#endif
