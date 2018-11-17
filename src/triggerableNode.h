#ifndef TRIGGERABLENODE_H
#define TRIGGERABLENODE_H

#include <sp2/scene/node.h>
#include <sp2/math/rect.h>


class TriggerableNode : public sp::Node
{
public:
    TriggerableNode(sp::P<sp::Node> parent, sp::string trigger_name)
    : sp::Node(parent), trigger_name(trigger_name)
    {
    }

    const sp::string& getTriggerName() { return trigger_name; }

    void triggerAdd();
    void triggerSubstract();
protected:
    virtual void onTrigger() = 0;
    virtual void onUnTrigger() {}
private:
    int trigger_counter = 0;
    sp::string trigger_name;
};

#endif//TRIGGER_H

