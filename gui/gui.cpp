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
        L"Mono",
            {
                { L"path", Schema::STRING },
                { L"volume", Schema::NUMBER },
            },
    },
        /* Stereo Drum */
        {
            L"Stereo",
            {
                { L"path", Schema::STRING },
                { L"volume", Schema::NUMBER },
                { L"stereo", Schema::STUB, {
                                               { L"pan", Schema::READ_ONLY_STRING },
                                           }},
                { L"params", Schema::SUBSCHEMA, {
                                                    { L"pan", Schema::NUMBER },
                                                }},
            },
        },
        /* Chorus Drum */
        {
            L"Chorus",
            {
                { L"path", Schema::STRING },
                { L"volume", Schema::NUMBER },
                { L"stereo", Schema::STUB, {
                                               { L"chorus", Schema::READ_ONLY_STRING },
                                           }},
                { L"params", Schema::SUBSCHEMA, {
                                                    { L"pan", Schema::NUMBER },
                                                    { L"delay", Schema::NUMBER },
                                                    { L"amount", Schema::NUMBER },
                                                    { L"speed", Schema::NUMBER },
                                                    { L"depth", Schema::NUMBER },
                                                }},
            },
        },
};

std::vector<Schema> whatSchemas {
    {
        L"Standard",
            {
                { L"bpm", Schema::NUMBER },
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
    LOGGER(l);
    auto found = std::find_if(windows.begin(), windows.end(), [whom](Vindow * w) -> bool {
                return w == whom;
            });
    if(found == windows.end())
    {
        l(L"window not found... not abort()-ing to not lose data, but it should be fixed");
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
                L"kick",
                &drumSchemas[0],
                {
                    { L"path", L"kick.wav" },
                    { L"volume", L"100" },
                }
            });
    m->whos.push_back({
                L"snare",
                &drumSchemas[1],
                {
                    { L"path", L"snare.wav" },
                    { L"volume", L"100" },
                    { L"stereo", L"chorus" },
                    { L"pan", L"-10" },
                }
            });
    m->whos.push_back({
                L"hat",
                &drumSchemas[2],
                {
                    { L"path", L"snare.wav" },
                    { L"volume", L"100" },
                    { L"stereo", L"chorus" },
                    { L"pan", L"-10" },
                    { L"delay", L"3" },
                    { L"amount", L"60" },
                    { L"speed", L"30" },
                    { L"depth", L"30" },
                }
            });
    m->whats.push_back({
                L"A1",
                L"120",
                &whatSchemas[0],
                {
                    { L'!', L'.', L'/', },
                    { L'.', L'!', L':', },
                    { L'.', L'.', L'/', },
                }});
    m->output.push_back({{ L'1' }});
    m->output.push_back({{ L'1' }});
    for(size_t i = 0; i < 80; ++i) {
        m->output.push_back({{ L'.' }});
    }
    m->output.push_back({{ L'1' }});
    create_window(m);

    return Fl::run();
#endif
}
