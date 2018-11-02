#include "spaceship.h"
#include "lineNodeBuilder.h"
#include "controls.h"
#include "main.h"
#include "grablingRope.h"

#include <sp2/random.h>
#include <sp2/graphics/textureManager.h>


Spaceship::Spaceship(sp::P<sp::Node> parent)
: sp::Node(parent)
{
    LineNodeBuilder builder;
    builder.loadFrom("ship-a.json", 1.5);
    builder.create(this, LineNodeBuilder::CollisionType::Polygon);

    engine_emitter = new sp::ParticleEmitter(this, 16, sp::ParticleEmitter::Origin::Global);
}

void Spaceship::onFixedUpdate()
{
    float turn_speed = 5;
    float trust_speed = 0.4;

    double angular_velocity = getAngularVelocity2D() * 0.95;
    sp::Vector2d velocity = getLinearVelocity2D();
    velocity += gravity;

    if (controls && alive)
    {
        float turn = controls->left.getValue() - controls->right.getValue();
        angular_velocity += turn * turn_speed;
        
        if (controls->primary_action.get())
        {
            sp::Vector2d trust_vector = sp::Vector2d(0, trust_speed).rotate(getRotation2D());
            velocity += trust_vector;
            
            sp::Vector2f particle_vector = sp::Vector2f(sp::random(-0.2, 0.2), 1).rotate(getRotation2D()) * sp::random(0.9, 1.1);
            sp::ParticleEmitter::Parameters parameters;
            parameters.position.x = -particle_vector.x;
            parameters.position.y = -particle_vector.y;
            parameters.velocity.x = velocity.x + particle_vector.x * -30;
            parameters.velocity.y = velocity.y + particle_vector.y * -30;
            parameters.acceleration = -parameters.velocity;
            //parameters.start_color = sp::Color(1, 1, 0);
            //parameters.end_color = sp::Color(1, 0, 0, 0);
            parameters.start_color = render_data.color;
            parameters.end_color = parameters.start_color;
            parameters.end_color.a = 0;

            parameters.start_size = 1.0;
            parameters.end_size = 4.0;
            parameters.lifetime = 0.3;
            engine_emitter->emit(parameters);
        }
        
        if (controls->secondary_action.getDown())
        {
            if (!rope)
            {
                rope = new GrablingRope(this);
                rope->setPosition(getPosition2D() + sp::Vector2d(0, -1).rotate(getRotation2D()));
            }
            else
            {
                rope.destroy();
            }
        }
    }
    
    setLinearVelocity(velocity);
    setAngularVelocity(angular_velocity);
    
    if (alive)
    {
        double velo_diff = (velocity - old_velocity).length();
        if (velo_diff > 10.0)
        {
            LOG(Debug, "VELO_DIFF", velo_diff);
            explode();
            return;
        }
    }

    old_velocity = velocity;
}

void Spaceship::onUpdate(float delta)
{
    if (icon)
    {
        if (icon->render_data.type != sp::RenderData::Type::None)
        {
            icon->setPosition(getPosition2D() + sp::Vector2d(0, 6));
            icon->render_data.color.a -= delta * 0.3;
            if (icon->render_data.color.a < 0.0)
                icon->render_data.type = sp::RenderData::Type::None;
        }
    }
}

void Spaceship::setIcon(sp::string name)
{
    if (icon)
    {
        icon->render_data.texture = sp::texture_manager.get("gui/icons/" + name + ".png");
        icon->render_data.color.a = 0.8;
        icon->render_data.type = sp::RenderData::Type::Additive;
    }
}

void Spaceship::onCollision(sp::CollisionInfo& info)
{
    if (!alive)
        return;
    if (info.force > 0.0)
    {
        sp::Vector2d local_hit_normal = info.normal.rotate(-getRotation2D());
        if (local_hit_normal.y > -0.9)
        {
            LOG(Debug, this, "HIT", info.force, local_hit_normal);
            explode();
        }
    }
}

void Spaceship::explode()
{
    sp::P<sp::ParticleEmitter> pe = new sp::ParticleEmitter(getParent(), 16, sp::ParticleEmitter::Origin::Local);
    pe->setPosition(getPosition2D());
    pe->auto_destroy = true;
    for(int n=0; n<64; n++)
    {
        sp::Vector2f velocity = sp::Vector2f(getLinearVelocity2D()) + sp::Vector2f(sp::random(1, 40), 0).rotate(sp::random(0, 360));
        sp::ParticleEmitter::Parameters parameters;
        parameters.velocity.x = velocity.x;
        parameters.velocity.y = velocity.y;
        parameters.acceleration = parameters.velocity;
        parameters.acceleration.y += -100.0;
        parameters.start_color = render_data.color;
        parameters.end_color = parameters.start_color;
        parameters.end_color.a = 0;

        parameters.start_size = 4.0;
        parameters.end_size = 10.0;
        parameters.lifetime = sp::random(0.1, 0.3);
        pe->emit(parameters);
    }
    engine_emitter->setParent(getParent());
    engine_emitter->auto_destroy = true;
    
    controls = nullptr;
    render_data.color.a = 0.5;
    alive = false;
}
