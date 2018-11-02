#include "grablingRope.h"
#include "main.h"
#include <sp2/collision/2d/circle.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>


GrablingRope::GrablingRope(sp::P<sp::Node> owner)
: sp::Node(owner->getParent()), owner(owner)
{
    sp::collision::Circle2D shape(0.5);
    shape.type = sp::collision::Shape::Type::Sensor;
    setCollisionShape(shape);
    setLinearVelocity(owner->getLinearVelocity2D());
    
    rope_joint = new sp::collision::RopeJoint2D(owner, sp::Vector2d(0, 0), this, sp::Vector2d(0, 0), 10.0);

    sp::MeshData::Vertices vertices;
    sp::MeshData::Indices indices{0,1,2,2,1,3};
    vertices.reserve(4);
    
    vertices.emplace_back(sp::Vector3f(0,-1, 0), sp::Vector2f(1, 0));
    vertices.emplace_back(sp::Vector3f(1,-1, 0), sp::Vector2f(1, 1));
    vertices.emplace_back(sp::Vector3f(0, 1, 0), sp::Vector2f(0, 0));
    vertices.emplace_back(sp::Vector3f(1, 1, 0), sp::Vector2f(0, 1));

    render_data.type = sp::RenderData::Type::Additive;
    render_data.shader = sp::Shader::get("rope.shader");
    render_data.mesh = sp::MeshData::create(std::move(vertices), std::move(indices));
    render_data.texture = sp::texture_manager.get("line.png");
}

GrablingRope::~GrablingRope()
{
    rope_joint.destroy();
}

void GrablingRope::onUpdate(float delta)
{
    if (!owner || !rope_joint)
        return;

    if (target)
        setPosition(target->getGlobalPoint2D(target_local_location));
    sp::Vector2d rope_vector = owner->getPosition2D() - getGlobalPosition2D();
    render_data.scale.x = rope_vector.x;
    render_data.scale.y = rope_vector.y;
}

void GrablingRope::onFixedUpdate()
{
    if (!owner)
    {
        delete this;
        return;
    }
    sp::Vector2d velocity = getLinearVelocity2D();
    velocity *= 0.99;
    velocity += gravity;
    setLinearVelocity(velocity);
}

void GrablingRope::onCollision(sp::CollisionInfo& info)
{
    if (info.other != owner)
    {
        removeCollisionShape();
        
        target = info.other;
        target_local_location = (info.position - info.other->getPosition2D()).rotate(-target->getRotation2D());
        rope_joint = new sp::collision::RopeJoint2D(owner, sp::Vector2d(0, 0), target, target_local_location, 10.0);
    }
}
