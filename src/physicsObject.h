#ifndef PHYSICS_OBJECT_H
#define PHYSICS_OBJECT_H

#include "levelObject.h"

class PhysicsObject : public LevelObject
{
public:
    PhysicsObject(sp::P<sp::Node> parent, sp::string name);

    virtual void onFixedUpdate() override;
    
    virtual void setProperty(sp::string name, sp::string value) override;
    
    bool isGoal() { return is_goal; }

    virtual bool isTriggerSource() { return true; }
private:
    bool is_goal = false;
};

#endif//PHYSICS_OBJECT_H
