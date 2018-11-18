#ifndef TIMER_H
#define TIMER_H

#include "levelObject.h"
#include <sp2/math/rect.h>


class Timer : public LevelObject
{
public:
    Timer(sp::P<sp::Node> parent, sp::string target);

    virtual void onFixedUpdate() override;

    virtual void setProperty(sp::string name, sp::string value) override;
    
private:
    sp::string target;

    int on_time = 60;
    int off_time = 60;
    int trigger_delay = 0;

    bool active = false;
};

#endif//TRIGGER_H
