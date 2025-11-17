cmake_minimum_required(VERSION 3.16)
project(Mappeoppgave)


set(CMAKE_CXX_STANDARD 20)

include(FetchContent)

set(THREEPP_BUILD_TESTS OFF)
set(THREEPP_BUILD_EXAMPLES OFF)

FetchContent_Declare(
        threepp

        GIT_REPOSITORY https://github.com/markaren/threepp.git

        GIT_TAG 619cdaacc2e7dc6ea709d8c7f482e2994af7349d
)
FetchContent_MakeAvailable(threepp)


FetchContent_Declare(
        assimp

        GIT_REPOSITORY https://github.com/assimp/assimp.git

        GIT_TAG cc6ab1408fa0dc7b386f5b8e5883c3cedcae950b
)

FetchContent_MakeAvailable(assimp)

add_subdirectory(src)

add_executable(Mappeoppgave main.cpp
        include/trees.h
        include/turret.hpp
        include/CollisionDetection.hpp
        include/bears.hpp

        include/bullet.hpp
        include/GameObject.hpp
        include/Game.hpp)

target_link_libraries(Mappeoppgave PRIVATE MappeoppgaveLib)