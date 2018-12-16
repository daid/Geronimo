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
, unknown4("unknown4_" + sp::string(index))
, unknown5("unknown5_" + sp::string(index))
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
        unknown4.setKey("v");
        unknown5.setKey("b");
        
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
        unknown4.setKey("t");
        unknown5.setKey("g");
        
        start.setKey("2");
    }
}

KeyState KeyState::fromIO(const sp::io::Keybinding& key)
{
    KeyState result = KeyState(key.get(), key.getDown(), key.getUp(), key.getValue());
    return result;
}

void KeyState::writeToFile(FILE *f) {
    int flags = (pressed ? 1 : 0) | (down ? 2 : 0) | (up ? 4 : 0);
    fwrite(&flags, 1, 1, f);
    fwrite(&value, sizeof(value), 1, f);
}

bool KeyState::readFromFile(FILE *f, KeyState& result) {
    int flags;
    size_t read_n = fread(&flags, 1, 1, f);
    result.pressed = (bool)(flags & 1);
    result.down = (bool)(flags & 2);
    result.up = (bool)(flags & 3);

    read_n += fread(&result.value, sizeof(result.value), 1, f);

    return read_n == 2;
}

PlayerControlsState Controls::playerControlStateFromIO()
{
    PlayerControlsState result = PlayerControlsState();

    result.up = KeyState::fromIO(up);
    result.down = KeyState::fromIO(down);
    result.left = KeyState::fromIO(left);
    result.right= KeyState::fromIO(right);

    result.primary_action = KeyState::fromIO(primary_action);
    result.secondary_action = KeyState::fromIO(secondary_action);

    result.unknown2 = KeyState::fromIO(unknown2);
    result.self_destruct= KeyState::fromIO(self_destruct);
    result.unknown5 = KeyState::fromIO(unknown4);
    result.unknown4 = KeyState::fromIO(unknown5);

    result.start = KeyState::fromIO(start);

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
    unknown5.writeToFile(f);
    unknown4.writeToFile(f);
    start.writeToFile(f);
}

bool PlayerControlsState::readFromFile(FILE *f, PlayerControlsState& result)
{
    if(!KeyState::readFromFile(f, result.up))
        return false;
    KeyState::readFromFile(f, result.down);
    KeyState::readFromFile(f, result.left);
    KeyState::readFromFile(f, result.right);
    KeyState::readFromFile(f, result.primary_action);
    KeyState::readFromFile(f, result.secondary_action);
    KeyState::readFromFile(f, result.unknown2);
    KeyState::readFromFile(f, result.self_destruct);
    KeyState::readFromFile(f, result.unknown5);
    KeyState::readFromFile(f, result.unknown4);
    KeyState::readFromFile(f, result.start);

    return true;
}

void ControlsState::writeToFile(FILE *f)
{
    players[0].writeToFile(f);
    players[1].writeToFile(f);
}

bool ControlsState::readFromFile(FILE *f, ControlsState& result)
{
    if(!PlayerControlsState::readFromFile(f, result.players[0]))
        return false;
    PlayerControlsState::readFromFile(f, result.players[1]);

    return true;
}