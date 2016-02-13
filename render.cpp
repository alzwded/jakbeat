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
#include <cstdio>
#include <parser_types.h>
#include <map>
#include <SDL.h>
#include <assert.h>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <utility>
#include <iterator>

std::map<std::string, std::vector<float>> LoadData(File& f)
{
    std::map<std::string, std::vector<float>> data;
    for(auto&& sample: f.samples) {
        auto& wav = data[sample.first];
        auto path = sample.second.path;
        Uint8* sdlWavData = nullptr;
        Uint32 len = 0;
        SDL_AudioSpec desired = {
            44100,
            AUDIO_F32SYS,
            1,
            0,
            4096,
            0
        };
        desired.callback = nullptr;
        desired.userdata = nullptr;
        auto hr = SDL_LoadWAV(
                path.c_str(),
                &desired,
                &sdlWavData,
                &len);
        if(hr == nullptr) {
            fprintf(stderr, "SDL_LoadWAV failed: %s\n", SDL_GetError());
            abort();
        }
        assert(desired.freq == 44100 && (desired.format == AUDIO_F32SYS || desired.format == AUDIO_F32 || desired.format == AUDIO_F32LSB || desired.format == AUDIO_S16LSB) && desired.channels == 1);
        if(desired.format == AUDIO_S16LSB) {
            wav.resize(len / sizeof(int16_t));
            int16_t* shorts = (int16_t*)sdlWavData;
            for(size_t i = 0; i < len/sizeof(int16_t); ++i) {
				//int16_t thing = (sdlWavData[2 * i] << 8) | sdlWavData[2 * i + 1];
				//wav[i] = (float)thing / (float)0x7FFF;
                wav[i] = (float)shorts[i]/(float)0x7FFF;
            }
        } else {
            wav.resize(len / sizeof(float));
            memcpy(wav.data(), sdlWavData, len);
        }
        SDL_FreeWAV(sdlWavData);
    }
    return std::move(data);
}

void Render(File f, std::string filename)
{
    std::map<std::string, std::vector<float>> data = LoadData(f);
    std::vector<float> outWAV;

    for(auto&& name: f.output) {
        auto&& phrase = f.phrases[name];
        size_t numSamplesPerBeat = 44100 * 60 / (phrase.bpm);
        size_t maxLen = std::accumulate(
                phrase.beats.begin(), phrase.beats.end(), (size_t)0,
                [](size_t a, decltype(phrase.beats)::value_type b) -> size_t {
                    return std::max(a, b.second.size());
                }
                );
        for(size_t i = 0; i < maxLen; ++i) {
            std::vector<float> ff(numSamplesPerBeat, 0.f);
            for(auto& beat: phrase.beats) {
                if(beat.second.size() <= i) continue;

                if(beat.second[i] == File::Beat::REST) continue;
                float gain = 1.f;
                if(beat.second[i] == File::Beat::HALF) gain = 0.5f;
                auto&& mydata = data[beat.first];
                size_t sampSize = mydata.size();
                size_t toCopy = std::min(sampSize, numSamplesPerBeat);
                auto volume = (float)f.samples[beat.first].volume / 100.f;
				for (size_t i = 0; i < toCopy; ++i) {
					ff[i] += gain * mydata[i] * volume;
				}
            }
            std::for_each(ff.begin(), ff.end(), [](float& f) {
                        f = tanhf(f);
                    });
            std::copy(ff.begin(), ff.end(), std::back_inserter(outWAV));
        }
    }

    extern void wav_write_file(std::string const&, std::vector<float> const&, unsigned);
    wav_write_file(filename, outWAV, 44100);
}
