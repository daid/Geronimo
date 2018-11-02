#include "controls.h"

Controls::Controls(int index)
: up("up" + sp::string(index))
, down("down" + sp::string(index))
, left("left" + sp::string(index))
, right("right" + sp::string(index))
, primary_action("primary_action" + sp::string(index))
, secondary_action("secondary_action" + sp::string(index))
, unknown2("unknown2_" + sp::string(index))
, unknown3("unknown3_" + sp::string(index))
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
        unknown3.setKey("c");
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
        unknown3.setKey("t");
        unknown4.setKey("y");
        unknown5.setKey("u");
        
        start.setKey("2");
    }
}
