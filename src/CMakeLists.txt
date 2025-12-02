# src/CMakeLists.txt

# Lag bibliotek av din kode
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

# Hvor skal den finne header-filer?
target_include_directories(MappeoppgaveLib PUBLIC
        ${CMAKE_SOURCE_DIR}/include
)

# Hvem er dette biblioteket avhengig av?
# Her linker vi bare til "ImGuiLib" som vi lagde i rot-filen! Mye ryddigere.
target_link_libraries(MappeoppgaveLib PUBLIC
        threepp
        assimp
        ImGuiLib
)
