#ifndef PUSH_LASER_H
#define PUSH_LASER_H

#include "laser.h"

class PushLaser : public Laser
{
public:
    PushLaser(sp::P<sp::Node> parent, sp::string trigger);
    
protected:
    virtual void onHit(sp::P<sp::Node> object, sp::Vector2d hit_location) override;
};

#endif//PUSH_LASER_H
