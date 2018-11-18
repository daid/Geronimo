#ifndef DOOR_H
#define DOOR_H

#include "triggerableNode.h"

class Door : public TriggerableNode
{
public:
    Door(sp::P<sp::Node> parent, sp::string trigger);

    virtual void onTrigger() override;
    virtual void onUnTrigger() override;
    virtual void onFixedUpdate() override;

    virtual void setProperty(sp::string name, sp::string value) override;
    
    sp::Vector2d opened_position;
private:
    sp::Vector2d target_position;
};

#endif//DOOR_H
