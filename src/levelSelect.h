#ifndef LEVEL_SELECT_H
#define LEVEL_SELECT_H

#include <sp2/scene/scene.h>
#include <sp2/graphics/gui/widget/widget.h>


class LevelNode;
class LevelSelect : public sp::Scene
{
public:
    LevelSelect();
    
    virtual void onFixedUpdate() override;

    void updateTrophys();
private:
    sp::P<sp::gui::Widget> gui;
    sp::P<sp::Camera> camera;
    sp::P<LevelNode> selection;
};

#endif//LEVEL_SELECT_H
