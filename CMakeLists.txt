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

# Assimp
FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG cc6ab1408fa0dc7b386f5b8e5883c3cedcae950b
)
FetchContent_MakeAvailable(assimp)

# ImGui
FetchContent_Declare(
        imgui
        GIT_REPOSITORY https://github.com/ocornut/imgui.git
        GIT_TAG v1.92.5
)
FetchContent_MakeAvailable(imgui)

# Catch2
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.11.0
)
FetchContent_MakeAvailable(Catch2)

# Make imgui lib for simplicity
add_library(ImGuiLib STATIC
        ${imgui_SOURCE_DIR}/imgui.cpp
        ${imgui_SOURCE_DIR}/imgui_draw.cpp
        ${imgui_SOURCE_DIR}/imgui_tables.cpp
        ${imgui_SOURCE_DIR}/imgui_widgets.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
        ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

# Tell imgui where the headers are
target_include_directories(ImGuiLib PUBLIC
        ${imgui_SOURCE_DIR}
        ${imgui_SOURCE_DIR}/backends
        ${threepp_SOURCE_DIR}/src/external/glfw/include
)


add_subdirectory(src)


add_executable(Mappeoppgave main.cpp)

# Link the library thats made in src/ (MappeoppgaveLib)
target_link_libraries(Mappeoppgave PRIVATE MappeoppgaveLib)

# Build tests
add_executable(Tests
        test_core.cpp
        test_collision.cpp
        test_entities.cpp
)


target_link_libraries(Tests PRIVATE MappeoppgaveLib Catch2::Catch2WithMain)

# Activate Ctest
include(CTest)
add_test(NAME unit_tests COMMAND Tests)


add_custom_command(TARGET Mappeoppgave POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/Models
        $<TARGET_FILE_DIR:Mappeoppgave>/Models
        COMMENT "Copies the Models-folder to the output-folder..."
)