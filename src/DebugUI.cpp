#include "DebugUI.hpp"
#include "Game.hpp"
#include "WorldManager.hpp"
#include "carKeyController.hpp"
#include "car.hpp"
#include "Wolf.hpp"
#include "GameConstants.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <cmath>

using namespace threepp;

// DebugUI was initally an AI driven experiment but due to time constraints it has been adapted as is
// without much changes only reconfiguring some parameters and ,changed to fit the program
DebugUI::DebugUI(Game& game, GLFWwindow* window): game_(game), window_(window) {
    if (!window) return;
    initBackend();
}

DebugUI::~DebugUI() {
    shutdownBackend();
}

void DebugUI::initBackend() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForOpenGL(window_, true);
    ImGui_ImplOpenGL3_Init("#version 130");
    initialized_ = true;
}

void DebugUI::shutdownBackend() {
    if (!initialized_) return;
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    initialized_ = false;
}

void DebugUI::newFrame() {
    if (!initialized_) return;
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void DebugUI::render() {
    if (!initialized_) return;
    drawMainPanel();
    drawMinimapPanel();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void DebugUI::drawMainPanel() {
    ImGui::SetNextWindowPos({10, 10}, ImGuiCond_Appearing);
    ImGui::SetNextWindowSize({300, 0}, ImGuiCond_Appearing);

    if (ImGui::Begin("Game")) {
        // Show game-over or victory status messages.
        if (game_.carDestroyed_) {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 0, 0, 255));
            ImGui::SetWindowFontScale(2.0f);
            ImGui::Text("GAME OVER!");
            ImGui::SetWindowFontScale(1.0f);
            ImGui::PopStyleColor();

            if (game_.worldManager_->allWolvesDead()) {
                 ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
                 ImGui::Text("BUT YOU WON!");
                 ImGui::PopStyleColor();
            } else {
                 ImGui::Text("The Deer Died.");
            }
            ImGui::Separator();
        } else if (game_.worldManager_->allWolvesDead()) {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 255, 0, 255));
            ImGui::SetWindowFontScale(2.0f);
            ImGui::Text("VICTORY!");
            ImGui::SetWindowFontScale(1.0f);
            ImGui::PopStyleColor();
            ImGui::Text("All wolves eliminated.");
            ImGui::Separator();
        }

        // ---Car health and blur status indicators.
        int hp = game_.car_.getHealth();
        int maxHp = Config::CAR_MAX_HEALTH;

        if (hp < maxHp * 0.3) ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 50, 50, 255));
        else if (hp < maxHp * 0.6) ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 255, 0, 255));
        else ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(50, 255, 50, 255));

        ImGui::Text("Car HP: %d / %d", hp, maxHp);
        ImGui::PopStyleColor();

        ImGui::Text("Blur: %.2f", game_.blurLevel_);

        if (ImGui::Button("Restart (R)")) {
            game_.restartGame();
        }

        if (game_.controller_) {
            float maxSpeed = game_.controller_->getMaxSpeed();
            ImGui::Text("Max Car Speed: %.1f", maxSpeed);
        }

        ImGui::Separator();
        ImGui::Text("Difficulty Level");
        if (ImGui::SliderInt("Level", &game_.difficultyLevel_, 1, 3)) {
            // Tooltip shows the derived base speed used for wolves at this difficulty.
            float baseSpeed = 50.0f + (game_.difficultyLevel_ - 1) * 40.0f;
            ImGui::SetTooltip("Wolf base speed: %.1f", baseSpeed);
        }
        ImGui::Text("(Changes apply on restart)");

        ImGui::Separator();
        ImGui::Text("Wolves Status:");
        int i = 0;

        for (const auto& w : game_.worldManager_->getWolves()) {
            ImVec4 color = w.isAlive() ? ImVec4(0,1,0,1) : ImVec4(1,0,0,1);
            ImGui::TextColored(color, "Wolf %d: %s (Hits: %d)", i++, w.isAlive() ? "Alive" : "Dead", w.getHits());
        }
    }
    ImGui::End();
}

void DebugUI::drawMinimapPanel() {
    const float mapSize = 200.f;
    const float minW = -Config::WORLD_BOUNDS, maxW = Config::WORLD_BOUNDS;
    const float scale = mapSize / (maxW - minW);

    ImGui::SetNextWindowPos({320, 10}, ImGuiCond_Appearing);
    ImGui::SetNextWindowSize({mapSize + 10, mapSize + 40}, ImGuiCond_Appearing);
    if (ImGui::Begin("Minimap")) {
        ImVec2 p0 = ImGui::GetCursorScreenPos();
        ImVec2 p1{p0.x + mapSize, p0.y + mapSize};
        auto* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(p0, p1, IM_COL32(30,30,50,255));
        dl->AddRect(p0, p1, IM_COL32(200,200,200,255));

        // 1. ---Helper to transform world XZ coordinates into minimap pixel space.
        auto toMap = [&](float wx, float wz){ return ImVec2(p0.x + (wx - minW) * scale, p0.y + (wz - minW) * scale); };

        // 2. ---Draw the car as a blue circle with a small heading arrow.
        auto cpos = game_.car_.getGroup()->position;
        ImVec2 c = toMap(cpos.x, cpos.z);
        dl->AddCircleFilled(c, 5, IM_COL32(0,200,255,255));
        float ang = game_.car_.getGroup()->rotation.y;
        ImVec2 tip{ c.x + sin(ang)*12.f, c.y + cos(ang)*12.f };
        dl->AddLine(c, tip, IM_COL32(0,255,255,255), 2.f);

        // 3. ---Draw living wolves as red dots.
        for (const auto& w : game_.worldManager_->getWolves()) {
            if (!w.isAlive()) continue;

            auto wp = w.getModel()->position;
            ImVec2 wm = toMap(wp.x, wp.z);
            dl->AddCircleFilled(wm, 4, IM_COL32(255,50,50,255));
        }

        // 4. ---Sample every 10th tree so the minimap stays readable.
        int ti = 0;
        for (auto& t : game_.worldManager_->getTrees()) {
            if (++ti % 10) continue;
            auto tp = t->getModel()->position;
            ImVec2 tm = toMap(tp.x, tp.z);
            dl->AddCircleFilled(tm, 2, IM_COL32(60,200,60,255));
        }
    }
    ImGui::End();
}