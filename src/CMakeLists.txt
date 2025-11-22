add_library(MappeoppgaveLib
        car.cpp
        carKeyController.cpp
        turret.cpp
        CollisionDetection.cpp
        bears.cpp
        bullet.cpp
        GameObject.cpp
        Game.cpp
        CameraRig.cpp
        BlurOverlay.cpp
        WorldManager.cpp
        Wolf.cpp
        DebugUI.cpp
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# Add GLFW include directory from threepp's bundled GLFW
target_include_directories(MappeoppgaveLib PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${threepp_SOURCE_DIR}/src/external/glfw/include
)

target_link_libraries(MappeoppgaveLib PUBLIC threepp assimp)

