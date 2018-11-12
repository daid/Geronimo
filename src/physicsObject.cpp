#include "physicsObject.h"

#include "levelScene.h"
#include "lineNodeBuilder.h"


PhysicsObject::PhysicsObject(sp::P<sp::Node> parent, sp::string name)
: sp::Node(parent)
{
    LineNodeBuilder builder;
    builder.loadFrom(name + ".json", 1.5);
    builder.create(this, LineNodeBuilder::CollisionType::Polygon);
}

void PhysicsObject::onFixedUpdate()
{
    sp::Vector2d velocity = getLinearVelocity2D();
    velocity += level_info.getGravityAt(getPosition2D());
    setLinearVelocity(velocity);
}
