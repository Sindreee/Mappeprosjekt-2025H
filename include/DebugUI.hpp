#ifndef MAPPEOPPGAVE_DEBUG_UI_HPP
#define MAPPEOPPGAVE_DEBUG_UI_HPP

#include <memory>

struct GLFWwindow;
class Game;

class DebugUI {
public:
    explicit DebugUI(Game& game, GLFWwindow* window);
    ~DebugUI();

    void newFrame();
    void render();

private:
    Game& game_;
    GLFWwindow* window_{};
    bool initialized_ = false;

    void initBackend();
    void shutdownBackend();

    void drawMainPanel();
    void drawMinimapPanel();
};

#endif
