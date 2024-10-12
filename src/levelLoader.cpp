#include "levelLoader.h"
#include "levelScene.h"
#include "main.h"
#include "lineNodeBuilder.h"
#include "spaceship.h"
#include "door.h"
#include "laser.h"
#include "pushLaser.h"
#include "physicsObject.h"
#include "bomb.h"
#include "trigger.h"
#include "timer.h"
#include "wheel.h"
#include "vehicle.h"
#include "remoteControl.h"

#include <nlohmann/json.hpp>
#include <sp2/random.h>
#include <sp2/tween.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/gui/loader.h>
#include <sp2/io/cameraCapture.h>


static sp::P<sp::Node> addIcon(sp::P<sp::Node> root, sp::Vector2d position, sp::string name);
static sp::P<sp::Node> addDecoration(sp::P<sp::Node> root, sp::Vector2d position, sp::Vector2f size, sp::string name);
static void constructSubParts(sp::P<sp::Node> parent, sp::string json_file);

void loadLevel(sp::P<sp::Node> root, sp::string name)
{
    {
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
        nlohmann::json json = nlohmann::json::parse(sp::io::ResourceProvider::get("levels/" + name + ".json")->readAll());

        float tw = json["tilewidth"];
        float th = json["tileheight"];
        float offset_x = float(json["width"]) / 2.0;
        float offset_y = float(json["height"]) / 2.0;
        for(const auto& layer : json["layers"])
        {
            for(const auto& object : layer["objects"])
            {
                float x = object["x"];
                float y = -float(object["y"]);
                sp::Vector2d position(x / tw - offset_x, y / th + offset_y);
                sp::P<LevelObject> obj;

                if (object["type"] == "ICON")
                {
                    addIcon(root, position, object["name"]);
                }
                else if (object["type"] == "DECORATION")
                {
                    float width = object["width"];
                    float height = object["height"];
                    addDecoration(root, position, sp::Vector2f(width, height), object["name"]);
                }
                else if (object["type"] == "START")
                {
                    int index = sp::stringutil::convert::toInt(object["name"]);
                    sp::P<Spaceship> spaceship = new Spaceship(root);
                    spaceship->setPosition(position);
                    spaceship->setIndex(index);
                    spaceship->icon = addIcon(root, position, "gamepad" + sp::string(index + 1));
                    obj = spaceship;
                }
                else if (object["type"] == "TARGET")
                {
                    float w = object["width"];
                    float h = -float(object["height"]);

                    level_info.target_areas.emplace_back(position.x, position.y + h / th, w / tw, -h / th);
                }
                else if (object["type"] == "OBJECT")
                {
                    obj = new PhysicsObject(root, object["name"]);
                    obj->setPosition(position);
                    constructSubParts(obj, std::string(object["name"]) + ".json");
                }
                else if (object["type"] == "VEHICLE")
                {
                    obj = new Vehicle(root, object["name"]);
                    obj->setPosition(position);
                    constructSubParts(obj, std::string(object["name"]) + ".json");
                }
                else if (object["type"] == "BOMB")
                {
                    obj = new Bomb(root, object["name"]);
                    obj->setPosition(position);
                    constructSubParts(obj, std::string(object["name"]) + ".json");
                }
                else if (object["type"] == "DOOR")
                {
                    obj = new Door(root, object["name"]);
                    LineNodeBuilder builder;
                    builder.addLoop(json, object, 2.0);
                    builder.create(obj, LineNodeBuilder::CollisionType::Chains);
                }
                else if (object["type"] == "LASER")
                {
                    obj = new Laser(root, object["name"]);
                    obj->setPosition(position);
                }
                else if (object["type"] == "PUSHLASER")
                {
                    obj = new PushLaser(root, object["name"]);
                    obj->setPosition(position);
                }
                else if (object["type"] == "TRIGGER")
                {
                    float w = object["width"];
                    float h = -float(object["height"]);

                    sp::string target = object["name"];
                    sp::string source = "";
                    if (target.find(":") > -1)
                    {
                        source = target.substr(0, target.find(":"));
                        target = target.substr(target.find(":") + 1);
                    }
                    obj = new Trigger(root, sp::Rect2d(position.x, position.y + h / th, w / tw, -h / th), source, target);
                }
                else if (object["type"] == "REMOTE_CONTROL")
                {
                    float w = object["width"];
                    float h = -float(object["height"]);

                    sp::string target = object["name"];
                    obj = new RemoteControl(root, sp::Rect2d(position.x, position.y + h / th, w / tw, -h / th), target);
                }
                else if (object["type"] == "TIMER")
                {
                    sp::string target = object["name"];
                    obj = new Timer(root, target);
                }
                else if (object["type"] == "GRAVITY")
                {
                    level_info.center_point_gravity = true;
                    level_info.gravity_center = position;
                }
                else if (object["type"] != "")
                {
                    LOG(Warning, "Unknown object type:", object["type"]);
                }
                if (obj)
                {
                    if (object.find("properties") != object.end())
                        for(const auto& prop : object["properties"])
                            obj->setProperty(prop["name"], prop["value"]);
                }
            }
        }

        level_info.camera_view_range = sp::Vector2d(std::max(0.0f, offset_x - 60), std::max(0.0f, offset_y - 60));
    }
}

void constructSubParts(sp::P<sp::Node> parent, sp::string json_file)
{
    auto json = nlohmann::json::parse(sp::io::ResourceProvider::get(json_file)->readAll());

    float tw = json["tilewidth"];
    float th = json["tileheight"];
    float offset_x = float(json["width"]) / 2.0;
    float offset_y = float(json["height"]) / 2.0;
    for(const auto& layer : json["layers"])
    {
        for(const auto& object : layer["objects"])
        {
            float x = object["x"];
            float y = -float(object["y"]);
            sp::Vector2d position(x / tw - offset_x, y / th + offset_y);
            
            sp::P<LevelObject> obj;
            if (object["type"] == "LASER")
            {
                obj = new Laser(parent, object["name"]);
                obj->setPosition(position);
            }
            else if (object["type"] == "PUSHLASER")
            {
                obj = new PushLaser(parent, object["name"]);
                obj->setPosition(position);
            }
            else if (object["type"] == "WHEEL")
            {
                float radius = sp::stringutil::convert::toFloat(object["name"]);
                obj = new Wheel(parent, position, radius);
            }
            else if (object["type"] != "")
            {
                LOG(Warning, "Unknown sub part object type:", object["type"]);
            }
            if (obj)
            {
                for(const auto& prop : object["properties"])
                    obj->setProperty(prop["name"], prop["value"]);
            }
        }
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

sp::P<sp::Node> addDecoration(sp::P<sp::Node> root, sp::Vector2d position, sp::Vector2f size, sp::string name)
{
    sp::Vector2f coordinate_size = size / 10.0f; //The coordinate system seems to use 10 times the render size.
    sp::P<sp::Node> node = new sp::Node(root);
    node->render_data.type = sp::RenderData::Type::Additive;
    node->render_data.shader = sp::Shader::get("internal:basic.shader");
    node->render_data.mesh = sp::MeshData::createQuad(coordinate_size);
    node->render_data.texture = sp::texture_manager.get("decorations/" + name + ".png");
    node->setPosition(sp::Vector2d(position.x + coordinate_size.x / 2, position.y)); //Centered around middle of rectangle.
    return node;
}
