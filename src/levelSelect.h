#ifndef LEVEL_SELECT_H
#define LEVEL_SELECT_H

#include <sp2/scene/scene.h>


class LevelNode;
class LevelSelect : public sp::Scene
{
public:
    LevelSelect();
    
    virtual void onFixedUpdate() override;

private:
    sp::P<sp::Camera> camera;
    sp::P<LevelNode> selection;
};

#endif//LEVEL_SELECT_H
