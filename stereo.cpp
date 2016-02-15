/*
Copyright (c) 2016, Vlad Meșco
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

#include <stereo.h>
#include <parser_types.h>
#include <map>
#include <assert.h>

typedef struct {
    stereo_plugin_init_fn init;
    stereo_plugin_fn fn;
    stereo_plugin_dispose_fn dispose;
} plugin_t;

namespace {
    struct pan_state
    {
        int pan;
    };
}

static void pan_assign_params(pan_state* state, IValue* params)
{
    switch(params->GetType()) {
    case IValue::SCALAR:
        {
            auto value = atoi(((Scalar*)params)->value.c_str());
            state->pan = value;
        };
        break;
    case IValue::OPTION:
        {
            auto o = (Option*)params;
            if(o->name.compare("pan") == 0) {
                return pan_assign_params(state, o->value);
            }
        };
        break;
    case IValue::LIST:
        {
            for(auto&& o: ((List*)params)->values) {
                pan_assign_params(state, o);
            }
        };
        break;
    }
}

static stereo_state_t pan_init(IValue* params)
{
    auto state = (pan_state*)malloc(sizeof(pan_state));
    state->pan = 0;
    assert(params);
    pan_assign_params(state, params);
    return state;
}

static stereo_sample_t pan_fn(stereo_state_t pstate, float sample)
{
    auto state = (pan_state*)pstate;
    if(state->pan < 0) return {sample, ((100 - abs(state->pan))/100.f) * sample};
    else if(state->pan > 0) return {((100 - abs(state->pan))/100.f) * sample, sample};
    return {sample, sample};
}

static void pan_dispose(stereo_state_t pstate)
{
    auto state = (pan_state*)pstate;
    free(state);
}

// TODO dynamic loading
static std::map<std::string, plugin_t> instanceMap{
    { "pan", { pan_init, pan_fn, pan_dispose }}
};

StereoInstance* NewStereoInstance(std::string name, IValue* params)
{
    auto&& found = instanceMap.find(name);
    if(found == instanceMap.end()) return new StereoInstance(new pan_state{0}, pan_fn, pan_dispose);
    auto&& plugin = found->second;
    auto state = plugin.init(params);
    return new StereoInstance(state, plugin.fn, plugin.dispose);
}
