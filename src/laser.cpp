#include "laser.h"
#include "spaceship.h"

#include <sp2/random.h>
#include <sp2/scene/scene.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/textureManager.h>

Laser::Laser(sp::P<sp::Node> parent, sp::string trigger)
: TriggerableNode(parent, trigger)
{
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
    render_data.color = sp::Color(1, 0.5, 0.5);
    
    spark_emitter = new sp::ParticleEmitter(this, 16, sp::ParticleEmitter::Origin::Global);
    spark_emitter->addEffector<sp::ParticleEmitter::SizeEffector>(3.0, 5.0);
    spark_emitter->addEffector<sp::ParticleEmitter::VelocityScaleEffector>(0.0, -10.0);

    active = true;
    sparkdelay = 0;
}

void Laser::onTrigger()
{
    active = false;
    render_data.type = sp::RenderData::Type::None;
}

void Laser::onUnTrigger()
{
    active = true;
    render_data.type = sp::RenderData::Type::Additive;
}

void Laser::onFixedUpdate()
{
    if (!active)
        return;

    sp::Vector2d position = getGlobalPosition2D();
    double rotation = getGlobalRotation2D();
    sp::Vector2d laser_vector = sp::Vector2d(0, 500).rotate(rotation);
    getScene()->queryCollisionAll(sp::Ray2d(position + laser_vector / 5000.0, position + laser_vector), [this, &laser_vector, position](sp::P<sp::Node> object, sp::Vector2d hit_location, sp::Vector2d hit_normal)
    {
        if (!object->isSolid())
            return true;
        laser_vector = hit_location - position;
        onHit(object, hit_location);
        return false;
    });
    
    if (sparkdelay > 0)
    {
        sparkdelay--;
    }
    else
    {
        sp::ParticleEmitter::Parameters parameters;
        parameters.position.x = laser_vector.x;
        parameters.position.y = laser_vector.y;
        parameters.velocity.x = sp::random(-7, 7);
        parameters.velocity.y = sp::random(-7, 7);
        parameters.color = render_data.color;

        parameters.lifetime = 0.5;
        spark_emitter->emit(parameters);
        sparkdelay = 2;
    }

    laser_vector = laser_vector.rotate(-rotation);
    render_data.scale.x = laser_vector.x;
    render_data.scale.y = laser_vector.y;
}

void Laser::setProperty(sp::string name, sp::string value)
{
    if (name == "angle")
        setRotation(sp::stringutil::convert::toFloat(value));
    else
        TriggerableNode::setProperty(name, value);
}

void Laser::onHit(sp::P<sp::Node> object, sp::Vector2d hit_location)
{
    if (sp::P<Spaceship>(object))
        (sp::P<Spaceship>(object))->explode();
}
