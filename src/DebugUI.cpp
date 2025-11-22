#include "DebugUI.hpp"
#include "Game.hpp"
#include "WorldManager.hpp"
#include "carKeyController.hpp"
#include "car.hpp"
#include "Wolf.hpp"

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <cmath>

using namespace threepp;

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
        ImGui::Text("Car hits: %d / %d", game_.carHitCount_, game_.carHitsToDie_);
        ImGui::Text("Blur: %.2f", game_.blurLevel_);
        ImGui::Text("Initialized: %s", game_.initialized_ ? "true" : "false");

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
            // Level changed - show info
            float baseSpeed = 50.0f + (game_.difficultyLevel_ - 1) * 40.0f;
            ImGui::SetTooltip("Wolf base speed: %.1f", baseSpeed);
        }
        ImGui::Text("(Changes apply on restart)");

        ImGui::Separator();
        ImGui::Text("Wolves Status:");
        int i = 0;
        for (auto& w : game_.worldManager_->getWolves()) {
            ImGui::Text("Wolf %d: alive=%s hits=%d", i++, w.alive ? "yes" : "no", w.hits);
        }
    }
    ImGui::End();
}

void DebugUI::drawMinimapPanel() {
    const float mapSize = 200.f;
    const float minW = -2000.f, maxW = 2000.f;
    const float scale = mapSize / (maxW - minW);

    ImGui::SetNextWindowPos({320, 10}, ImGuiCond_Appearing);
    ImGui::SetNextWindowSize({mapSize + 10, mapSize + 40}, ImGuiCond_Appearing);
    if (ImGui::Begin("Minimap")) {
        ImVec2 p0 = ImGui::GetCursorScreenPos();
        ImVec2 p1{p0.x + mapSize, p0.y + mapSize};
        auto* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(p0, p1, IM_COL32(30,30,50,255));
        dl->AddRect(p0, p1, IM_COL32(200,200,200,255));
        auto toMap = [&](float wx, float wz){ return ImVec2(p0.x + (wx - minW) * scale, p0.y + (wz - minW) * scale); };

        auto cpos = game_.car_.getPosition();
        ImVec2 c = toMap(cpos.x, cpos.z);
        dl->AddCircleFilled(c, 5, IM_COL32(0,200,255,255));
        float ang = game_.car_.getGroup()->rotation.y;
        ImVec2 tip{ c.x + sin(ang)*12.f, c.y + cos(ang)*12.f };
        dl->AddLine(c, tip, IM_COL32(0,255,255,255), 2.f);

        for (auto& w : game_.worldManager_->getWolves()) {
            if (!w.alive) continue;
            auto wp = w.object->position;
            ImVec2 wm = toMap(wp.x, wp.z);
            dl->AddCircleFilled(wm, 4, IM_COL32(255,50,50,255));
        }

        // Draw trees (sample every 10th to avoid clutter)
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
