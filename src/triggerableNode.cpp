#include "triggerableNode.h"

void TriggerableNode::triggerAdd()
{
    if (once && trigger_counter == trigger_level)
        return;
    trigger_counter++;

    if (trigger_counter == trigger_level)
        onTrigger();
}

void TriggerableNode::triggerSubstract()
{
    if (once && trigger_counter == trigger_level)
        return;
    
    if (trigger_counter == trigger_level)
        onUnTrigger();
    trigger_counter--;
}

void TriggerableNode::setProperty(sp::string name, sp::string value)
{
    if (name == "trigger_once")
        once = sp::stringutil::convert::toBool(value);
    else if (name == "trigger_level")
        trigger_level = sp::stringutil::convert::toInt(value);
    else
        LevelObject::setProperty(name, value);
}
