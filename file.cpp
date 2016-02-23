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
#include <parser_types.h>
#include <errorassert.h>
#include <string.h>

static void AddWho(File* f, Section* s)
{
    for(auto&& o: s->options) {
        auto& sample = f->samples[o->name];
        ASSERT(o->value->GetType() == IValue::LIST, "Expecting a list of parameters for sample declaration ", o->name);
        for(auto&& o2: ((List*)o->value)->values) {
            ASSERT(o2->GetType() == IValue::OPTION, "Expecting key value pairs for ", o->name);
            Option* o = (Option*)o2;
            if(o->name.compare("path") == 0) {
                auto&& val = o->value;
                ASSERT(val->GetType() == IValue::SCALAR, "Expecting path to be a string");
                sample.path = ((Scalar*)val)->value;
            } else if(o->name.compare("volume") == 0) {
                auto&& val = o->value;
                ASSERT(val->GetType() == IValue::SCALAR, "Expecting volume to be a scalar");
                sample.volume = atoi(((Scalar*)val)->value.c_str());
            } else if(o->name.compare("stereo") == 0) {
                auto&& val = o->value;
                ASSERT(val->GetType() == IValue::SCALAR, "Expecting stereo to be a scalar");
                sample.effect->name = ((Scalar*)val)->value;
            } else if(o->name.compare("params") == 0) {
                auto&& val = o->value;
                sample.effect->params.reset(val->Clone());
            } else {
                ASSERT(o->name == "volume" || o->name == "path" || o->name == "stereo" || o->name == "params", "Unknown parameter ", o->name);
            }
        }
    }
}

static void AddWhat(File* f, Section* s)
{
    for(auto&& o: s->options) {
        if(o->name.compare("Output") == 0) {
            IValue* v = o->value;
            switch(v->GetType()) {
                case IValue::SCALAR:
                    f->output.push_back(((Scalar*)v)->value);
                    break;
                case IValue::LIST:
                    {
                        for(auto&& o: ((List*)v)->values) {
                            ASSERT(o->GetType() == IValue::SCALAR, "Expecting a list of options");
                            f->output.push_back(((Scalar*)o)->value);
                        }
                    }
            }
        } else {
            auto& phrase = f->phrases[o->name];
            ASSERT(o->value->GetType() == IValue::LIST, "Expecting a list of options for ", o->name);
            for(auto& o2: ((List*)o->value)->values) {
                ASSERT(o2->GetType() == IValue::OPTION, "Expecting a list of options for ", o->name);
                auto o = (Option*)o2;
                auto name = o->name;
                if(name.compare("bpm") == 0) {
                    ASSERT(o->value->GetType() == IValue::SCALAR, "Expecting bpm to be a scalar");
                    phrase.bpm = atoi(((Scalar*)o->value)->value.c_str());
                } else {
                    ASSERT(name == "bpm", "Unknown parameter ", name);
                }
            }
        }
    }
}

static void AddBeats(File* f, Section* s)
{
    auto& phrase = f->phrases[s->name];
    for(auto&& o: s->options) {
        auto& beats = phrase.beats[o->name];
        switch(o->value->GetType()) {
        case IValue::SCALAR:
            for(char c: ((Scalar*)o->value)->value) {
                switch(c) {
                case '.':
                    beats.push_back(File::Beat::REST);
                    break;
                case '-':
                    beats.push_back(File::Beat::HALF);
                    break;
                case '!':
                    beats.push_back(File::Beat::FULL);
                    break;
                case '_':    
                    beats.push_back(File::Beat::STOP);
                    break;
                case 10:
                case 13:
                case '\t':
                case ' ':
                    break;
                default:
                    ASSERT(c == '.' || c == '-' || c == '!' || c == '_', "Unknown beat character ", c);
                    break;
                }
            }
            break;
        default:
            ASSERT("Not implemented" == NULL);
            break;
        }
    }
}

void File::Add(Section* s)
{
    if(s->name.compare("WHO") == 0) AddWho(this, s);
    else if(s->name.compare("WHAT") == 0) AddWhat(this, s);
    else AddBeats(this, s); 
    delete s;
}
