#include "bomb.h"
#include "explosion.h"


Bomb::Bomb(sp::P<sp::Node> parent, sp::string name)
: PhysicsObject(parent, name)
{
}

void Bomb::onFixedUpdate()
{
    switch(state)
    {
    case State::Inactive:
        {
            float velocity = getLinearVelocity2D().length();
            velocity_average = velocity_average * 0.9 + velocity * 0.1;
            if (velocity_average > 3.0)
            {
                state = State::Triggered;
                state_timer = 30;
            }
        }
        break;
    case State::Triggered:
        state_timer--;
        if (state_timer < 1)
        {
            state = State::Delay;
            render_data.color = sp::Color(1, 1, 1);
            state_timer = explosion_delay;
        }else{
            if (state_timer % 4 < 2)
                render_data.color = sp::Color(1, 0.2, 0.2);
            else
                render_data.color = sp::Color(1, 1, 1);
        }
        break;
    case State::Delay:
        state_timer--;
        if (state_timer < 1)
        {
            new Explosion(getParent(), getPosition2D(), getLinearVelocity2D(), sp::Color(1, 0.8, 0.8));
            for(sp::P<sp::Node> obj : getParent()->getChildren())
            {
                sp::Vector2d diff = obj->getPosition2D() - getPosition2D();
                double dist = diff.length();
                obj->setLinearVelocity(obj->getLinearVelocity2D() + (diff / dist) * (explosion_power / (dist * dist)));
            }
            delete this;
            return;
        }
        int blink_delay = 60;
        if (state_timer < 180)
            blink_delay = 20;
        if (state_timer < 60)
            blink_delay = 6;
        if (state_timer % blink_delay < 3)
            render_data.color = sp::Color(1, 0.2, 0.2);
        else
            render_data.color = sp::Color(1, 1, 1);
        break;
    }

    PhysicsObject::onFixedUpdate();
}

void Bomb::setProperty(sp::string name, sp::string value)
{
    if (name == "delay")
        explosion_delay = sp::stringutil::convert::toFloat(value) * 60.0f;
    else if (name == "power")   //Raw explosion power
        explosion_power = sp::stringutil::convert::toFloat(value);
    else if (name == "range")   //Explosion kill range
        explosion_power = std::pow(sp::stringutil::convert::toFloat(value), 2.0) * 10.0f;
    else
        PhysicsObject::setProperty(name, value);
}
