#ifndef LEVEL_SCENE_H
#define LEVEL_SCENE_H

#include <sp2/scene/scene.h>
#include <sp2/scene/camera.h>
#include <sp2/math/rect.h>


class Spaceship;
class LevelScene : public sp::Scene
{
public:
    LevelScene();
    
    void loadLevel(sp::string name);
    
    sp::P<sp::Node> addIcon(sp::Vector2d position, sp::string name);
    
    virtual void onFixedUpdate() override;
    
    bool inTargetArea(sp::Vector2d position);
private:
    sp::PList<Spaceship> players;
    sp::PList<sp::Node> target_objects;
    sp::P<sp::Camera> camera;
    sp::Vector2d camera_view_range;
    int end_level_countdown;
    sp::string level_name;
    std::vector<sp::Rect2d> target_areas;
};

#endif//LEVEL_SCENE_H
