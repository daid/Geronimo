#ifndef WHEEL_H
#define WHEEL_H

#include "levelObject.h"


class Wheel : public LevelObject
{
public:
    Wheel(sp::P<sp::Node> parent, sp::Vector2d position, float radius);

private:
};

#endif//WHEEL_H
