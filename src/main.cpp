#include <sp2/engine.h>
#include <sp2/window.h>
#include <sp2/logging.h>
#include <sp2/assert.h>
#include <sp2/random.h>
#include <sp2/io/directoryResourceProvider.h>
#include <sp2/audio/music.h>
#include <sp2/graphics/gui/widget/button.h>
#include <sp2/graphics/gui/scene.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/gui/loader.h>
#include <sp2/graphics/scene/graphicslayer.h>
#include <sp2/graphics/scene/basicnoderenderpass.h>
#include <sp2/graphics/scene/collisionrenderpass.h>
#include <sp2/graphics/spriteAnimation.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/graphics/meshbuilder.h>
#include <sp2/audio/sound.h>
#include <sp2/scene/scene.h>
#include <sp2/scene/node.h>
#include <sp2/scene/particleEmitter.h>
#include <sp2/scene/camera.h>
#include <sp2/collision/2d/box.h>
#include <sp2/collision/2d/circle.h>
#include <sp2/collision/2d/polygon.h>
#include <sp2/collision/2d/chains.h>
#include <sp2/collision/2d/joint.h>
#include <sp2/io/keybinding.h>

#include <json11/json11.hpp>

#include "lineNodeBuilder.h"
#include "physicsObject.h"
#include "controls.h"
#include "spaceship.h"

sp::Vector2d gravity;

sp::P<sp::Window> window;

Controls controls[2]{{0}, {1}};
sp::io::Keybinding escape_key{"exit", "Escape"};


class LevelScene : public sp::Scene
{
public:
    LevelScene()
    : sp::Scene("LEVEL")
    {
    }
    
    void loadLevel(sp::string name)
    {
        {
            for(auto obj : getRoot()->getChildren())
                delete obj;
            level_name = name;
            gravity = sp::Vector2d(0, -0.15);
            target_areas.clear();
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
    
    sp::P<sp::Node> addIcon(sp::Vector2d position, sp::string name)
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
    
    virtual void onFixedUpdate()
    {
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
                if (target_objects.size() > 0 || !inTargetArea(position) || (player->getLinearVelocity2D() - gravity).length() > 0.05)
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
                if (!inTargetArea(position) || (target->getLinearVelocity2D() - gravity).length() > 0.05)
                    in_target = false;
            }
        }
        
        view_position /= double(players.size());
        view_position.x = std::min(camera_view_range.x, view_position.x);
        view_position.x = std::max(-camera_view_range.x, view_position.x);
        view_position.y = std::min(camera_view_range.y, view_position.y);
        view_position.y = std::max(-camera_view_range.y, view_position.y);
        camera->setPosition(view_position);

        if (!alive || in_target)
        {
            if (end_level_countdown > 0)
                end_level_countdown--;
            else
                loadLevel(level_name);
        }
        else
        {
            end_level_countdown = 60 * 2;
        }
    }
    
    bool inTargetArea(sp::Vector2d position)
    {
        for(auto& target_area : target_areas)
            if (target_area.contains(position))
                return true;
        return false;
    }
    
private:
    sp::PList<Spaceship> players;
    sp::PList<sp::Node> target_objects;
    sp::P<sp::Camera> camera;
    sp::Vector2d camera_view_range;
    int end_level_countdown;
    sp::string level_name;
    std::vector<sp::Rect2d> target_areas;
};

int main(int argc, char** argv)
{
    sp::P<sp::Engine> engine = new sp::Engine();

    //Create resource providers, so we can load things.
    new sp::io::DirectoryResourceProvider("resources");

    sp::texture_manager.setDefaultSmoothFiltering(false);

    //Create a window to render on
    window = new sp::Window(4.0/3.0);
#ifndef DEBUG
    window->setFullScreen(true);
    window->hideCursor();
#endif
    
    sp::gui::Theme::loadTheme("default", "gui/theme/basic.theme.txt");
    new sp::gui::Scene(sp::Vector2d(256, 192), sp::gui::Scene::Direction::Horizontal);

    sp::P<sp::SceneGraphicsLayer> scene_layer = new sp::SceneGraphicsLayer(1);
    scene_layer->addRenderPass(new sp::BasicNodeRenderPass());
#ifdef DEBUG
    scene_layer->addRenderPass(new sp::CollisionRenderPass());
#endif
    window->addLayer(scene_layer);

    sp::P<LevelScene> level_scene = new LevelScene();
    
    level_scene->loadLevel("level4");
    
    engine->run();
    
    return 0;
}
