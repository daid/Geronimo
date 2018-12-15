#ifndef SPACESHIP_H
#define SPACESHIP_H

#include "levelObject.h"
#include <sp2/scene/particleEmitter.h>
#include "controls.h"

class GrablingRope;
class Spaceship : public LevelObject
{
public:
    Spaceship(sp::P<sp::Node> parent);

    void setControlState(PlayerControlsState controlState);

    virtual void onFixedUpdate() override;
    virtual void onUpdate(float delta) override;
    
    virtual void onCollision(sp::CollisionInfo& info) override;
    
    void explode();
    
    bool isAlive() { return alive; }
    bool hadActivity() { return activity; }
    
    void setIcon(sp::string name);
    void setIndex(int idx);
    
    sp::P<sp::Node> icon;
    int index;
private:
    PlayerControlsState controlState;

    sp::Vector2d old_velocity;
    sp::P<sp::ParticleEmitter> engine_emitter;
    bool alive = true;
    bool activity = false;
    sp::P<GrablingRope> rope;

};


#endif//SPACESHIP_H
