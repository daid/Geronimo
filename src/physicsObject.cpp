#include "physicsObject.h"

#include "levelScene.h"
#include "lineNodeBuilder.h"


PhysicsObject::PhysicsObject(sp::P<sp::Node> parent, sp::string name)
: LevelObject(parent)
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

void PhysicsObject::setProperty(sp::string name, sp::string value)
{
    if (name == "GOAL" && value == "TARGET")
        is_goal = true;
    else if (name == "velocity")
        setLinearVelocity(sp::stringutil::convert::toVector2d(value));
    else if (name == "angular_velocity")
        setAngularVelocity(sp::stringutil::convert::toFloat(value));
    else
        LevelObject::setProperty(name, value);
}
