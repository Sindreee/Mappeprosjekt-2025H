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
)
target_include_directories(MappeoppgaveLib PUBLIC ${CMAKE_SOURCE_DIR}/include)

target_link_libraries(MappeoppgaveLib PUBLIC threepp assimp)