#ifndef LINE_NODE_BUILDER_H
#define LINE_NODE_BUILDER_H

#include <sp2/scene/node.h>
#include <sp2/graphics/meshdata.h>
#include <sp2/collision/2d/polygon.h>
#include <sp2/collision/2d/chains.h>
#include <nlohmann/json.hpp>


class LineNodeBuilder
{
public:
    enum class CollisionType
    {
        None,
        Polygon,
        Chains,
    };

    void loadFrom(sp::string filename, float line_width=1.0, sp::string type="");
    void addLoop(const nlohmann::json& base_json, const nlohmann::json& object_json, float line_width=1.0);
    void addLoop(const std::vector<sp::Vector2f>& points, float line_width=1.0);
    void create(sp::P<sp::Node> node, CollisionType collision_type);
private:
    sp::MeshData::Vertices vertices;
    sp::MeshData::Indices indices;
    
    sp::collision::Polygon2D polygon;
    sp::collision::Chains2D chains;
};

#endif//LINE_NODE_BUILDER_H
