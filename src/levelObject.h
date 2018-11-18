#ifndef LEVEL_OBJECT_H
#define LEVEL_OBJECT_H

#include <sp2/scene/node.h>

class LevelObject : public sp::Node
{
public:
    LevelObject(sp::P<sp::Node> parent);

    virtual void setProperty(sp::string name, sp::string value);
};

#endif//LEVEL_OBJECT_H
