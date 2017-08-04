#include "chipy/Module.h"

namespace chipy
{

class RandModule : public Module
{
public:
    using Module::Module;

    ValuePtr get_member(const std::string &name)  override;
};

}
