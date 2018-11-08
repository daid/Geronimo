#include "levelScene.h"
#include "main.h"
#include "lineNodeBuilder.h"
#include "spaceship.h"
#include "physicsObject.h"

#include <json11/json11.hpp>
#include <sp2/random.h>
#include <sp2/tween.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/gui/loader.h>


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
        builder.loadFrom(name + ".json", 2.0);
        sp::P<sp::Node> node = new sp::Node(getRoot());
        builder.create(node, LineNodeBuilder::CollisionType::Chains);
        node->render_data.color = sp::Color(0.8, 1.0, 0.8);
    }
    {
        std::string err;
        json11::Json json = json11::Json::parse(sp::io::ResourceProvider::get(name + ".json")->readAll(), err);
        
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
                    target_objects.add(node);
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
            if (!inTargetArea(position) || (target->getLinearVelocity2D() - level_info.gravity).length() > 0.05)
                in_target = false;
        }
    }
    
    view_position /= double(players.size());
    view_position.x = std::min(camera_view_range.x, view_position.x);
    view_position.x = std::max(-camera_view_range.x, view_position.x);
    view_position.y = std::min(camera_view_range.y, view_position.y);
    view_position.y = std::max(-camera_view_range.y, view_position.y);
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
                    bool store_result = false;
                    if (level_info.fuel_ticks_used < level_info.fuel_trophy)
                    {
                        level_info.fuel_trophy = level_info.fuel_ticks_used;
                        store_result = true;
                    }
                    if (level_info.time_ticks < level_info.time_trophy)
                    {
                        level_info.time_trophy = level_info.time_ticks;
                        store_result = true;
                    }
                    
                    if (store_result)
                    {
                        FILE* f = fopen((level_name + ".trophy").c_str(), "wb");
                        fwrite(&level_info.fuel_trophy, sizeof(level_info.fuel_trophy), 1, f);
                        fwrite(&level_info.time_trophy, sizeof(level_info.time_trophy), 1, f);
                        fclose(f);
                        end_level_countdown = 60 * 3;
                        
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
                    else
                    {
                        exitLevel();
                    }
                }
                else
                {
                    loadLevel(level_name);
                }
            }
        }
        else
        {
            end_level_countdown = 60 * 2;
        }
    }

    if (controls[0].start.getDown())
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

void LevelScene::exitLevel()
{
    disable();
    gui->hide();
    sp::Scene::get("LEVEL_SELECT")->enable();
}
