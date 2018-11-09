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
