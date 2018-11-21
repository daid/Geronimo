#ifndef CONTROLS_H
#define CONTROLS_H

#include <sp2/io/keybinding.h>

class Controls
{
public:
    Controls(int index);

    int index;
    
    sp::io::Keybinding up;
    sp::io::Keybinding down;
    sp::io::Keybinding left;
    sp::io::Keybinding right;

    sp::io::Keybinding primary_action;
    sp::io::Keybinding secondary_action;

    sp::io::Keybinding unknown2;
    sp::io::Keybinding restart;
    sp::io::Keybinding unknown4;
    sp::io::Keybinding unknown5;
    
    sp::io::Keybinding start;
};

#endif//CONTROLS_H
