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

    struct reverb_state
    {
        int delay, pan, level, decay;
        float lap1[512], lap2[512], lfb1[512], lfb2[512];
        float rap1[512], rap2[512], rfb1[512], rfb2[512];
        size_t head;
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

static stereo_state_t reverb_init(IValue* params)
{
    reverb_state* state = (reverb_state*)malloc(sizeof(reverb_state));
    memset(state, 0, sizeof(reverb_state));
    state->delay = 100;
    state->pan = -50;
    //state->level = 50;
    state->level = 100;
    //state->decay = 67;
    state->decay = 1;
    state->head = 0;
    return state;
}

static stereo_sample_t reverb_fn(stereo_state_t pstate, float dry)
{
    reverb_state* state = (reverb_state*)pstate;

    // compute reverb for left and for right
    // so a double buffer; cross talk should exist
    // left: dry + lwet[-1] + lwet[-3] + rwet[-4]
    // right: dry + rwet[-1] + rwet[-3] + rwet[-4]
    // left[0] = lgain*(dry + decay*left[-1] + decay*decay*left[-3]) + rgain*decay*right[-4]
    // right[0] = rgain*(dry + decay*right[-2] + decay*decay*right[-4]) + lgain*decay*left[-5]
    // no
    //
    // left: dry + AP_1 + AP_3 + FB_5 + FB_6
    // right: dry + AP_2 + AP_4 + FB_6 + FB_7
    // left: dry->AP_1->AP_3->FB_5->FB_7
    // right: dry->AP_2->AP4->FB_6->FB_8

    /*
    ap1[0] = dry * lgain + ap1[0 - delay] * decay - dry * lgain * decay; 
    ap3[0] = ap1[0] + ap3[0 - delay] * decay - ap1[0] * decay;
    fb5[0] = ap3[0] + fb5[0 - delay] * decay;
    fb7[0] = fb5[0] + fb7[0 - delay] * decay;
    left = level * (fb5[head] + fb8[head]) + (1 - level) * lgain * dry;
    left = level * (fb6[head] + fb7[head]) + (1 - level) * rgain * dry;

    params: decay, delay (ap), level, pan
    */

    int delay = state->delay / 100.f * 512.f;
    float decay = state->decay / 105.f;
    float lgain = 1.0;
    float rgain = 1.0;
    if(state->pan < 0) {
        rgain = (100.f - abs(state->pan)) / 100.f;
    } else if(state->pan > 0) {
        lgain = (100.f - abs(state->pan)) / 100.f;
    }

    state->lap1[state->head] = 0.6 * dry * lgain
        + 0.5 * state->lap1[(state->head + 512 - delay) % 512] * decay
        - 0.33 * dry * lgain * decay;
    state->rap1[state->head] = 0.6 * dry * rgain
        + 0.5 * state->lap1[(state->head + 512 - delay) % 512] * decay
        - 0.33 * dry * rgain * decay;
    state->lap2[state->head] = 0.6 * state->lap1[state->head]
        + 0.5 * state->lap2[(state->head + 512 - delay) % 512] * decay
        - 0.33 * state->lap1[state->head] * decay;
    state->rap2[state->head] = 0.6 * state->rap1[state->head]
        + 0.5 * state->rap2[(state->head + 512 - delay) % 512] * decay
        - 0.33 * state->rap1[state->head] * decay;
    
#define RF1_DELAY 220
#define RF2_DELAY 75

    state->lfb1[state->head] = 0.6 * state->lap2[state->head]
        + 0.5 * state->lfb1[(state->head + 512 - RF1_DELAY) % 512] * decay;
    state->rfb1[state->head] = 0.6 * state->rap2[state->head]
        + 0.5 * state->rfb1[(state->head + 512 - RF1_DELAY) % 512] * decay;

    state->lfb2[state->head] = 0.6 * state->lfb1[state->head]
        + 0.33 * state->lfb2[(state->head + 512 - RF2_DELAY) % 512] * decay
        - 0.33 * state->rfb2[(state->head + 512 - RF1_DELAY) % 512] * decay;
    state->rfb2[state->head] = 0.6 * state->rfb1[state->head]
        + 0.33 * state->rfb2[(state->head + 512 - RF2_DELAY) % 512] * decay
        - 0.33 * state->lfb2[(state->head + 512 - RF1_DELAY) % 512] * decay;

    float left = (state->level/100.f) * state->lfb2[state->head] + (100.f - state->level) / 100.f * lgain * dry;
    float right = (state->level/100.f) * state->rfb2[state->head] + (100.f - state->level) / 100.f * rgain * dry;

    state->head = (state->head + 1) % 512;

    return {left, right};
}

static void reverb_dispose(stereo_state_t state)
{
    free(state);
}

// TODO dynamic loading
static std::map<std::string, plugin_t> instanceMap{
    { "pan", { pan_init, pan_fn, pan_dispose }},
    { "chorus", { chorus_init, chorus_fn, chorus_dispose }},
    { "reverb", { reverb_init, reverb_fn, reverb_dispose }}
};

StereoInstance* NewStereoInstance(std::string name, IValue* params)
{
    auto&& found = instanceMap.find(name);
    if(found == instanceMap.end()) return new StereoInstance(new pan_state{0}, pan_fn, pan_dispose);
    auto&& plugin = found->second;
    auto state = plugin.init(params);
    return new StereoInstance(state, plugin.fn, plugin.dispose);
}
