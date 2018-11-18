#include "triggerableNode.h"

void TriggerableNode::triggerAdd()
{
    if (!trigger_counter)
        onTrigger();
    trigger_counter++;
}

void TriggerableNode::triggerSubstract()
{
    trigger_counter--;
    if (!trigger_counter)
        onUnTrigger();
}

void TriggerableNode::setProperty(sp::string name, sp::string value)
{
    LevelObject::setProperty(name, value);
}
