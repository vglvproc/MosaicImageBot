#pragma once
#include <map>
#include <docopt/docopt.h>
#include <string>

class Command {
public:
    virtual ~Command() = default;
};