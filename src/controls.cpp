#include "controls.h"
#include <cstdint>

Controls::Controls(int index)
: index(index)
, up("up" + sp::string(index))
, down("down" + sp::string(index))
, left("left" + sp::string(index))
, right("right" + sp::string(index))
, primary_action("primary_action" + sp::string(index))
, secondary_action("secondary_action" + sp::string(index))
, unknown2("unknown2_" + sp::string(index))
, self_destruct("self_destruct_" + sp::string(index))
, replay_time("replay_time_" + sp::string(index))
, replay_fuel("replay_fuel_" + sp::string(index))
, start("start" + sp::string(index))
{
    if (index == 0)
    {
        up.setKeys("up", "keypad 8");
        down.setKeys("down", "keypad 2");
        left.setKeys("left", "keypad 4");
        right.setKeys("right", "keypad 6");
        
        primary_action.setKey("space");
        secondary_action.setKey("z");
        
        unknown2.setKey("x");
        self_destruct.setKey("c");
        replay_time.setKey("v");
        replay_fuel.setKey("b");
        
        start.setKey("1");
    }

    if (index == 1)
    {
        up.setKey("w");
        down.setKey("s");
        left.setKey("a");
        right.setKey("d");
        
        primary_action.setKey("q");
        secondary_action.setKey("e");
        
        unknown2.setKey("r");
        self_destruct.setKey("f");
        replay_time.setKey("t");
        replay_fuel.setKey("g");
        
        start.setKey("2");
    }
}

KeyState KeyState::fromIO(const sp::io::Keybinding& key)
{
    KeyState result = KeyState(key.get(), key.getDown(), key.getUp(), key.getValue());
    return result;
}

void KeyState::writeToFile(FILE *f)
{
    int flags = (pressed ? 1 : 0) | (down ? 2 : 0) | (up ? 4 : 0);
    fwrite(&flags, 1, 1, f);
    fwrite(&value, sizeof(value), 1, f);
}

bool KeyState::readFromFile(FILE *f)
{
    int flags;
    size_t read_n = fread(&flags, 1, 1, f);
    pressed = (bool)(flags & 1);
    down = (bool)(flags & 2);
    up = (bool)(flags & 3);

    read_n += fread(&value, sizeof(value), 1, f);

    return read_n == 2;
}

PlayerControlsState Controls::playerControlStateFromIO()
{
    PlayerControlsState result = PlayerControlsState();

    result.up.fromIO(up);
    result.down.fromIO(down);
    result.left.fromIO(left);
    result.right.fromIO(right);
    result.primary_action.fromIO(primary_action);
    result.secondary_action.fromIO(secondary_action);
    result.unknown2.fromIO(unknown2);
    result.self_destruct.fromIO(self_destruct);
    result.replay_fuel.fromIO(replay_fuel);
    result.replay_time.fromIO(replay_time);
    result.start.fromIO(start);

    return result;
}

void PlayerControlsState::writeToFile(FILE *f)
{
    up.writeToFile(f);
    down.writeToFile(f);
    left.writeToFile(f);
    right.writeToFile(f);
    primary_action.writeToFile(f);
    secondary_action.writeToFile(f);
    unknown2.writeToFile(f);
    self_destruct.writeToFile(f);
    replay_fuel.writeToFile(f);
    replay_time.writeToFile(f);
    start.writeToFile(f);
}

bool PlayerControlsState::readFromFile(FILE *f)
{
    if(!up.readFromFile(f))
        return false;
    down.readFromFile(f);
    left.readFromFile(f);
    right.readFromFile(f);
    primary_action.readFromFile(f);
    secondary_action.readFromFile(f);
    unknown2.readFromFile(f);
    self_destruct.readFromFile(f);
    replay_fuel.readFromFile(f);
    replay_time.readFromFile(f);
    start.readFromFile(f);

    return true;
}

void ControlsState::writeToFile(FILE *f)
{
    players[0].writeToFile(f);
    players[1].writeToFile(f);
}

bool ControlsState::readFromFile(FILE *f)
{
    if(!players[0].readFromFile(f))
        return false;
    players[1].readFromFile(f);

    return true;
}
