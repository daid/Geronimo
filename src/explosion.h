#ifndef EXPLOSION_H
#define EXPLOSION_H

#include <sp2/scene/particleEmitter.h>


class Explosion : public sp::ParticleEmitter
{
public:
    Explosion(sp::P<sp::Node> parent, sp::Vector2d position, sp::Vector2d velocity, sp::Color color=sp::Color(1,1,1));
};

#endif//EXPLOSION_H
