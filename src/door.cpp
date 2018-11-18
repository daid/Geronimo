#include "door.h"

Door::Door(sp::P<sp::Node> parent, sp::string trigger)
: TriggerableNode(parent, trigger)
{
    opened_position = sp::Vector2d(20, 0);
}

void Door::onTrigger()
{
    target_position = opened_position;
}

void Door::onUnTrigger()
{
    target_position = sp::Vector2d(0, 0);
}

void Door::onFixedUpdate()
{
    setPosition(getPosition2D() * 0.9 + target_position * 0.1);
}

void Door::setProperty(sp::string name, sp::string value)
{
    if (name == "offset")
        opened_position = sp::stringutil::convert::toVector2d(value);
    else
        TriggerableNode::setProperty(name, value);
}
