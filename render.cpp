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
#include <cstdio>
#include <file.h>
#include <parser_types.h>
#include <map>
#include <SDL.h>
#include <errorassert.h>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <utility>
#include <iterator>
#include <type_traits>
#include <string_utils.h>

std::map<std::wstring, std::vector<float>> LoadData(File& f)
{
    std::map<std::wstring, std::vector<float>> data;
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
        auto wpath = W2MB(path);
        auto hr = SDL_LoadWAV(
                wpath.get(),
                &desired,
                &sdlWavData,
                &len);
        if(hr == nullptr) {
            fwprintf(stderr, L"SDL_LoadWAV failed: %s\n", SDL_GetError());
            abort();
        }
        ASSERT(desired.freq == 44100 && (desired.format == AUDIO_F32SYS || desired.format == AUDIO_F32 || desired.format == AUDIO_F32LSB || desired.format == AUDIO_S16LSB) && desired.channels == 1,
                L"Expecting a mono sample at 44100Hz either in float32 format or signed 16bit little endian; got sample rate ", desired.freq,
                L", channels ", desired.channels,
                L" and format code ", desired.format);
        if(desired.format == AUDIO_S16LSB) {
            wav.resize(len / sizeof(int16_t));
            int16_t* shorts = (int16_t*)sdlWavData;
            for(size_t i = 0; i < len/sizeof(int16_t); ++i) {
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

#define RenderNew Render

void RenderNew(File f, std::wstring filename, bool split)
{
    std::map<std::wstring, std::vector<float>> data = LoadData(f);
    std::map<std::wstring, std::pair<std::vector<float>, std::vector<float>>> unmixed;
    size_t end = 0;

    for(auto&& track: f.samples) {
        size_t i = 0;
        auto&& leftData = unmixed[track.first].first;
        auto&& rightData = unmixed[track.first].second;
        float gain = 0.f;
        float volume = (float)track.second.volume / 100.f;
        auto&& mydata = data[track.first];
        std::remove_reference<decltype(mydata)>::type::iterator ptr = mydata.end();

        for(auto&& name: f.output) {
            auto&& phrase = f.phrases[name];
            size_t numSamplesPerBeat = 44100 * 60 / phrase.bpm;
            size_t numBeats = std::accumulate(
                    phrase.beats.begin(), phrase.beats.end(), (size_t)0,
                    [](size_t a, decltype(phrase.beats)::value_type b) -> size_t {
                        return std::max(a, b.second.size());
                    }
                    );
            size_t newestI = i + numBeats * numSamplesPerBeat;

            auto&& found = phrase.beats.find(track.first);
            if(found == phrase.beats.end()) {
				for (size_t k = 0; k < numSamplesPerBeat * numBeats && ptr != mydata.end(); ++k, ++ptr) {
						while (i + k >= leftData.size()) leftData.emplace_back();
						while (i + k >= rightData.size()) rightData.emplace_back();
                        auto data = track.second.effect->apply(gain * (*ptr) * volume);
						leftData[i + k] = data.data[0];
						rightData[i + k] = data.data[1];
				}
                auto startLeft = leftData.size();
                auto startRight = rightData.size();
                leftData.resize(i + numSamplesPerBeat * numBeats);
                rightData.resize(i + numSamplesPerBeat * numBeats);
                i += numSamplesPerBeat * numBeats;
                for(auto j = startLeft; j < i; ++j) {
                    leftData[j] = 0.f;
                }
                for(auto j = startRight; j < i; ++j) {
                    rightData[j] = 0.f;
                }
            } else {
                auto&& beats = phrase.beats[track.first];

                for(auto&& beat: beats) {
                    if(beat == File::Beat::REST
                            || beat == File::Beat::STOP) {
                        if(beat == File::Beat::STOP) {
                            ptr = mydata.end();
                            gain = 0.f;
                        }
                        for(size_t k = 0;
                                k < numSamplesPerBeat && ptr != mydata.end();
                                ++k, ++ptr)
                        {
                            while(i + k >= leftData.size()) leftData.emplace_back();
                            while(i + k >= rightData.size()) rightData.emplace_back();
                            auto data = track.second.effect->apply(gain * (*ptr) * volume);
                            leftData[i + k] = data.data[0];
                            rightData[i + k] = data.data[1];
                        }
                        i += numSamplesPerBeat;
                        continue;
                    } else if(beat == File::Beat::HALF) {
                        gain = 0.5f;
                    } else if(beat == File::Beat::FULL) {
                        gain = 1.f;
                    }

                    ptr = mydata.begin();
                    for(size_t k = 0;
                            k < numSamplesPerBeat && ptr != mydata.end();
                            ++k, ++ptr)
                    {
                        while(i + k >= leftData.size()) leftData.emplace_back();
                        while(i + k >= rightData.size()) rightData.emplace_back();
                        auto data = track.second.effect->apply(gain * (*ptr) * volume);
                        leftData[i + k] = data.data[0];
                        rightData[i + k] = data.data[1];
                    }
                    i += numSamplesPerBeat;
                }
            }

            i = newestI;
        }
    }

    extern void wav_write_file(std::wstring const&, std::vector<float> const&, unsigned, unsigned);

    if(split)
    {
        size_t maxLen = 0;
        maxLen = std::accumulate(unmixed.begin(), unmixed.end(), maxLen, [](size_t a, decltype(unmixed)::value_type const& b) -> size_t {
                    return std::max(a, std::max(b.second.first.size(), b.second.second.size()));
                });

        for(auto&& channel : unmixed) {
            std::wstringstream fnameBuilder;
            fnameBuilder << filename << L"_" << channel.first << L".wav";

            std::vector<float> outWAV(maxLen * 2, 0.f);

            for(size_t i = 0; i < maxLen; ++i) {
                if(i < channel.second.first.size())
                    outWAV[2 * i + 0] = tanhf(channel.second.first[i]);
                if(i < channel.second.second.size())
                    outWAV[2 * i + 1] = tanhf(channel.second.second[i]);
            }

            wav_write_file(fnameBuilder.str(), outWAV, 44100, 2);
        }
    }
    else
    {
        std::vector<float> left;
        std::vector<float> right;
        size_t maxLen = 0;
        maxLen = std::accumulate(unmixed.begin(), unmixed.end(), maxLen, [](size_t a, decltype(unmixed)::value_type const& b) -> size_t {
                    return std::max(a, std::max(b.second.first.size(), b.second.second.size()));
                });
        left.resize(maxLen);
        right.resize(maxLen);

        for(auto&& track: unmixed) {
            for(size_t i = 0; i < track.second.first.size(); ++i) {
                left[i] += track.second.first[i];
            }
            for(size_t i = 0; i < track.second.second.size(); ++i) {
                right[i] += track.second.second[i];
            }
        }

        std::for_each(left.begin(), left.end(), [](float& f) { f = tanhf(f); });
        std::for_each(right.begin(), right.end(), [](float& f) { f = tanhf(f); });

        while(left.size() < right.size()) left.emplace_back();
        while(right.size() < left.size()) right.emplace_back();

        std::vector<float> outWAV;

        for(size_t i = 0; i < left.size(); ++i) {
            outWAV.push_back(left[i]);
            outWAV.push_back(right[i]);
        }

        wav_write_file(filename, outWAV, 44100, 2);
    }
}

void RenderOld(File f, std::wstring filename, bool split)
{
    ASSERT(split == false, L"Split mode not supported in old renderer");
    auto&& data = LoadData(f);
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

    extern void wav_write_file(std::wstring const&, std::vector<float> const&, unsigned, unsigned);
    wav_write_file(filename, outWAV, 44100, 1);
}
