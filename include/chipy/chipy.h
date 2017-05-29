#pragma once

#include "Interpreter.h"

namespace chipy
{

BitStream compile_file(const std::string &filename);
BitStream compile_code(const std::string &code);

}
