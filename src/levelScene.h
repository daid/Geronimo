#ifndef LEVEL_SCENE_H
#define LEVEL_SCENE_H

#include <sp2/scene/scene.h>
#include <sp2/scene/camera.h>
#include <sp2/math/rect.h>
#include <sp2/graphics/gui/widget/widget.h>


class Spaceship;
class LevelScene : public sp::Scene
{
public:
    LevelScene();
    
    void loadLevel(sp::string name);
    
    sp::P<sp::Node> addIcon(sp::Vector2d position, sp::string name);
    
    virtual void onFixedUpdate() override;
    virtual void onUpdate(float delta) override;
    void doCameraShake() { shake = 15; }
    
    bool inTargetArea(sp::Vector2d position);
private:
    void levelFinished();
    void levelFailed();
    void earnTrophy(int type_bits);
    void exitLevel();

    sp::PList<Spaceship> players;
    sp::PList<sp::Node> target_objects;
    sp::P<sp::Camera> camera;
    sp::Vector2d camera_view_range;
    int end_level_countdown;
    int shake = 0;
    bool level_already_finished;
    sp::string level_name;
    std::vector<sp::Rect2d> target_areas;
    
    sp::P<sp::gui::Widget> gui;
};

class LevelInfo
{
public:
    sp::Vector2d getGravityAt(sp::Vector2d position);
    int fuel_ticks_used;
    int time_ticks;
    
    int fuel_trophy;
    int time_trophy;
    
    bool center_point_gravity;
    sp::Vector2d gravity_center;
};

extern LevelInfo level_info;

#endif//LEVEL_SCENE_H
