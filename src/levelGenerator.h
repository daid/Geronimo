#ifndef LEVEL_GENERATOR_H
#define LEVEL_GENERATOR_H

#include <sp2/scene/node.h>
#include <sp2/math/rect.h>
#include "spaceship.h"
#include <random>

class LevelGenerator : public sp::Node
{
public:
    LevelGenerator(sp::P<sp::Node> parent);
    
    virtual void onFixedUpdate() override;

private:
    void buildLevel(int min_index, int max_index);

    sp::PList<Spaceship> players;

    int fixed_frame_count; //Number of fixed updates since level initialized

    std::vector<sp::Vector2f> getMapSection(unsigned int index);
    void combinePaths(std::vector<std::vector<sp::Vector2f>>& output, const std::vector<sp::Vector2f>& input);
    float rnd(float min, float max);
    
    std::vector<uint32_t> cave_seeds;
    std::vector<sp::Vector2f> entry_points;
    int map_index_min;
    int map_index_max;
    
    std::mt19937_64 main_random_engine;
    std::mt19937_64 section_random;
};

#endif//LEVEL_GENERATOR_H
