#ifndef LEVEL_SCENE_H
#define LEVEL_SCENE_H

#include <sp2/scene/scene.h>
#include <sp2/scene/camera.h>
#include <sp2/math/rect.h>
#include <sp2/graphics/gui/widget/widget.h>
#include "cameraCaptureTexture.h"
#include "controls.h"
#include <vector>

class Spaceship;
class LevelScene : public sp::Scene
{
public:
    LevelScene();
    
    void loadLevel(sp::string name, bool replay=false, std::string replay_file="");
    
    sp::P<sp::Node> addIcon(sp::Vector2d position, sp::string name);
    
    virtual void onFixedUpdate() override;
    virtual void onUpdate(float delta) override;
    void doCameraShake() { shake = 15; }
    
    bool inTargetArea(sp::Vector2d position);
private:
    void levelFinished();
    void levelFailed();
    void earnTrophyA(int flags);
    void earnTrophyB(int flags);
    void exitLevel();

    sp::PList<Spaceship> players;
    sp::PList<sp::Node> target_objects;
    sp::P<sp::Camera> camera;
    int end_level_countdown;
    int shake = 0;
    bool level_already_finished;
    int trophy_earned_flags;
    sp::string level_name;
    
    sp::P<sp::gui::Widget> gui;
    
    CameraCaptureTexture* camera_capture_texture;

    const static int max_replay_time_sec = 10*60; //Max number of seconds of a replay file
    bool replay;
    std::vector<ControlsState> replay_controls_buffer;
    int fixed_frame_count;

    void loadReplay(std::string filepath);
    static void saveReplay(std::string filepath, std::vector<ControlsState> replay_buffer);
};

class LevelInfo
{
public:
    sp::Vector2d getGravityAt(sp::Vector2d position);
    int fuel_ticks_used;
    int time_ticks;
    
    int fuel_trophy;
    int time_trophy;
    float depth_trophy;

    enum class TrophyMode
    {
        Normal,
        Depth,
    } trophy_mode;

    sp::Vector2d camera_view_range;
    std::vector<sp::Rect2d> target_areas; //TODO: Replace with with a better system
    
    bool center_point_gravity;
    sp::Vector2d gravity_center;
};

extern LevelInfo level_info;

#endif//LEVEL_SCENE_H
