#include "pushLaser.h"

PushLaser::PushLaser(sp::P<sp::Node> parent, sp::string trigger)
: Laser(parent, trigger)
{
    render_data.color = sp::Color(0.5, 0.5, 1.0);
}

void PushLaser::onHit(sp::P<sp::Node> object, sp::Vector2d hit_location)
{
    object->setLinearVelocity(object->getLinearVelocity2D() + aim_vector * 0.0007);
}
