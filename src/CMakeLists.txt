
add_library(MappeoppgaveLib
        car.cpp
        carKeyController.cpp
        turret.cpp
        CollisionDetection.cpp
        Bear.cpp
        bullet.cpp
        GameObject.cpp
        Game.cpp
        CameraRig.cpp
        BlurOverlay.cpp
        WorldManager.cpp
        Wolf.cpp
        DebugUI.cpp
)


target_include_directories(MappeoppgaveLib PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)



target_link_libraries(MappeoppgaveLib PUBLIC
        threepp
        assimp
        ImGuiLib
)
