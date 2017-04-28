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
#ifndef PARSER_TYPES_H
#define PARSER_TYPES_H

#include <vector>
#include <map>
#include <memory>
#include <functional>
#include <stereo.h>

struct IValue
{
    typedef enum {
        SCALAR, LIST, OPTION
    } Type;
    virtual IValue* Clone() = 0;
    virtual Type GetType() const = 0;
    virtual ~IValue() {}
};

struct List : IValue
{
    IValue* Clone() override
    {
        List* clone = new List();
        for(auto&& v: values) {
            clone->values.push_back(v->Clone());
        }
        return clone;
    }

    IValue::Type GetType() const override { return IValue::LIST; }
    std::vector<IValue*> values;

    virtual ~List()
    {
        for(auto&& v : values) delete v;
    }
};

struct Scalar : IValue
{
    IValue* Clone() override { return new Scalar(value.c_str()); }
    IValue::Type GetType() const override { return IValue::SCALAR; }
    std::string value;
    Scalar(char* s)
    {
        value.assign(s);
        free(s);
    }
    Scalar(char const* s) { value.assign(s); }
};

struct Option : IValue
{
    IValue* Clone() override { return new Option(name.c_str(), value->Clone()); }
    IValue::Type GetType() const override { return IValue::OPTION; }
    std::string name;
    IValue* value;
    Option(char* name_, IValue* value_)
    {
        name.assign(name_);
        free(name_);
        value = value_;
    }
    Option(char const* name_, IValue* value_)
    {
        name.assign(name_);
        value = value_;
    }
    virtual ~Option()
    {
        delete value;
    }
};

struct Section
{
    std::string name;
    std::vector<Option*> options;

    void SetName(char* s)
    {
        name.assign(s);
        free(s);
    }

    ~Section()
    {
        for(auto&& o : options) {
            delete o;
        }
    }
};

struct File
{
    void Add(Section*);

    struct Sample
    {
        struct Effect {
            std::string name;
            std::shared_ptr<IValue> params;
            std::function<stereo_sample_t(float)> apply;
            std::function<StereoInstance*()> getInstance;

            Effect()
                : params(nullptr)
                  , name("")
            {
                this->apply = [this](float mono) -> stereo_sample_t {
                    auto instance = std::shared_ptr<StereoInstance>(NewStereoInstance(name, params.get()));
                    this->apply = [instance](float mono) -> stereo_sample_t {
                        return (*instance)(mono);
                    };
                    return (*instance)(mono);
                };
            }

            stereo_sample_t operator()(float mono)
            {
                auto inst = getInstance();
                return (*inst)(mono);
            }
        };

        int volume;
        std::string path;
        std::shared_ptr<Effect> effect = decltype(effect)(new Effect()); // pointer because iterating over a map copies this whole thing (for some reason)
    };

    enum class Beat {
        REST, HALF, FULL, STOP
    };

    struct Phrase
    {
        int bpm = 120;
        std::map<std::string, std::vector<Beat>> beats;
    };

    std::map<std::string, Sample> samples;
    std::map<std::string, Phrase> phrases;
    std::vector<std::string> output;
};

#endif
