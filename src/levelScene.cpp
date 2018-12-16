#include "levelScene.h"
#include "main.h"
#include "lineNodeBuilder.h"
#include "spaceship.h"
#include "door.h"
#include "laser.h"
#include "physicsObject.h"
#include "trigger.h"
#include "levelLoader.h"
#include "levelGenerator.h"

#include <json11/json11.hpp>
#include <sp2/random.h>
#include <sp2/tween.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/gui/loader.h>
#include <sp2/io/cameraCapture.h>
#include <sp2/engine.h>


LevelInfo level_info;

LevelScene::LevelScene()
: sp::Scene("LEVEL")
{
    gui = sp::gui::Loader::load("gui/hud.gui", "HUD");
    gui->hide();
    gui->getWidgetWithID("CAMERA_PREVIEW")->setRotation(-90);
    disable();
    
    camera_capture_texture = new CameraCaptureTexture();

    replay_controls_buffer = std::vector<ControlsState>();
}

void LevelScene::loadLevel(sp::string name, bool replay, std::string replay_file)
{
    level_name = name;
    level_already_finished = false;
    end_level_countdown = 60;
    level_info.target_areas.clear();

    level_info.fuel_ticks_used = 0;
    level_info.time_ticks = 0;
    level_info.center_point_gravity = false;

    level_info.fuel_trophy = 6000;
    level_info.time_trophy = 10 * 60 * 60;
    level_info.depth_trophy = 10.0;
    level_info.trophy_mode = LevelInfo::TrophyMode::Normal;

    for(auto obj : getRoot()->getChildren())
        delete obj;

    gui->show();
    gui->getWidgetWithID("BIG_ASS_TROPHY")->hide();
    gui->getWidgetWithID("CAMERA_PREVIEW")->hide();

    if (name == "X")
    {
        new LevelGenerator(getRoot());
        level_info.trophy_mode = LevelInfo::TrophyMode::Depth;
    }
    else
    {
        ::loadLevel(getRoot(), name);
    }

    for(auto obj : getRoot()->getChildren())
    {
        sp::P<sp::Node> n = sp::P<sp::Node>(obj);
        players.add(n);
        sp::P<PhysicsObject> po = n;
        if (po && po->isGoal())
            target_objects.add(po);
    }

    this->replay = replay;
    if(replay)
    {
        loadReplay(replay_file);
    }
    else
    {
        replay_controls_buffer.clear();
    }

    camera = new sp::Camera(getRoot());
    camera->setOrtographic(60);
    setDefaultCamera(camera);
}

void LevelScene::onFixedUpdate()
{
    ControlsState controlsState;

    if(!replay)
    {
        if (level_info.time_ticks > 0)  // Work around the fact that the first frame all buttons seem pressed
        {
            for(auto player : players) {
                controlsState.players[player->index] = controls[player->index].playerControlStateFromIO();
            }
        }

        // Make sure we don't record forever
        if (replay_controls_buffer.size() < sp::Engine::fixed_update_frequency*max_replay_time_sec)
        {
            replay_controls_buffer.push_back(controlsState);
        }
    } else {

        if(level_info.time_ticks < replay_controls_buffer.size()) {
            controlsState = replay_controls_buffer[level_info.time_ticks];
        }

        //Abort playback
        for(auto player : players)
        {
            if(controls[player->index].primary_action.getDown() || controls[player->index].secondary_action.getDown())
            {
                for(auto p : players)
                {
                    p->explode();
                }
            }
        }

    }

    sp::Vector2d view_position;
    bool alive = false;
    bool in_target = true;
    for(auto player : players)
    {
        player->setControlState(controlsState.players[player->index]);

        sp::Vector2d position = player->getPosition2D();
        view_position += position;
        if (player->isAlive())
        {
            alive = true;
            if (target_objects.size() > 0 || !inTargetArea(position) || (player->getLinearVelocity2D() - level_info.getGravityAt(position)).length() > 0.07)
                in_target = false;
            else
                player->setIcon("checkmark");
        }
        else
        {
            in_target = false;
        }
    }
    
    if (target_objects.size())
    {
        in_target = true;
        for(auto target : target_objects)
        {
            sp::Vector2d position = target->getPosition2D();
            if (!inTargetArea(position) || (target->getLinearVelocity2D() - level_info.getGravityAt(position)).length() > 0.7)
                in_target = false;
        }
    }
    
    view_position /= double(players.size());
    view_position.x = std::min(level_info.camera_view_range.x, view_position.x);
    view_position.x = std::max(-level_info.camera_view_range.x, view_position.x);
    view_position.y = std::min(level_info.camera_view_range.y, view_position.y);
    view_position.y = std::max(-level_info.camera_view_range.y, view_position.y);

    if (shake)
    {
        shake--;
        view_position.x += std::sin(float(shake) * 3) * 1.5;
        view_position.y += std::cos(float(shake) * 7) * 1.5;
    }
    camera->setPosition(view_position);

    if (level_already_finished)
    {
        gui->getWidgetWithID("CAMERA_PREVIEW")->render_data.texture = camera_capture_texture;

        if (end_level_countdown > 0)
        {
            end_level_countdown--;
        }
        else
        {
            if (trophy_earned_flags)
                earnTrophyB(trophy_earned_flags);
            exitLevel();
        }
    }
    else
    {
        if (!alive || in_target)
        {
            if (end_level_countdown > 0)
            {
                end_level_countdown--;
            }
            else
            {
                if (alive)
                {
                    level_already_finished = true;
                    levelFinished();
                }
                else
                {
                    if (level_info.trophy_mode == LevelInfo::TrophyMode::Depth)
                    {
                        float depth = getCamera()->getPosition2D().length();
                        if (depth > level_info.depth_trophy)
                        {
                            level_info.depth_trophy = depth;
                            level_already_finished = true;
                            earnTrophyA(4);
                        }
                        else
                        {
                            levelFailed();
                        }
                    }
                    else
                    {
                        levelFailed();
                    }
                }
            }
        }
        else
        {
            end_level_countdown = 60 * 2;
        }
    }

    if (controls[0].start.getDown() || escape_key.getDown())
    {
        exitLevel();
    }

    level_info.time_ticks += 1;
}

