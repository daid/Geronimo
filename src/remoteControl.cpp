#include "remoteControl.h"
#include "vehicle.h"
#include "spaceship.h"
#include "levelScene.h"
#include "physicsObject.h"
#include "triggerableNode.h"


RemoteControl::RemoteControl(sp::P<sp::Node> parent, sp::Rect2d area, sp::string target)
: LevelObject(parent), area(area), target(target)
{
    active = false;
    trigger_delay = 60;
}

void RemoteControl::onFixedUpdate()
{
    bool has_object = false;
    sp::P<Spaceship> ship;
    for(auto obj : getParent()->getChildren())
    {
        ship = sp::P<sp::Node>(obj);
        if (ship && (area.contains(obj->getPosition2D()) && obj->getLinearVelocity2D().length() < 0.7))
        {
            has_object = true;
            break;
        }
    }
    if (has_object == active)
    {
        trigger_delay = 30;
        return;
    }

    if (trigger_delay > 0)
    {
        trigger_delay--;
        if (trigger_delay == 0)
        {
            active = !active;
            
            if (active && ship)
            {
                for(auto obj : getParent()->getChildren())
                {
                    sp::P<Vehicle> vehicle = sp::P<sp::Node>(obj);
                    if (vehicle && vehicle->name == target)
                        ship->setControllingVehicle(vehicle);
                }
            }
        }
    }
}

void RemoteControl::setProperty(sp::string name, sp::string value)
{
    LevelObject::setProperty(name, value);
}
