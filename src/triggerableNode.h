#ifndef TRIGGERABLENODE_H
#define TRIGGERABLENODE_H

#include "levelObject.h"
#include <sp2/math/rect.h>


class TriggerableNode : public LevelObject
{
public:
    TriggerableNode(sp::P<sp::Node> parent, sp::string trigger_name)
    : LevelObject(parent), trigger_name(trigger_name)
    {
    }

    const sp::string& getTriggerName() { return trigger_name; }
    
    virtual void setProperty(sp::string name, sp::string value) override;

    void triggerAdd();
    void triggerSubstract();
protected:
    virtual void onTrigger() = 0;
    virtual void onUnTrigger() {}
private:
    int trigger_counter = 0; //Currently active triggers.
    int trigger_level = 1; //Amount of active triggers required for this triggerable to activate.
    sp::string trigger_name;
    bool once = false;
};

#endif//TRIGGER_H

