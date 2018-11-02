#ifndef SPACESHIP_H
#define SPACESHIP_H

#include <sp2/scene/node.h>
#include <sp2/scene/particleEmitter.h>

class Controls;
class Spaceship : public sp::Node
{
public:
    Spaceship(sp::P<sp::Node> parent);
    
    virtual void onFixedUpdate() override;
    virtual void onUpdate(float delta) override;
    
    virtual void onCollision(sp::CollisionInfo& info) override;
    
    void explode();
    
    bool isAlive() { return alive; }
    
    void setIcon(sp::string name);
    
    sp::P<sp::Node> icon;
    Controls* controls = nullptr;
private:

    sp::Vector2d old_velocity;
    sp::P<sp::ParticleEmitter> engine_emitter;
    bool alive = true;
};


#endif//SPACESHIP_H
