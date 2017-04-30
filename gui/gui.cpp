/*
Copyright (c) 2017, Vlad Meșco
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
#include "model.h"
#include "window.h"

#include <FL/Fl.H>

#include <algorithm>
#include <cassert>

Schema drumSchemas[] = {
    /* Basic Drum */
    {
        "Mono",
        {
            { "path", Schema::STRING },
            { "volume", Schema::NUMBER },
        },
    },
    /* Stereo Drum */
    {
        "Stereo",
        {
            { "path", Schema::STRING },
            { "volume", Schema::NUMBER },
            { "stereo", Schema::READ_ONLY_STRING, {
                                                      { "pan", Schema::STUB },
                                                  }},
            { "params", Schema::SUBSCHEMA, {
                                               { "pan", Schema::NUMBER },
                                           }},
        },
    },
    /* Chorus Drum */
    {
        "Chorus",
        {
            { "path", Schema::STRING },
            { "volume", Schema::NUMBER },
            { "stereo", Schema::READ_ONLY_STRING, {
                                                      { "chorus", Schema::STUB },
                                                  }},
            { "params", Schema::SUBSCHEMA, {
                                               { "pan", Schema::NUMBER },
                                               { "delay", Schema::NUMBER },
                                               { "amount", Schema::NUMBER },
                                               { "speed", Schema::NUMBER },
                                               { "depth", Schema::NUMBER },
                                           }},
        },
    },
};

Schema whatSchemas[] = {
    {
        "Standard",
        {
            { "bpm", Schema::NUMBER },
        },
    },
};

std::list<Window> windows;

std::shared_ptr<Model> load_model(std::string path)
{
    return {};
}

void save_model(std::shared_ptr<Model> m)
{
}

bool is_any_model_dirty()
{
    return std::any_of(windows.begin(), windows.end(), [](Window& w) -> bool {
                return w.GetModel()->dirty;
            });
}

void create_window(std::shared_ptr<Model> m, int argc, char* argv[])
{
    windows.emplace_back(m);
    assert(m);
    assert(windows.back().GetModel());
    windows.back().show(1, argv);
}

void destroy_window(Window* whom)
{
    auto found = std::find_if(windows.begin(), windows.end(), [whom](Window const& w) -> bool {
                return &w == whom;
            });
    if(found == windows.end())
    {
        fprintf(stderr, "window not found... not abort()-ing to not lose data, but it should be fixed");
        return;
    }
    windows.erase(found);
}

int main(int argc, char* argv[])
{
#if 1
    /* test code */

    std::shared_ptr<Model> m(new Model());
    m->whos.push_back({
                "kick",
                &drumSchemas[0],
                {
                    { "path", "kick.wav" },
                    { "volume", "100" },
                }
            });
    m->dirty = true;
    create_window(m, argc, argv);

    return Fl::run();
#endif
}
