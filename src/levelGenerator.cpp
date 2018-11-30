#include "levelGenerator.h"
#include "levelScene.h"
#include "spaceship.h"
#include "main.h"
#include "lineNodeBuilder.h"

#include <sp2/assert.h>
#include <stdio.h>
#include "clipper/clipper.hpp"


LevelGenerator::LevelGenerator(sp::P<sp::Node> parent)
: sp::Node(parent)
{
    level_info.camera_view_range = sp::Vector2d(std::numeric_limits<double>::max(), std::numeric_limits<double>::max());

    FILE* f = fopen("X.trophy", "rb");
    if (f)
    {
        fread(&level_info.depth_trophy, sizeof(level_info.depth_trophy), 1, f);
        fclose(f);
    }

    for(int n=0; n<2; n++)
    {
        sp::P<Spaceship> spaceship = new Spaceship(parent);
        spaceship->setPosition(sp::Vector2d(n * 10, 0));
        spaceship->setControls(&controls[n]);
    }
    
    map_index_min = 0;
    map_index_max = 3;
    buildLevel(map_index_min, map_index_max);
}


void LevelGenerator::buildLevel(int min_index, int max_index)
{
    std::vector<std::vector<sp::Vector2f>> paths;
    for(int n=min_index; n<max_index+1; n++)
        combinePaths(paths, getMapSection(n));

    LineNodeBuilder builder;
    for(auto& path : paths)
        builder.addLoop(path, 2.0);
    builder.create(this, LineNodeBuilder::CollisionType::Chains);
}

void LevelGenerator::onFixedUpdate()
{
    const float spawn_range = 150;
    const float despawn_range = 300;
    sp::Vector2f camera_position = sp::Vector2f(getScene()->getCamera()->getPosition2D());

    if ((entry_points[map_index_max] - camera_position).length() < spawn_range)
    {
        LOG(Debug, "Growing level at back!");
        map_index_max += 1;
        buildLevel(map_index_min, map_index_max);
    }
    if (map_index_max > map_index_min && (entry_points[map_index_max] - camera_position).length() > despawn_range)
    {
        LOG(Debug, "Shrinking level at back!");
        map_index_max -= 1;
        buildLevel(map_index_min, map_index_max);
    }
    
    if (map_index_min > 0 && (entry_points[map_index_min] - camera_position).length() < spawn_range)
    {
        LOG(Debug, "Growing level at front!");
        map_index_min -= 1;
        buildLevel(map_index_min, map_index_max);
    }
    
    if (map_index_min < map_index_max && (entry_points[map_index_min] - camera_position).length() > despawn_range)
    {
        LOG(Debug, "Shrinking level at front!");
        map_index_min += 1;
        buildLevel(map_index_min, map_index_max);
    }
}

std::vector<sp::Vector2f> LevelGenerator::getMapSection(unsigned int index)
{
    std::vector<sp::Vector2f> result;
    if (cave_seeds.size() < index + 1)
        cave_seeds.push_back(main_random_engine());
    
    sp2assert(entry_points.size() >= index, "Internal logic error, indexes not requested in sequence");
    section_random.seed(cave_seeds[index]);
    
    sp::Vector2f exit;
    sp::Vector2f entry;
    if (index > 0)
        entry = entry_points[index - 1];
    
    if (index == 0)
    {
        for(int n=-70; n<70; n+=10)
            result.emplace_back(n, -40 + rnd(-5, 5));
        result.emplace_back(100, 100);
        result.emplace_back(-100, 100);
        exit = sp::Vector2f(0, -40);
    }
    else
    {
        float length = 100;
        if (index < 3)
            exit = entry + sp::Vector2f(0, -length).rotate(rnd(-45, 45));
        else
            exit = entry + sp::Vector2f(0, -length).rotate(rnd(-90, 90));
        float width = rnd(8, 12);
        if (index < 15)
            width += 15 - index;
        
        sp::Vector2f forward = (exit - entry).normalized();
        sp::Vector2f side = forward.rotate(90);
        
        result.push_back(entry - forward * width);
        result.push_back(entry + side * width);
        for(int n=1; n<length/10; n++)
            result.push_back(entry + forward * (n * 10.0f) + side * rnd(width - 5, width + 5));
        result.push_back(exit + side * width);
        result.push_back(exit + forward * width);
        result.push_back(exit - side * width);
        for(int n=1; n<length/10; n++)
            result.push_back(exit - forward * (n * 10.0f) - side * rnd(width - 5, width + 5));
        result.push_back(entry - side * width);
    }
    while(entry_points.size() < index + 1)
        entry_points.push_back(exit);
    
    return result;
}

void LevelGenerator::combinePaths(std::vector<std::vector<sp::Vector2f>>& output, const std::vector<sp::Vector2f>& input)
{
    if (output.size() == 0)
    {
        output.push_back(input);
        return;
    }
    ClipperLib::Paths paths0;
    ClipperLib::Path path1;
    for(auto& path : output)
    {
        ClipperLib::Path path0;
        for(auto p0 : path)
            path0.emplace_back(p0.x, p0.y);
        paths0.emplace_back(std::move(path0));
    }
    for(sp::Vector2f p0 : input)
        path1.emplace_back(p0.x, p0.y);
    
    ClipperLib::Paths result;
    ClipperLib::Clipper clipper;
    clipper.AddPaths(paths0, ClipperLib::ptSubject, true);
    clipper.AddPath(path1, ClipperLib::ptClip, true);
    clipper.Execute(ClipperLib::ctUnion, result);
    
    output.clear();
    for(ClipperLib::Path& path : result)
    {
        std::vector<sp::Vector2f> outpath;
        for(ClipperLib::IntPoint p0 : path)
            outpath.emplace_back(p0.X, p0.Y);
        output.emplace_back(std::move(outpath));
    }
}

float LevelGenerator::rnd(float fmin, float fmax)
{
    return std::uniform_real_distribution<>(fmin, fmax)(section_random);
}
