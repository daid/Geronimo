#include "physicsObject.h"

#include "main.h"
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
    velocity += gravity;
    setLinearVelocity(velocity);
}
