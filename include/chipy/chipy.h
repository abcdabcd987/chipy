#pragma once

#include "Interpreter.h"
#include <json/json.h>

namespace chipy
{

json::Document value_to_document(ValuePtr val);

BitStream compile_file(const std::string &filename);
BitStream compile_code(const std::string &code);

}
