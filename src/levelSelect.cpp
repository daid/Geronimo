#include "levelSelect.h"
#include "main.h"
#include "lineNodeBuilder.h"
#include "levelScene.h"

#include <sp2/random.h>
#include <sp2/scene/camera.h>
#include <sp2/engine.h>
#include <sp2/tween.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/fontManager.h>
#include <sp2/graphics/gui/loader.h>


class LevelNode : public sp::Node
{
public:
    LevelNode(sp::P<sp::Node> parent, sp::string level_name, sp::string label)
    : sp::Node(parent), level_name(level_name)
    {
        LineNodeBuilder builder;
        builder.addLoop({{-5, -5}, {5,-5}, {5,5}, {-5,5}});
        builder.create(this, LineNodeBuilder::CollisionType::None);
        render_data.color = sp::Color(0.8, 1.0, 0.8);
        
        if (label != "")
        {
            float size = 5.0f;
            if (label.length() > 2)
                size = 3.5f;
            sp::P<sp::Node> node = new sp::Node(this);
            node->render_data.type = sp::RenderData::Type::Additive;
            node->render_data.shader = sp::Shader::get("internal:basic.shader");
            node->render_data.mesh = sp::font_manager.get("gui/theme/KenVector Bold.ttf")->createString(label, 32, size, sp::Vector2d(0, 0), sp::Alignment::Bottom);
            node->render_data.texture = sp::font_manager.get("gui/theme/KenVector Bold.ttf")->getTexture(32);
            node->render_data.color = sp::Color(0.8, 1.0, 0.8);
            node->setPosition(sp::Vector2d(0, -size/5.0f*3.0));
        }
    }
    
    sp::string level_name;
    sp::P<LevelNode> left, right, up, down;
};

class LevelNodeLink : public sp::Node
{
public:
    LevelNodeLink(sp::P<LevelNode> source, sp::P<LevelNode> target)
    : sp::Node(source)
    {
        sp::MeshData::Vertices vertices;
        sp::MeshData::Indices indices{0,1,2,2,1,3};
        vertices.reserve(4);
        
        vertices.emplace_back(sp::Vector3f(0,-1, 0), sp::Vector2f(1, 0));
        vertices.emplace_back(sp::Vector3f(1,-1, 0), sp::Vector2f(1, 1));
        vertices.emplace_back(sp::Vector3f(0, 1, 0), sp::Vector2f(0, 0));
        vertices.emplace_back(sp::Vector3f(1, 1, 0), sp::Vector2f(0, 1));

        render_data.type = sp::RenderData::Type::Additive;
        render_data.shader = sp::Shader::get("rope.shader");
        render_data.mesh = sp::MeshData::create(std::move(vertices), std::move(indices));
        render_data.texture = sp::texture_manager.get("line.png");
        render_data.color = sp::Color(0.8, 1.0, 0.8);
        
        sp::Vector2d diff = target->getGlobalPosition2D() - getGlobalPosition2D();
        if (std::abs(diff.x) > std::abs(diff.y))
        {
            diff.x -= std::copysign(10.0, diff.x);
            setPosition(sp::Vector2d(std::copysign(-5, diff.x), 0));
            if (diff.x > 0) { source->right = target; target->left = source; }
            else { source->left = target; target->right = source; }
        }
        else
        {
            diff.y -= std::copysign(10.0, diff.y);
            setPosition(sp::Vector2d(0, std::copysign(-5, diff.y)));
            if (diff.y > 0) { source->up = target; target->down = source; }
            else { source->down = target; target->up = source; }
        }
        render_data.scale.x = diff.x;
        render_data.scale.y = diff.y;
    }
};


LevelSelect::LevelSelect()
: sp::Scene("LEVEL_SELECT")
{
    sp::P<LevelNode> prev;
    for(int n=0; sp::io::ResourceProvider::get("levels/level" + sp::string(n+1) + ".json") != nullptr; n++)
    {
        sp::P<LevelNode> next = new LevelNode(getRoot(), "level" + sp::string(n+1), sp::string(n + 1));
        all_levels.add(next);
        next->setPosition(sp::Vector2d(n * 30, (n % 2) * -10));
        if (prev)
        {
            new LevelNodeLink(prev, next);
        }
        else
        {
            selection = next;

            sp::P<LevelNode> level_x = new LevelNode(getRoot(), "X", "X");
            level_x->setPosition(sp::Vector2d(-70, -30));
            new LevelNodeLink(level_x, selection);
        }

        prev = next;
        for(char c='b'; sp::io::ResourceProvider::get("levels/level" + sp::string(n+1) + sp::string(c) + ".json") != nullptr; c++)
        {
            sp::P<LevelNode> sub = new LevelNode(getRoot(), "level" + sp::string(n+1) + sp::string(c), sp::string(n + 1) + sp::string(c));
            all_levels.add(sub);
            sub->setPosition(prev->getPosition2D() + sp::Vector2d(10, -30));
            new LevelNodeLink(prev, sub);
            prev = sub;
        }
        prev = next;
    }

    gui = sp::gui::Loader::load("gui/level_select_hud.gui", "HUD");
    gui->getWidgetWithID("FUEL")->getWidgetWithID("PHOTO")->setRotation(-90);
    gui->getWidgetWithID("TIME")->getWidgetWithID("PHOTO")->setRotation(-90);
    gui->getWidgetWithID("DEPTH")->getWidgetWithID("PHOTO")->setRotation(-90);

    camera = new sp::Camera(getRoot());
    camera->setOrtographic(60);
    setDefaultCamera(camera);
    
    {
        sp::P<sp::Node> node = new sp::Node(camera);
        LineNodeBuilder builder;
        builder.addLoop({{-6, -6}, {6,-6}, {6,6}, {-6,6}});
        builder.create(node, LineNodeBuilder::CollisionType::None);
        node->render_data.color = sp::Color(0.8, 1.0, 0.8);
    }
    
    updateTrophys();
}

