#include "levelSelect.h"
#include "main.h"
#include "lineNodeBuilder.h"
#include "levelScene.h"

#include <sp2/scene/camera.h>
#include <sp2/tween.h>
#include <sp2/graphics/textureManager.h>


class LevelNode : public sp::Node
{
public:
    LevelNode(sp::P<sp::Node> parent, sp::string level_name)
    : sp::Node(parent), level_name(level_name)
    {
        LineNodeBuilder builder;
        builder.addLoop({{-5, -5}, {5,-5}, {5,5}, {-5,5}});
        builder.create(this, LineNodeBuilder::CollisionType::None);
        render_data.color = sp::Color(0.8, 1.0, 0.8);
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
    for(int n=0; n<4; n++)
    {
        sp::P<LevelNode> next = new LevelNode(getRoot(), "level" + sp::string(n+1));
        next->setPosition(sp::Vector2d(n * 30, (n % 2) * -10));
        if (prev)
            new LevelNodeLink(prev, next);
        else
            selection = next;
        prev = next;
    }

    camera = new sp::Camera(getRoot());
    camera->setOrtographic(60);
    setDefaultCamera(camera);
}

void LevelSelect::onFixedUpdate()
{
    for(int n=0; n<2; n++)
    {
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
    }
    camera->setPosition(camera->getPosition2D() * 0.8 + selection->getPosition2D() * 0.2);
}
