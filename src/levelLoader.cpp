#include "levelLoader.h"
#include "levelScene.h"
#include "main.h"
#include "lineNodeBuilder.h"
#include "spaceship.h"
#include "door.h"
#include "laser.h"
#include "physicsObject.h"
#include "trigger.h"
#include "levelLoader.h"

#include <json11/json11.hpp>
#include <sp2/random.h>
#include <sp2/tween.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/gui/loader.h>
#include <sp2/io/cameraCapture.h>

static sp::P<sp::Node> addIcon(sp::P<sp::Node> root, sp::Vector2d position, sp::string name);

void loadLevel(sp::P<sp::Node> root, sp::string name)
{
    {
        for(auto obj : root->getChildren())
            delete obj;
        level_info.fuel_ticks_used = 0;
        level_info.time_ticks = 0;
        level_info.center_point_gravity = false;

        level_info.fuel_trophy = 6000;
        level_info.time_trophy = 10 * 60 * 60;

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
        sp::P<sp::Node> node = new sp::Node(root);
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
                    addIcon(root, position, object["name"].string_value());
                }
                else if (object["type"] == "START")
                {
                    int index = sp::stringutil::convert::toInt(object["name"].string_value());
                    sp::P<Spaceship> spaceship = new Spaceship(root);
                    spaceship->setPosition(position);
                    spaceship->setControls(&controls[index]);
                    spaceship->icon = addIcon(root, position, "gamepad" + sp::string(index + 1));
                }
                else if (object["type"] == "TARGET")
                {
                    float w = object["width"].number_value();
                    float h = -object["height"].number_value();

                    level_info.target_areas.emplace_back(position.x, position.y + h / th, w / tw, -h / th);
                }
                else if (object["type"] == "OBJECT")
                {
                    sp::P<PhysicsObject> node = new PhysicsObject(root, object["name"].string_value());
                    node->setPosition(position);

                    for(const auto& prop : object["properties"].array_items())
                    {
                        if (prop["name"] == "GOAL" && prop["value"] == "TARGET")
                            node->setAsGoalObject();
                        else if (prop["name"] == "initial_velocity") {
                            const std::string velocity_str = prop["value"].string_value();
                            const size_t comma_pos = velocity_str.find(",");
                            if (comma_pos == std::string::npos)
                                LOG(Warning, "Invalid initial velocity (needs comma delimiter):", velocity_str);
                            const std::string velocity_x = velocity_str.substr(0, comma_pos);
                            const std::string velocity_y = velocity_str.substr(comma_pos + 1);
                            sp::Vector2d velocity(sp::stringutil::convert::toFloat(velocity_x),
                                                  sp::stringutil::convert::toFloat(velocity_y));
                            node->setLinearVelocity(velocity);
                        }
                        else
                            LOG(Warning, "Unknown object property:", prop["name"].string_value(), prop["value"].string_value());
                    }
                }
                else if (object["type"] == "DOOR")
                {
                    sp::P<Door> door = new Door(root, object["name"].string_value());
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
                    sp::P<Laser> laser = new Laser(root, object["name"].string_value());
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
                    new Trigger(root, sp::Rect2d(position.x, position.y + h / th, w / tw, -h / th), source, target);
                }
                else if (object["type"] == "GRAVITY")
                {
                    level_info.center_point_gravity = true;
                    level_info.gravity_center = position;
                }
                else if (object["type"] != "")
                {
                    LOG(Warning, "Unknown object type:", object["type"].string_value());
                }
            }
        }

        level_info.camera_view_range = sp::Vector2d(std::max(0.0f, offset_x - 60), std::max(0.0f, offset_y - 60));
    }
}

sp::P<sp::Node> addIcon(sp::P<sp::Node> root, sp::Vector2d position, sp::string name)
{
    static std::shared_ptr<sp::MeshData> quad;
    if (!quad)
        quad = sp::MeshData::createQuad(sp::Vector2f(8, 8));

    sp::P<sp::Node> node = new sp::Node(root);

    node->render_data.type = sp::RenderData::Type::Additive;
    node->render_data.shader = sp::Shader::get("internal:basic.shader");
    node->render_data.mesh = quad;
    node->render_data.texture = sp::texture_manager.get("gui/icons/" + name + ".png");
    node->render_data.color.a = 0.8;
    node->setPosition(position);
    return node;
}