void LevelScene::onUpdate(float delta)
{
    if (level_already_finished)
        return;

    switch (level_info.trophy_mode)
    {
    case LevelInfo::TrophyMode::Normal: {
        float fuel = float(level_info.fuel_ticks_used) / float(level_info.fuel_trophy);
        float time = float(level_info.time_ticks) / float(level_info.time_trophy);
        gui->getWidgetWithID("FUEL")->show();
        gui->getWidgetWithID("FUEL")->getWidgetWithID("BAR")->setAttribute("value", sp::string(fuel / 2.0, 5));
        gui->getWidgetWithID("FUEL")->getWidgetWithID("TROPHY")->setVisible(fuel <= 1.0);
        gui->getWidgetWithID("FUEL")->getWidgetWithID("FAILED")->setVisible(fuel > 1.0);
        gui->getWidgetWithID("TIME")->show();
        gui->getWidgetWithID("TIME")->getWidgetWithID("BAR")->setAttribute("value", sp::string(time / 2.0, 5));
        gui->getWidgetWithID("TIME")->getWidgetWithID("TROPHY")->setVisible(time <= 1.0);
        gui->getWidgetWithID("TIME")->getWidgetWithID("FAILED")->setVisible(time > 1.0);
        gui->getWidgetWithID("DEPTH")->hide();
        }break;
    case LevelInfo::TrophyMode::Depth: {
        float depth = getCamera()->getPosition2D().length();
        gui->getWidgetWithID("FUEL")->hide();
        gui->getWidgetWithID("TIME")->hide();
        gui->getWidgetWithID("DEPTH")->show();
        gui->getWidgetWithID("DEPTH")->getWidgetWithID("BAR")->setAttribute("value", sp::string(depth / (level_info.depth_trophy * 2), 5));
        gui->getWidgetWithID("DEPTH")->getWidgetWithID("TROPHY")->setVisible(depth >= level_info.depth_trophy);
        gui->getWidgetWithID("DEPTH")->getWidgetWithID("FAILED")->setVisible(depth < level_info.depth_trophy);
        }break;
    }
}

bool LevelScene::inTargetArea(sp::Vector2d position)
{
    for(auto& target_area : level_info.target_areas)
        if (target_area.contains(position))
            return true;
    return false;
}

void LevelScene::levelFinished()
{
    int trophy = 0;
    if (level_info.fuel_ticks_used < level_info.fuel_trophy)
    {
        level_info.fuel_trophy = level_info.fuel_ticks_used;
        trophy |= 1;
    }
    if (level_info.time_ticks < level_info.time_trophy)
    {
        level_info.time_trophy = level_info.time_ticks;
        trophy |= 2;
    }
    
    if (trophy && !replay)
    {
        earnTrophyA(trophy);

        if(trophy & 1)
        {
            saveReplay(level_name + "-fuel.replay", replay_controls_buffer);
        }

        if(trophy & 2)
        {
            saveReplay(level_name + "-time.replay", replay_controls_buffer);
        }
    }
    else
    {
        exitLevel();
    }
}

