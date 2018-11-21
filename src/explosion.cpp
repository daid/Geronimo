#include "explosion.h"
#include "levelScene.h"

#include <sp2/random.h>
#include <sp2/tween.h>


Explosion::Explosion(sp::P<sp::Node> parent, sp::Vector2d position, sp::Vector2d start_velocity, sp::Color color)
: sp::ParticleEmitter(parent, 64, sp::ParticleEmitter::Origin::Local)
{
    setPosition(position);
    auto_destroy = true;

    for(int n=0; n<64; n++)
    {
        sp::Vector2f velocity = sp::Vector2f(start_velocity) + sp::Vector2f(sp::random(1, 10), 0).rotate(sp::random(0, 360));
        sp::ParticleEmitter::Parameters parameters;
        parameters.velocity.x = velocity.x;
        parameters.velocity.y = velocity.y;
        parameters.acceleration = parameters.velocity;
        parameters.acceleration.y += -30.0;
        parameters.start_color = color;
        parameters.end_color = color;
        parameters.end_color.a = 0;

        parameters.start_size = 5.0;
        parameters.end_size = 10.0;
        parameters.lifetime = sp::Tween<double>::easeInCubic(sp::random(0.0, 1.0), 0.0, 1.0, 0.3, 3.0);
        emit(parameters);
    }
    
    sp::P<LevelScene>(getScene())->doCameraShake();
 }
