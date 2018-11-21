#ifndef BOMB_H
#define BOMB_H

#include "physicsObject.h"


class Bomb : public PhysicsObject
{
public:
    Bomb(sp::P<sp::Node> parent, sp::string name);

    virtual void onFixedUpdate() override;
    
    virtual void setProperty(sp::string name, sp::string value) override;

    virtual bool isTriggerSource() override { return false; }
private:
    double velocity_average = 0.0;
    enum class State
    {
        Inactive,
        Triggered,
        Delay
    } state = State::Inactive;
    int state_timer = 0;
    int explosion_delay = 60 * 15;
    double explosion_power = 2500.0; //Range of "kill": sqrt(explosion_power / 10.0)
};

#endif//BOMB_H
