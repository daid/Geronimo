#ifndef LEVEL_SELECT_H
#define LEVEL_SELECT_H

#include <sp2/engine.h>
#include <sp2/scene/scene.h>
#include <sp2/graphics/gui/widget/widget.h>


class LevelNode;
class LevelSelect : public sp::Scene
{
public:
    LevelSelect();
    
    virtual void onFixedUpdate() override;
    virtual void onEnable() override;
    virtual void onDisable() override;

    void updateTrophys();
private:
    void startRandomReplay();
    bool startReplay(sp::string level_name, sp::string type);

    static constexpr int auto_replay_delay = 30 * sp::Engine::fixed_update_frequency;

    sp::P<sp::gui::Widget> gui;
    sp::P<sp::Camera> camera;
    sp::P<LevelNode> selection;
    sp::PList<LevelNode> all_levels;
    int auto_replay_countdown;
};

#endif//LEVEL_SELECT_H
