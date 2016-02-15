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
#include <map>

typedef struct {
    stereo_plugin_init_fn init;
    stereo_plugin_fn fn;
    stereo_plugin_dispose_fn dispose;
} plugin_t;

static stereo_state_t pan_init(IValue* params)
{
    return NULL;
}

static stereo_sample_t pan_fn(stereo_state_t state, float sample)
{
    return {sample, sample};
}

static void pan_dispose(stereo_state_t state)
{
}

// TODO dynamic loading
static std::map<std::string, plugin_t> instanceMap{
    { "pan", { pan_init, pan_fn, pan_dispose }}
};

StereoInstance* NewStereoInstance(std::string name, IValue* params)
{
    auto&& found = instanceMap.find(name);
    if(found == instanceMap.end()) return nullptr;
    auto&& plugin = found->second;
    auto state = plugin.init(params);
    return new StereoInstance(state, plugin.fn, plugin.dispose);
}
