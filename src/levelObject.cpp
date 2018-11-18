#include "levelObject.h"

#include <sp2/logging.h>

LevelObject::LevelObject(sp::P<sp::Node> parent)
: sp::Node(parent)
{
}

void LevelObject::setProperty(sp::string name, sp::string value)
{
    LOG(Warning, "Setting unknown propert", name, "to", value, "on", this);
}
