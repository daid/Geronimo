#include <sp2/engine.h>
#include <sp2/window.h>
#include <sp2/logging.h>
#include <sp2/assert.h>
#include <sp2/random.h>
#include <sp2/io/directoryResourceProvider.h>
#include <sp2/io/virtualTouchKeys.h>
#include <sp2/graphics/gui/widget/button.h>
#include <sp2/graphics/gui/scene.h>
#include <sp2/graphics/gui/theme.h>
#include <sp2/graphics/gui/loader.h>
#include <sp2/graphics/scene/graphicslayer.h>
#include <sp2/graphics/scene/basicnoderenderpass.h>
#include <sp2/graphics/scene/collisionrenderpass.h>
#include <sp2/graphics/textureManager.h>
#include <sp2/graphics/meshdata.h>

#include <nlohmann/json.hpp>

#include "lineNodeBuilder.h"
#include "physicsObject.h"
#include "controls.h"
#include "spaceship.h"
#include "levelScene.h"
#include "levelSelect.h"

sp::P<sp::Window> window;

Controls controls[2]{{0}, {1}};
sp::io::Keybinding escape_key{"exit", "Escape"};


int main(int argc, char** argv)
{
    sp::P<sp::Engine> engine = new sp::Engine();
    
    //Create resource providers, so we can load things.
    new sp::io::DirectoryResourceProvider("resources");
    new sp::io::DirectoryResourceProvider("photos");

    sp::texture_manager.setDefaultSmoothFiltering(false);

    //Create a window to render on
    window = new sp::Window(4.0/3.0);
#ifndef DEBUG
    window->setFullScreen(true);
    window->hideCursor();
#endif
    
    sp::gui::Theme::loadTheme("default", "gui/theme/basic.theme.txt");
    new sp::gui::Scene(sp::Vector2d(320, 240));

    sp::P<sp::SceneGraphicsLayer> scene_layer = new sp::SceneGraphicsLayer(1);
    scene_layer->addRenderPass(new sp::BasicNodeRenderPass());
#ifdef DEBUG
    scene_layer->addRenderPass(new sp::CollisionRenderPass());
#endif
    window->addLayer(scene_layer);

#ifdef ANDROID
    sp::io::VirtualTouchKeyLayer* touch_layer = new sp::io::VirtualTouchKeyLayer(50);
    touch_layer->addButton(sp::Rect2f(sp::Vector2f(0, 0), sp::Vector2f(0.5, 1.0)), controls[0].left);
    touch_layer->addButton(sp::Rect2f(sp::Vector2f(0.5, 0), sp::Vector2f(0.5, 1)), controls[0].right);
    window->addLayer(touch_layer);
#endif

    new LevelScene();
    new LevelSelect();
    engine->run();
    
    return 0;
}