void LevelScene::levelFailed()
{
    bool activity = false;
    for(auto player : players)
        activity = activity || player->hadActivity();
    
    if (activity && !replay)
        loadLevel(level_name);
    else
        exitLevel();
}


void LevelScene::loadReplay(std::string filepath)
{
    replay_controls_buffer.clear();

    FILE* replay_fh = fopen(filepath.c_str(), "r");
    ControlsState frame;
    while(ControlsState::readFromFile(replay_fh, frame))
    {
        replay_controls_buffer.push_back(frame);
    }
}


void LevelScene::saveReplay(std::string filepath, std::vector<ControlsState> replay_buffer)
{
    FILE* f = fopen(filepath.c_str(), "w");
    for(auto frame : replay_buffer)
    {
        frame.writeToFile(f);
    }
    fclose(f);
}

void LevelScene::earnTrophyA(int flags)
{
    trophy_earned_flags = flags;
    end_level_countdown = 60 * 3;
    camera_capture_texture->open(0);

    FILE* f = fopen((level_name + ".trophy").c_str(), "wb");
    switch (level_info.trophy_mode)
    {
    case LevelInfo::TrophyMode::Normal:
        fwrite(&level_info.fuel_trophy, sizeof(level_info.fuel_trophy), 1, f);
        fwrite(&level_info.time_trophy, sizeof(level_info.time_trophy), 1, f);
        break;
    case LevelInfo::TrophyMode::Depth:
        fwrite(&level_info.depth_trophy, sizeof(level_info.depth_trophy), 1, f);
        break;
    }
    fclose(f);
    
    gui->getWidgetWithID("BIG_ASS_TROPHY")->show();
    gui->getWidgetWithID("CAMERA_PREVIEW")->show();
    
    for(int n=0; n<15; n++) //Create fireworks!
    {
        sp::P<sp::ParticleEmitter> pe = new sp::ParticleEmitter(camera, 64, sp::ParticleEmitter::Origin::Local);
        pe->setPosition(sp::Vector2d(sp::random(-60, 60), sp::random(-60, 60)));
        pe->auto_destroy = true;
        for(int n=0; n<64; n++)
        {
            sp::Vector2f velocity = sp::Vector2f(sp::random(1, 40), 0).rotate(sp::random(0, 360));
            sp::ParticleEmitter::Parameters parameters;
            parameters.velocity.x = velocity.x;
            parameters.velocity.y = velocity.y;
            parameters.acceleration = parameters.velocity;
            parameters.acceleration.y += -100.0;
            parameters.start_color = sp::HsvColor(sp::random(0, 360), 20, 100);
            parameters.end_color = parameters.start_color;
            parameters.end_color.a = 0;

            parameters.start_size = 5.0;
            parameters.end_size = 10.0;
            parameters.lifetime = sp::Tween<double>::easeInCubic(sp::random(0.0, 1.0), 0.0, 1.0, 0.3, 3.0);
            pe->emit(parameters);
        }
    }
}

void LevelScene::earnTrophyB(int flags)
{
    sp::Image photo = camera_capture_texture->getFrame();
    if (photo.getSize().x < 10)
        return;

    if (flags & 1)
    {
        photo.saveToFile("photos/" + level_name + ".trophy.fuel.png");
        sp::texture_manager.forceRefresh(level_name + ".trophy.fuel.png");
    }
    if (flags & 2)
    {
        photo.saveToFile("photos/" + level_name + ".trophy.time.png");
        sp::texture_manager.forceRefresh(level_name + ".trophy.time.png");
    }
    if (flags & 4)
    {
        photo.saveToFile("photos/" + level_name + ".trophy.depth.png");
        sp::texture_manager.forceRefresh(level_name + ".trophy.depth.png");
    }
}

void LevelScene::exitLevel()
{
    disable();
    gui->hide();
    camera_capture_texture->close();
    sp::Scene::get("LEVEL_SELECT")->enable();
}

sp::Vector2d LevelInfo::getGravityAt(sp::Vector2d position)
{
    if (center_point_gravity)
        return (position - gravity_center).normalized() * -0.15;
    return sp::Vector2d(0, -0.15);
}
