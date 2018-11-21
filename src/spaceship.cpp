#include "spaceship.h"
#include "lineNodeBuilder.h"
#include "controls.h"
#include "levelScene.h"
#include "grablingRope.h"
#include "explosion.h"

#include <sp2/random.h>
#include <sp2/tween.h>
#include <sp2/graphics/textureManager.h>


Spaceship::Spaceship(sp::P<sp::Node> parent)
: LevelObject(parent)
{
    LineNodeBuilder builder;
    builder.loadFrom("ship-a.json", 1.5);
    builder.create(this, LineNodeBuilder::CollisionType::Polygon);

    engine_emitter = new sp::ParticleEmitter(this, 16, sp::ParticleEmitter::Origin::Global);
}

void Spaceship::setControls(Controls* controls)
{
    this->controls = controls;
    if (controls)
    {
        switch(controls->index)
        {
        case 0: render_data.color = sp::Color(0.8, 0.8, 1); break;
        case 1: render_data.color = sp::Color(1, 0.8, 0.8); break;
        }
    }
    else
        render_data.color = sp::Color(1, 1, 1);
}

void Spaceship::onFixedUpdate()
{
    float turn_speed = 5;
    float trust_speed = 0.4;

    double angular_velocity = getAngularVelocity2D() * 0.95;
    sp::Vector2d velocity = getLinearVelocity2D();
    velocity += level_info.getGravityAt(getPosition2D());

    if (controls && alive)
    {
        float turn = controls->left.getValue() - controls->right.getValue();
        angular_velocity += turn * turn_speed;
        
        if (controls->primary_action.get())
        {
            activity = true;
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

            level_info.fuel_ticks_used += 1;
        }
        
        if (controls->secondary_action.getDown())
        {
            if (!rope)
            {
                rope = new GrablingRope(this);
                rope->setPosition(getPosition2D() + sp::Vector2d(0, -1).rotate(getRotation2D()));
                rope->setLinearVelocity(getLinearVelocity2D() + sp::Vector2d(0, -5).rotate(getRotation2D()));
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
        if (local_hit_normal.y > -0.7)
        {
            LOG(Debug, this, "HIT", info.force, local_hit_normal);
            explode();
        }
    }
}

void Spaceship::explode()
{
    if (!alive)
        return;
    new Explosion(getParent(), getPosition2D(), getLinearVelocity2D());
    engine_emitter->setParent(getParent());
    engine_emitter->auto_destroy = true;
    
    controls = nullptr;
    render_data.color.a = 0.5;
    alive = false;
}
