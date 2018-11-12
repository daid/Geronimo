#ifndef SPACESHIP_H
#define SPACESHIP_H

#include <sp2/scene/node.h>
#include <sp2/scene/particleEmitter.h>

class Controls;
class GrablingRope;
class Spaceship : public sp::Node
{
public:
    Spaceship(sp::P<sp::Node> parent);
    
    void setControls(Controls* controls);
    
    virtual void onFixedUpdate() override;
    virtual void onUpdate(float delta) override;
    
    virtual void onCollision(sp::CollisionInfo& info) override;
    
    void explode();
    
    bool isAlive() { return alive; }
    bool hadActivity() { return activity; }
    
    void setIcon(sp::string name);
    
    sp::P<sp::Node> icon;
private:
    Controls* controls = nullptr;

    sp::Vector2d old_velocity;
    sp::P<sp::ParticleEmitter> engine_emitter;
    bool alive = true;
    bool activity = false;
    sp::P<GrablingRope> rope;
};


#endif//SPACESHIP_H
