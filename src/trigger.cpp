#include "trigger.h"
#include "spaceship.h"
#include "levelScene.h"
#include "physicsObject.h"
#include "triggerableNode.h"


Trigger::Trigger(sp::P<sp::Node> parent, sp::Rect2d area, sp::string source, sp::string target)
: sp::Node(parent), area(area), source(source), target(target)
{
    initialized = false;
    active = false;
    trigger_delay = 60;
}

void Trigger::onFixedUpdate()
{
    if (!initialized)
    {
        initialized = true;

        if (source == "")
        {
            for(auto obj : getParent()->getChildren())
                if (sp::P<Spaceship>(sp::P<sp::Node>(obj)))
                    source_objects.add(obj);
        }
        else if (source == "OBJECT")
        {
            for(auto obj : getParent()->getChildren())
                if (sp::P<PhysicsObject>(sp::P<sp::Node>(obj)))
                    source_objects.add(obj);
        }
        else
        {
            LOG(Error, "Unknown trigger source:", source);
        }
    }
    
    bool has_object = false;
    for(auto obj : source_objects)
    {
        if ((area.contains(obj->getPosition2D()) && obj->getLinearVelocity2D().length() < 0.5))
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
            for(auto obj : getParent()->getChildren())
            {
                sp::P<TriggerableNode> t = sp::P<sp::Node>(obj);
                if (t && t->getTriggerName() == target)
                {
                    if (active)
                        t->onTrigger();
                    else
                        t->onUnTrigger();
                }
            }
        }
    }
}
