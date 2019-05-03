#ifndef VEHICLE_H
#define VEHICLE_H

#include "levelObject.h"
#include "wheel.h"
#include "controls.h"


class Wheel;
class Vehicle : public LevelObject
{
public:
    Vehicle(sp::P<sp::Node> parent, sp::string name);

    virtual void onFixedUpdate() override;
    
    virtual void setProperty(sp::string name, sp::string value) override;

    void addWheel(sp::P<Wheel> wheel) { wheels.add(wheel); }
    
    void setControlState(const PlayerControlsState& control_state);
    
    sp::string name;
private:
    sp::PList<Wheel> wheels;
};

#endif//VEHICLE_H
