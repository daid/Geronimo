#include "levelScene.h"
#include "main.h"
#include "lineNodeBuilder.h"
#include "spaceship.h"
#include "door.h"
#include "laser.h"
#include "physicsObject.h"
#include "trigger.h"

#include <json11/json11.hpp>
#include <sp2/random.h>
#include <sp2/tween.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/gui/loader.h>
#include <sp2/io/cameraCapture.h>


LevelInfo level_info;

LevelScene::LevelScene()
: sp::Scene("LEVEL")
{
    gui = sp::gui::Loader::load("gui/hud.gui", "HUD");
    gui->hide();
    disable();
}

void LevelScene::loadLevel(sp::string name)
{
    {
        for(auto obj : getRoot()->getChildren())
            delete obj;
        level_name = name;
        level_info.gravity = sp::Vector2d(0, -0.15);
        level_info.fuel_ticks_used = 0;
        level_info.time_ticks = 0;
        level_already_finished = false;
        end_level_countdown = 60;
    
        level_info.fuel_trophy = 6000;
        level_info.time_trophy = 10 * 60 * 60;
        target_areas.clear();
        
        gui->show();
        gui->getWidgetWithID("BIG_ASS_TROPHY")->hide();
        
        FILE* f = fopen((name + ".trophy").c_str(), "rb");
        if (f)
        {
            fread(&level_info.fuel_trophy, sizeof(level_info.fuel_trophy), 1, f);
            fread(&level_info.time_trophy, sizeof(level_info.time_trophy), 1, f);
            fclose(f);
        }
    }
    
    {
        LineNodeBuilder builder;
        builder.loadFrom("levels/" + name + ".json", 2.0);
        sp::P<sp::Node> node = new sp::Node(getRoot());
        builder.create(node, LineNodeBuilder::CollisionType::Chains);
        node->render_data.color = sp::Color(0.8, 1.0, 0.8);
    }
    {
        std::string err;
        json11::Json json = json11::Json::parse(sp::io::ResourceProvider::get("levels/" + name + ".json")->readAll(), err);
        
        float tw = json["tilewidth"].number_value();
        float th = json["tileheight"].number_value();
        float offset_x = json["width"].number_value() / 2.0;
        float offset_y = json["height"].number_value() / 2.0;
        for(const auto& layer : json["layers"].array_items())
        {
            for(const auto& object : layer["objects"].array_items())
            {
                float x = object["x"].number_value();
                float y = -object["y"].number_value();
                sp::Vector2d position(x / tw - offset_x, y / th + offset_y);
                
                if (object["type"] == "ICON")
                {
                    addIcon(position, object["name"].string_value());
                }
                else if (object["type"] == "START")
                {
                    int index = sp::stringutil::convert::toInt(object["name"].string_value());
                    sp::P<Spaceship> spaceship = new Spaceship(getRoot());
                    spaceship->setPosition(position);
                    spaceship->setControls(&controls[index]);
                    spaceship->icon = addIcon(position, "gamepad" + sp::string(index + 1));
                    players.add(spaceship);
                }
                else if (object["type"] == "TARGET")
                {
                    float w = object["width"].number_value();
                    float h = -object["height"].number_value();
                    
                    target_areas.emplace_back(position.x, position.y + h / th, w / tw, -h / th);
                }
                else if (object["type"] == "OBJECT")
                {
                    sp::P<PhysicsObject> node = new PhysicsObject(getRoot(), object["name"].string_value());
                    node->setPosition(position);
                    
                    for(const auto& prop : object["properties"].array_items())
                    {
                        if (prop["name"] == "GOAL" && prop["value"] == "TARGET")
                            target_objects.add(node);
                        else
                            LOG(Warning, "Unknown object property:", prop["name"].string_value(), prop["value"].string_value());
                    }
                }
                else if (object["type"] == "DOOR")
                {
                    sp::P<Door> door = new Door(getRoot(), object["name"].string_value());
                    LineNodeBuilder builder;
                    builder.addLoop(json, object, 2.0);
                    builder.create(door, LineNodeBuilder::CollisionType::Chains);
                    //node->render_data.color = sp::Color(0.8, 1.0, 0.8);

                    for(const auto& prop : object["properties"].array_items())
                    {
                        if (prop["name"] == "offset")
                            door->opened_position = sp::stringutil::convert::toVector2d(prop["value"].string_value());
                        else
                            LOG(Warning, "Unknown object property:", prop["name"].string_value(), prop["value"].string_value());
                    }
                }
                else if (object["type"] == "LASER")
                {
                    sp::P<Laser> laser = new Laser(getRoot(), object["name"].string_value());
                    laser->setPosition(position);
                    for(const auto& prop : object["properties"].array_items())
                    {
                        if (prop["name"] == "angle")
                            laser->setAngle(sp::stringutil::convert::toFloat(prop["value"].string_value()));
                        else
                            LOG(Warning, "Unknown object property:", prop["name"].string_value(), prop["value"].string_value());
                    }
                }
                else if (object["type"] == "TRIGGER")
                {
                    float w = object["width"].number_value();
                    float h = -object["height"].number_value();
                    
                    sp::string target = object["name"].string_value();
                    sp::string source = "";
                    if (target.find(":") > -1)
                    {
                        source = target.substr(0, target.find(":"));
                        target = target.substr(target.find(":") + 1);
                    }
                    new Trigger(getRoot(), sp::Rect2d(position.x, position.y + h / th, w / tw, -h / th), source, target);
                }
                else if (object["type"] != "")
                {
                    LOG(Warning, "Unknown object type:", object["type"].string_value());
                }
            }
        }
        
        camera = new sp::Camera(getRoot());
        camera->setOrtographic(60);
        setDefaultCamera(camera);
        
        camera_view_range = sp::Vector2d(std::max(0.0f, offset_x - 60), std::max(0.0f, offset_y - 60));
    }
}

