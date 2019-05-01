#ifndef LASER_H
#define LASER_H

#include "triggerableNode.h"
#include <sp2/scene/particleEmitter.h>


class Laser : public TriggerableNode
{
public:
    Laser(sp::P<sp::Node> parent, sp::string trigger);

    virtual void onTrigger() override;
    virtual void onUnTrigger() override;
    virtual void onFixedUpdate() override;

    virtual void setProperty(sp::string name, sp::string value) override;
protected:
    virtual void onHit(sp::P<sp::Node> object, sp::Vector2d hit_location);
private:
    bool active;
    int sparkdelay;
    
    sp::P<sp::ParticleEmitter> spark_emitter;
};

#endif//LASER_H
