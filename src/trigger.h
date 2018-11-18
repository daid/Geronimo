#ifndef TRIGGER_H
#define TRIGGER_H

#include "levelObject.h"
#include <sp2/math/rect.h>


class Trigger : public LevelObject
{
public:
    Trigger(sp::P<sp::Node> parent, sp::Rect2d area, sp::string source, sp::string target);

    virtual void onFixedUpdate() override;

    virtual void setProperty(sp::string name, sp::string value) override;
private:
    sp::Rect2d area;
    sp::string source;
    sp::string target;

    bool initialized;
    sp::PList<sp::Node> source_objects;
    int trigger_delay;
    bool active;
};

#endif//TRIGGER_H
