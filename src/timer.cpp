#include "timer.h"

#include "trigger.h"
#include "spaceship.h"
#include "levelScene.h"
#include "physicsObject.h"
#include "triggerableNode.h"


Timer::Timer(sp::P<sp::Node> parent, sp::string target)
: LevelObject(parent), target(target)
{
}

void Timer::onFixedUpdate()
{
    if (trigger_delay > 0)
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
                    t->triggerAdd();
                else
                    t->triggerSubstract();
            }
        }
        
        if (active)
            trigger_delay = off_time;
        else
            trigger_delay = on_time;
    }
}

void Timer::setProperty(sp::string name, sp::string value)
{
    if (name == "delay")
        on_time = off_time = sp::stringutil::convert::toFloat(value) * 60.0;
    else if (name == "on_delay")
        on_time = sp::stringutil::convert::toFloat(value) * 60.0;
    else if (name == "off_delay")
        off_time = sp::stringutil::convert::toFloat(value) * 60.0;
    else if (name == "start_delay")
        trigger_delay = sp::stringutil::convert::toFloat(value) * 60.0;
    else
        LevelObject::setProperty(name, value);
}
