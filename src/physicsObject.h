#ifndef PHYSICS_OBJECT_H
#define PHYSICS_OBJECT_H

#include <sp2/scene/node.h>

class PhysicsObject : public sp::Node
{
public:
    PhysicsObject(sp::P<sp::Node> parent, sp::string name);

    virtual void onFixedUpdate() override;
};

#endif//PHYSICS_OBJECT_H