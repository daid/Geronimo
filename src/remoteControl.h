#ifndef REMOTE_CONTROL_H
#define REMOTE_CONTROL_H

#include "levelObject.h"
#include <sp2/math/rect.h>


class RemoteControl : public LevelObject
{
public:
    RemoteControl(sp::P<sp::Node> parent, sp::Rect2d area, sp::string target);

    virtual void onFixedUpdate() override;

    virtual void setProperty(sp::string name, sp::string value) override;
private:
    sp::Rect2d area;
    sp::string target;

    int trigger_delay;
    bool active;
};

#endif//REMOTE_CONTROL_H
