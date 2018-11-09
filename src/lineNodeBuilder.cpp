#include "lineNodeBuilder.h"

#include <sp2/assert.h>
#include <sp2/graphics/textureManager.h>


void LineNodeBuilder::loadFrom(sp::string filename, float line_width, sp::string type)
{
    std::string err;
    json11::Json json = json11::Json::parse(sp::io::ResourceProvider::get(filename)->readAll(), err);
    
    float tw = json["tilewidth"].number_value();
    float th = json["tileheight"].number_value();
    float offset_x = json["width"].number_value() / 2.0;
    float offset_y = json["height"].number_value() / 2.0;
    for(const auto& layer : json["layers"].array_items())
    {
        for(const auto& object : layer["objects"].array_items())
        {
            if (sp::string(object["type"].string_value()) != type)
                continue;
            float x = object["x"].number_value();
            float y = -object["y"].number_value();
            std::vector<sp::Vector2f> points;
            for(const auto& point : object["polygon"].array_items())
                points.emplace_back((x + point["x"].number_value()) / tw - offset_x, (y-point["y"].number_value()) / th + offset_y);
            if (points.size() > 0)
                addLoop(points, line_width);
        }
    }
}

void LineNodeBuilder::addLoop(const json11::Json& base_json, const json11::Json& object_json, float line_width)
{
    float tw = base_json["tilewidth"].number_value();
    float th = base_json["tileheight"].number_value();
    float offset_x = base_json["width"].number_value() / 2.0;
    float offset_y = base_json["height"].number_value() / 2.0;
    float x = object_json["x"].number_value();
    float y = -object_json["y"].number_value();
    std::vector<sp::Vector2f> points;
    for(const auto& point : object_json["polygon"].array_items())
        points.emplace_back((x + point["x"].number_value()) / tw - offset_x, (y-point["y"].number_value()) / th + offset_y);
    if (points.size() > 0)
        addLoop(points, line_width);
}

void LineNodeBuilder::addLoop(const std::vector<sp::Vector2f>& points, float line_width)
{
    sp2assert(points.size() > 2, "Need at least 3 points for a line mesh builder");
    
    line_width /= 2.0f;
    
    indices.reserve(points.size() * 6);
    vertices.reserve(points.size() * 4);

    chains.loops.push_back(points);
    
    int idx0 = vertices.size();
    sp::Vector2f p0 = points[points.size() - 2];
    sp::Vector2f p1 = points[points.size() - 1];
    for(sp::Vector2f p2 : points)
    {
        polygon.add(p2);

        sp::Vector2f normal0 = (p1-p0).normalized();
        sp::Vector2f normal1 = (p2-p1).normalized();
        normal0 = sp::Vector2f(-normal0.y, normal0.x);
        normal1 = sp::Vector2f(-normal1.y, normal1.x);
        
        float f1 = 1.0f / std::cos((normal1.angle() - normal0.angle()) / 2.0f / 180.0f * float(sp::pi));
        normal0 = (normal1 + normal0).normalized() * std::abs(f1);

        sp::Vector2f v0 = p1 - normal0 * line_width;
        sp::Vector2f v1 = p1 + normal0 * line_width;
        
        vertices.emplace_back(sp::Vector3f(v0.x, v0.y, 0), sp::Vector2f(1, 0));
        vertices.emplace_back(sp::Vector3f(v1.x, v1.y, 0), sp::Vector2f(0, 0));

        p0 = p1;
        p1 = p2;
    }
    
    unsigned int vertex_count = points.size() * 2;
    for(unsigned int n=0; n<points.size(); n++)
    {
        indices.emplace_back(idx0 + (n * 2 + 0) % vertex_count);
        indices.emplace_back(idx0 + (n * 2 + 2) % vertex_count);
        indices.emplace_back(idx0 + (n * 2 + 1) % vertex_count);
        indices.emplace_back(idx0 + (n * 2 + 1) % vertex_count);
        indices.emplace_back(idx0 + (n * 2 + 2) % vertex_count);
        indices.emplace_back(idx0 + (n * 2 + 3) % vertex_count);
    }
}

void LineNodeBuilder::create(sp::P<sp::Node> node, CollisionType collision_type)
{
    chains.friction = 0.9;
    polygon.friction = 0.9;

    node->render_data.type = sp::RenderData::Type::Additive;
    node->render_data.shader = sp::Shader::get("internal:basic.shader");
    node->render_data.mesh = sp::MeshData::create(std::move(vertices), std::move(indices));
    node->render_data.texture = sp::texture_manager.get("line.png");
    
    switch(collision_type)
    {
    case CollisionType::None:
        break;
    case CollisionType::Polygon:
        node->setCollisionShape(polygon);
        break;
    case CollisionType::Chains:
        node->setCollisionShape(chains);
        break;
    }
}
