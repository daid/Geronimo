#include "wheel.h"
#include "lineNodeBuilder.h"
#include "vehicle.h"

#include <sp2/scene/scene.h>
#include <sp2/collision/2d/circle.h>
#include <sp2/collision/2d/revolutejoint.h>


Wheel::Wheel(sp::P<sp::Node> parent, sp::Vector2d position, float radius)
: LevelObject(parent->getScene()->getRoot())
{
    setPosition(parent->getPosition2D() + position);
    sp::collision::Circle2D shape(radius);
    shape.friction = 10.0;
    shape.angular_damping = 1.0;
    setCollisionShape(shape);
    
    LineNodeBuilder builder;
    std::vector<sp::Vector2f> circle_points;
    for(int m=0; m<15; m+=5)
    {
        circle_points.clear();
        circle_points.push_back(sp::Vector2f(0, 0));
        for(int n=0; n<=5; n++)
        {
            circle_points.push_back(sp::Vector2f(radius, 0).rotate(float(m + n) / 15.0 * 360.0));
        }
        builder.addLoop(circle_points, 1.5);
    }
    builder.create(this, LineNodeBuilder::CollisionType::None);
    
    new sp::collision::RevoluteJoint2D(parent, position, this, sp::Vector2d(0, 0));
    
    sp::P<Vehicle> vehicle = parent;
    if (vehicle)
        vehicle->addWheel(this);
}
