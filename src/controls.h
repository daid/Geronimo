#ifndef CONTROLS_H
#define CONTROLS_H

#include <sp2/io/keybinding.h>
#include <cstdio>

class KeyState
{
public:
    KeyState(bool pressed, bool down, bool up, float value) : pressed(pressed), down(down), up(up), value(value) {}
    KeyState() : pressed(false), down(false), up(false), value(0) {}

    bool pressed;       //True when this key is currently being pressed.
    bool down;          //True for 1 update cycle when the key is pressed.
    bool up;            //True for 1 update cycle when the key is released.
    float value;        //Returns a value in the range -1 to 1 for this keybinding. On keyboard keys this is always 0 or 1, but for joysticks this can be anywhere in the range -1.0 to 1.0

    void writeToFile(FILE* f);

    static bool readFromFile(FILE* f, KeyState& result);
    static KeyState fromIO(const sp::io::Keybinding& key);
};

class PlayerControlsState
{
public:
    KeyState up;
    KeyState down;
    KeyState left;
    KeyState right;
    KeyState primary_action;
    KeyState secondary_action;
    KeyState unknown2;
    KeyState self_destruct;
    KeyState replay_time;
    KeyState replay_fuel;
    KeyState start;

    void writeToFile(FILE* f);

    static bool readFromFile(FILE* f, PlayerControlsState& result);
};

class ControlsState
{
public:
    PlayerControlsState players[2];
    void writeToFile(FILE* f);

    static bool readFromFile(FILE* f, ControlsState& result);
};

class Controls
{
public:
    Controls(int index);
    PlayerControlsState playerControlStateFromIO();

    int index;
    
    sp::io::Keybinding up;
    sp::io::Keybinding down;
    sp::io::Keybinding left;
    sp::io::Keybinding right;

    sp::io::Keybinding primary_action;
    sp::io::Keybinding secondary_action;

    sp::io::Keybinding unknown2;
    sp::io::Keybinding self_destruct;
    sp::io::Keybinding replay_time;
    sp::io::Keybinding replay_fuel;
    
    sp::io::Keybinding start;
};

#endif//CONTROLS_H
