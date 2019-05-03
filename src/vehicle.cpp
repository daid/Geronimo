#include "vehicle.h"
#include "wheel.h"

#include "levelScene.h"
#include "lineNodeBuilder.h"
#include "main.h"

Vehicle::Vehicle(sp::P<sp::Node> parent, sp::string name)
: LevelObject(parent), name(name)
{
    LineNodeBuilder builder;
    builder.loadFrom(name + ".json", 1.5);
    builder.create(this, LineNodeBuilder::CollisionType::Polygon);
}

void Vehicle::onFixedUpdate()
{
    sp::Vector2d velocity = getLinearVelocity2D();
    velocity += level_info.getGravityAt(getPosition2D());
    setLinearVelocity(velocity);
}

void Vehicle::setProperty(sp::string name, sp::string value)
{
    if (name == "velocity")
        setLinearVelocity(sp::stringutil::convert::toVector2d(value));
    else if (name == "angular_velocity")
        setAngularVelocity(sp::stringutil::convert::toFloat(value));
    else if (name == "name")
        this->name = name;
    else
        LevelObject::setProperty(name, value);
}

void Vehicle::setControlState(const PlayerControlsState& control_state)
{
    float move = control_state.left.value - control_state.right.value;
    if (move)
    {
        for(Wheel* w : wheels)
            w->setAngularVelocity(move * 200.0);
    }
}
