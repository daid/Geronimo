#include "explosion.h"
#include "levelScene.h"

#include <sp2/random.h>
#include <sp2/tween.h>


Explosion::Explosion(sp::P<sp::Node> parent, sp::Vector2d position, sp::Vector2d start_velocity, sp::Color color)
: sp::ParticleEmitter(parent, 64, sp::ParticleEmitter::Origin::Local)
{
    setPosition(position);
    auto_destroy = true;
    
    addEffector<sp::ParticleEmitter::ColorEffector>(sp::Color(1, 1, 1, 1), sp::Color(1, 1, 1, 0));
    addEffector<sp::ParticleEmitter::SizeEffector>(5.0f, 10.0f);
    addEffector<sp::ParticleEmitter::VelocityScaleEffector>(10.0f, 1.0f);
    addEffector<sp::ParticleEmitter::ConstantAcceleration>(sp::Vector3f(0.0f, -30.0f, 0.0f));

    for(int n=0; n<64; n++)
    {
        sp::Vector2f velocity = sp::Vector2f(start_velocity) + sp::Vector2f(sp::random(1, 10), 0).rotate(sp::random(0, 360));
        sp::ParticleEmitter::Parameters parameters;
        parameters.velocity.x = velocity.x;
        parameters.velocity.y = velocity.y;

        parameters.lifetime = sp::Tween<double>::easeInCubic(sp::random(0.0, 1.0), 0.0, 1.0, 0.3, 3.0);
        emit(parameters);
    }
    
    sp::P<LevelScene>(getScene())->doCameraShake();
 }
