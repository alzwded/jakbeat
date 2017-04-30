#ifndef FILE_H
#define FILE_H

#include <parser_types.h>
#include "model.h"

struct File
{
    void Add(Section*);

    std::shared_ptr<Model> model;
};

#endif