sp::P<sp::Node> LevelScene::addIcon(sp::Vector2d position, sp::string name)
{
    static std::shared_ptr<sp::MeshData> quad;
    if (!quad)
        quad = sp::MeshData::createQuad(sp::Vector2f(8, 8));

    sp::P<sp::Node> node = new sp::Node(getRoot());

    node->render_data.type = sp::RenderData::Type::Additive;
    node->render_data.shader = sp::Shader::get("internal:basic.shader");
    node->render_data.mesh = quad;
    node->render_data.texture = sp::texture_manager.get("gui/icons/" + name + ".png");
    node->render_data.color.a = 0.8;
    node->setPosition(position);
    return node;
}

void LevelScene::onFixedUpdate()
{
    level_info.time_ticks += 1;

    sp::Vector2d view_position;
    bool alive = false;
    bool in_target = true;
    for(auto player : players)
    {
        sp::Vector2d position = player->getPosition2D();
        view_position += position;
        if (player->isAlive())
        {
            alive = true;
            if (target_objects.size() > 0 || !inTargetArea(position) || (player->getLinearVelocity2D() - level_info.gravity).length() > 0.05)
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
            if (!inTargetArea(position) || (target->getLinearVelocity2D() - level_info.gravity).length() > 0.5)
                in_target = false;
        }
    }
    
    view_position /= double(players.size());
    view_position.x = std::min(camera_view_range.x, view_position.x);
    view_position.x = std::max(-camera_view_range.x, view_position.x);
    view_position.y = std::min(camera_view_range.y, view_position.y);
    view_position.y = std::max(-camera_view_range.y, view_position.y);

    if (shake)
    {
        shake--;
        view_position.x += std::sin(float(shake) * 3) * 1.5;
        view_position.y += std::cos(float(shake) * 7) * 1.5;
    }
    camera->setPosition(view_position);

    if (level_already_finished)
    {
        if (end_level_countdown > 0)
        {
            end_level_countdown--;
        }
        else
        {
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
                    levelFailed();
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
}

void LevelScene::onUpdate(float delta)
{
    if (level_already_finished)
        return;

    float fuel = float(level_info.fuel_ticks_used) / float(level_info.fuel_trophy);
    float time = float(level_info.time_ticks) / float(level_info.time_trophy);
    gui->getWidgetWithID("FUEL")->getWidgetWithID("BAR")->setAttribute("value", sp::string(fuel / 2.0, 5));
    gui->getWidgetWithID("FUEL")->getWidgetWithID("TROPHY")->setVisible(fuel <= 1.0);
    gui->getWidgetWithID("FUEL")->getWidgetWithID("FAILED")->setVisible(fuel > 1.0);
    gui->getWidgetWithID("TIME")->getWidgetWithID("BAR")->setAttribute("value", sp::string(time / 2.0, 5));
    gui->getWidgetWithID("TIME")->getWidgetWithID("TROPHY")->setVisible(time <= 1.0);
    gui->getWidgetWithID("TIME")->getWidgetWithID("FAILED")->setVisible(time > 1.0);
}

bool LevelScene::inTargetArea(sp::Vector2d position)
{
    for(auto& target_area : target_areas)
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
    
    if (trophy)
    {
        end_level_countdown = 60 * 3;
        earnTrophy(trophy);
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
    
    if (activity)
        loadLevel(level_name);
    else
        exitLevel();
}

void LevelScene::earnTrophy(int type_bits)
{
    sp::io::CameraCapture camera_capture(0);
    if (camera_capture.isOpen())
    {
        sp::Image photo = camera_capture.getFrame();
        if (type_bits & 1)
        {
            photo.saveToFile("photos/" + level_name + ".trophy.fuel.png");
            sp::texture_manager.forceRefresh(level_name + ".trophy.fuel.png");
        }
        if (type_bits & 2)
        {
            photo.saveToFile("photos/" + level_name + ".trophy.time.png");
            sp::texture_manager.forceRefresh(level_name + ".trophy.time.png");
        }
    }

    FILE* f = fopen((level_name + ".trophy").c_str(), "wb");
    fwrite(&level_info.fuel_trophy, sizeof(level_info.fuel_trophy), 1, f);
    fwrite(&level_info.time_trophy, sizeof(level_info.time_trophy), 1, f);
    fclose(f);
    
    gui->getWidgetWithID("BIG_ASS_TROPHY")->show();
    
    for(int n=0; n<10; n++) //Create fireworks!
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

void LevelScene::exitLevel()
{
    disable();
    gui->hide();
    sp::Scene::get("LEVEL_SELECT")->enable();
}
