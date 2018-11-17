#ifndef TIMER_H
#define TIMER_H

#include <sp2/scene/node.h>
#include <sp2/math/rect.h>


class Timer : public sp::Node
{
public:
    Timer(sp::P<sp::Node> parent, sp::string target);

    virtual void onFixedUpdate() override;

    int on_time = 60;
    int off_time = 60;
    int trigger_delay = 0;
private:
    sp::string target;

    bool active = false;
};

#endif//TRIGGER_H
