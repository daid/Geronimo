#include "triggerableNode.h"

void TriggerableNode::triggerAdd()
{
    if (once && trigger_counter)
        return;
    if (!trigger_counter)
        onTrigger();
    trigger_counter++;
}

void TriggerableNode::triggerSubstract()
{
    if (once)
        return;
    trigger_counter--;
    if (!trigger_counter)
        onUnTrigger();
}

void TriggerableNode::setProperty(sp::string name, sp::string value)
{
    if (name == "trigger_once")
        once = sp::stringutil::convert::toBool(value);
    else
        LevelObject::setProperty(name, value);
}
