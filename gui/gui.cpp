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
#include "logger.h"

#include <FL/Fl.H>

#include <algorithm>
#include <vector>
#include <cassert>

std::vector<Schema> drumSchemas {
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
            { "stereo", Schema::STUB, {
                                                      { "pan", Schema::READ_ONLY_STRING },
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
            { "stereo", Schema::STUB, {
                                                      { "chorus", Schema::READ_ONLY_STRING },
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

std::vector<Schema> whatSchemas {
    {
        "Standard",
        {
            { "bpm", Schema::NUMBER },
        },
    },
};

std::list<Vindow*> windows;

std::shared_ptr<Model> load_model(std::string path)
{
    return {};
}

void save_model(std::shared_ptr<Model> m)
{
}

bool is_any_model_dirty()
{
    return std::any_of(windows.begin(), windows.end(), [](Vindow* w) -> bool {
                return w->GetModel()->dirty;
            });
}

void create_window(std::shared_ptr<Model> m)
{
    windows.emplace_back(new Vindow(m, drumSchemas, whatSchemas));
    assert(m);
    assert(windows.back()->GetModel());
    windows.back()->show();
}

void destroy_window(Vindow* whom)
{
    auto found = std::find_if(windows.begin(), windows.end(), [whom](Vindow * w) -> bool {
                return w == whom;
            });
    if(found == windows.end())
    {
        fprintf(stderr, "window not found... not abort()-ing to not lose data, but it should be fixed");
        return;
    }
    windows.erase(found);
    Fl::delete_widget(whom);
}

int main(int argc, char* argv[])
{
    Fl::scheme("gtk+");
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
    m->whos.push_back({
                "snare",
                &drumSchemas[1],
                {
                    { "path", "snare.wav" },
                    { "volume", "100" },
                    { "stereo", "chorus" },
                    { "pan", "-10" },
                }
            });
    m->whos.push_back({
                "hat",
                &drumSchemas[2],
                {
                    { "path", "snare.wav" },
                    { "volume", "100" },
                    { "stereo", "chorus" },
                    { "pan", "-10" },
                    { "delay", "3" },
                    { "amount", "60" },
                    { "speed", "30" },
                    { "depth", "30" },
                }
            });
    m->whats.push_back({
                "A1",
                "120",
                &whatSchemas[0],
                {
                    { '!', '.', '/', },
                    { '.', '!', ':', },
                    { '.', '.', '/', },
                }});
    m->output.push_back({{ '1' }});
    m->output.push_back({{ '1' }});
    for(size_t i = 0; i < 80; ++i) {
        m->output.push_back({{ '.' }});
    }
    m->output.push_back({{ '1' }});
    create_window(m);

    return Fl::run();
#endif
}
