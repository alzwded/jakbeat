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

    struct chorus_state
    {
        int delay, pan;
        float buffer[4096];
        size_t writeHead;
        int steps;
        int phase;
        float amount;
        float depth;
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

static void chorus_assign_params(chorus_state* state, IValue* params)
{
    switch(params->GetType())
    {
    case IValue::LIST:
        {
            for(auto&& o: ((List*)params)->values) {
                chorus_assign_params(state, o);
            }
        };
        break;
    case IValue::OPTION:
        {
            auto o = (Option*)params;
            if(o->name.compare("delay") == 0) {
                if(o->value->GetType() == IValue::SCALAR) {
                    auto value = atoi(((Scalar*)o->value)->value.c_str());
                    state->delay = (int)(value/100.f * 2048.f);
                }
            } else if(o->name.compare("pan") == 0) {
                if(o->value->GetType() == IValue::SCALAR) {
                    auto value = atoi(((Scalar*)o->value)->value.c_str());
                    state->pan = value;
                }
            } else if(o->name.compare("amount") == 0) {
                if(o->value->GetType() == IValue::SCALAR) {
                    auto value = atoi(((Scalar*)o->value)->value.c_str());
                    state->amount = value/100.f;
                }
            } else if(o->name.compare("speed") == 0) {
                if(o->value->GetType() == IValue::SCALAR) {
                    auto value = atoi(((Scalar*)o->value)->value.c_str());

                    state->steps = 44100 / (value / 100.f * 20.f);
                }
            } else if(o->name.compare("depth") == 0) {
                if(o->value->GetType() == IValue::SCALAR) {
                    auto value = atoi(((Scalar*)o->value)->value.c_str());

                    state->steps = 44100 / (value / 100.f * 200.f);
                }
            }
        };
    }
}

static stereo_state_t chorus_init(IValue* params)
{
    auto state = new chorus_state;
    state->phase = 0;
    state->delay = 1500;
    state->pan = 0;
    state->amount = 0.33;
    state->steps = 44100 / 5;
    state->depth = 44100.f / 20.f;
    state->writeHead = 0;
    memset(state->buffer, 0, sizeof(state->buffer));
    chorus_assign_params(state, params);
    return state;
}

static stereo_sample_t chorus_fn(stereo_state_t pstate, float sample)
{
    auto state = (chorus_state*)pstate;

    auto lfoA = sin(2 * 3.14159 * (state->phase/44100.0));
    auto delay = state->delay + state->depth*lfoA;
    state->writeHead = (state->writeHead + 1) % 4096;
    state->buffer[state->writeHead] = sample;
    auto s0 = sample;
    auto s1 = state->buffer[(state->writeHead + 4096 - state->delay) % 4096];
    auto s2 = state->buffer[(state->writeHead + 4096 - 2*state->delay) % 4096];

    float leftGain = 1.f;
    float rightGain = 1.f;
    float gain0 = 1.f - state->amount/2.f;
    float gain1 = state->amount * 0.5;
    float gain2 = state->amount * 0.5;
    if(state->pan < 0) {
        leftGain = 1.f;
        rightGain = (100.f - abs(state->pan)) / 100.f;
        gain1 = state->amount * 0.33;
        gain2 = state->amount * 0.67;
    } else if(state->pan > 0) {
        leftGain = (100.f - abs(state->pan)) / 100.f;
        rightGain = 1.f;
        gain1 = state->amount * 0.67;
        gain2 = state->amount * 0.33;
    }

    float amount = state->amount / 100.f;
    float left = leftGain * gain0 * s0 + leftGain * gain1 * s1;
    float right = rightGain * gain0 * s0 + rightGain * gain2 * s2;

    state->phase = (state->phase + state->steps) % 44100;

    return {left, right};
}

static void chorus_dispose(stereo_state_t pstate)
{
    free(pstate);
}

// TODO dynamic loading
static std::map<std::string, plugin_t> instanceMap{
    { "pan", { pan_init, pan_fn, pan_dispose }},
    { "chorus", { chorus_init, chorus_fn, chorus_dispose }}
};

StereoInstance* NewStereoInstance(std::string name, IValue* params)
{
    auto&& found = instanceMap.find(name);
    if(found == instanceMap.end()) return new StereoInstance(new pan_state{0}, pan_fn, pan_dispose);
    auto&& plugin = found->second;
    auto state = plugin.init(params);
    return new StereoInstance(state, plugin.fn, plugin.dispose);
}
