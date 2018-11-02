#ifndef GRABLING_ROPE_H
#define GRABLING_ROPE_H

#include <sp2/scene/node.h>
#include <sp2/collision/2d/ropejoint.h>

class GrablingRope : public sp::Node
{
public:
    GrablingRope(sp::P<sp::Node> owner);
    virtual ~GrablingRope();
    
    virtual void onUpdate(float delta) override;
    virtual void onFixedUpdate() override;

    virtual void onCollision(sp::CollisionInfo& info) override;
private:
    sp::P<sp::Node> owner;
    sp::P<sp::collision::RopeJoint2D> rope_joint;
    sp::P<sp::Node> target;
    sp::Vector2d target_local_location;
};

#endif//GRABLING_ROPE_H