void LevelSelect::onFixedUpdate()
{
    sp::P<sp::Node> old_selection = selection;
    for(int n=0; n<2; n++)
    {
        if (controls[n].left.get() || controls[n].right.get() || controls[n].up.get() || controls[n].down.get())
            auto_replay_countdown = auto_replay_delay;
        if (controls[n].right.getDown() && selection->right) selection = selection->right;
        if (controls[n].left.getDown() && selection->left) selection = selection->left;
        if (controls[n].up.getDown() && selection->up) selection = selection->up;
        if (controls[n].down.getDown() && selection->down) selection = selection->down;

        if (controls[n].secondary_action.getDown())
        {
            sp::P<LevelScene> level_scene = sp::Scene::get("LEVEL");
            level_scene->loadLevel(selection->level_name);
            level_scene->enable();
            disable();
        }

        if (controls[n].replay_fuel.getDown() || controls[n].replay_time.getDown())
        {
            sp::string replay_file;
            if (controls[n].replay_fuel.getDown())
                replay_file = selection->level_name + "-fuel.replay";
            else
                replay_file = selection->level_name + "-time.replay";
            startReplay(replay_file);
        }
    }
    if (old_selection != selection)
    {
        updateTrophys();
    }
    
    if (escape_key.getDown())
        sp::Engine::getInstance()->shutdown();
    
    if (auto_replay_countdown > 0)
    {
        auto_replay_countdown--;
    }
    else
    {
        startRandomReplay();
    }

    camera->setPosition(camera->getPosition2D() * 0.8 + selection->getPosition2D() * 0.2);
}

void LevelSelect::onEnable()
{
    updateTrophys();
    gui->show();
    auto_replay_countdown = auto_replay_delay;
}

void LevelSelect::onDisable()
{
    gui->hide();
}

void LevelSelect::startRandomReplay()
{
    int index = sp::irandom(0, all_levels.size() - 1);
    for (LevelNode* level : all_levels)
    {
        if (index > 0)
        {
            index--;
        }
        else
        {
            sp::string replay_file;
            if (sp::random(0, 100) < 50)
                replay_file = level->level_name + "-fuel.replay";
            else
                replay_file = level->level_name + "-time.replay";
            startReplay(replay_file);
            return;
        }
    }
}

bool LevelSelect::startReplay(sp::string filename)
{
    LOG(Debug, filename);
    FILE* f_existance_test = fopen(filename.c_str(), "r");
    if (!f_existance_test)
        return false;
    fclose(f_existance_test);

    sp::P<LevelScene> level_scene = sp::Scene::get("LEVEL");
    level_scene->loadLevel(selection->level_name, true, filename);
    level_scene->enable();
    disable();
    return true;
}

void LevelSelect::updateTrophys()
{
    gui->getWidgetWithID("FUEL")->setVisible(sp::io::ResourceProvider::get(selection->level_name + ".trophy.fuel.png") != nullptr);
    gui->getWidgetWithID("FUEL")->getWidgetWithID("PHOTO")->setAttribute("texture", selection->level_name + ".trophy.fuel.png");
    gui->getWidgetWithID("TIME")->setVisible(sp::io::ResourceProvider::get(selection->level_name + ".trophy.time.png") != nullptr);
    gui->getWidgetWithID("TIME")->getWidgetWithID("PHOTO")->setAttribute("texture", selection->level_name + ".trophy.time.png");
    gui->getWidgetWithID("DEPTH")->setVisible(sp::io::ResourceProvider::get(selection->level_name + ".trophy.depth.png") != nullptr);
    gui->getWidgetWithID("DEPTH")->getWidgetWithID("PHOTO")->setAttribute("texture", selection->level_name + ".trophy.depth.png");
}
