#include "controls.h"

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
