# ImGui Integration Guide

## Overview
ImGui has been successfully integrated into your Mappeoppgave project. The integration provides a debug UI with two main panels:

1. **Game Panel** - Shows game state and controls
2. **Minimap Panel** - Visual top-down view of game entities

## What Was Done

### 1. CMake Configuration (`src/CMakeLists.txt`)
- Added ImGui source files to the build
- Included ImGui headers and backends
- Added GLFW include path from threepp's bundled GLFW

### 2. Created DebugUI Class
**Files**: `include/DebugUI.hpp`, `src/DebugUI.cpp`

The DebugUI class manages ImGui rendering and contains two panels:

#### Main Panel Features:
- Car hits counter (wolf melee damage)
- Blur level indicator
- Game state (initialized/ended)
- Restart button (alternative to R key)
- Max car speed slider (live tuning)
- Collapsible wolves status (alive/dead, bullet hits)

#### Minimap Features:
- 200x200 pixel minimap
- Shows world bounds [-2000, 2000]
- **Car** (cyan circle with direction arrow)
- **Wolves** (red circles, only alive ones)
- **Trees** (small green circles, sampled every 10th to reduce clutter)

### 3. Game Integration
**Modified**: `include/Game.hpp`, `src/Game.cpp`

- Added `debugUI_` member to Game class
- Made `DebugUI` a friend class to access private game state
- Initialize ImGui using `canvas_.windowPtr()` to get GLFWwindow*
- Call `debugUI_->newFrame()` before rendering
- Call `debugUI_->render()` after 3D rendering

## How It Works

### Render Loop Order:
```cpp
1. Update game logic (car, wolves, collisions)
2. debugUI_->newFrame()        // Start ImGui frame
3. renderer_.render(scene)      // Render 3D scene
4. blurOverlay_->render()       // Render blur effect
5. debugUI_->render()           // Render ImGui on top
```

### Window Handle:
The crucial fix was getting the GLFWwindow* from threepp's Canvas:
```cpp
static_cast<GLFWwindow*>(canvas_.windowPtr())
```

This allows ImGui backends to properly initialize with GLFW.

## Using the ImGui UI

### Controls:
- **Mouse**: Click and drag windows, interact with sliders/buttons
- **Keyboard**: Use keyboard navigation if enabled
- **R Key**: Still works for restart (plus UI button)

### Panels:
Both panels appear on screen startup:
- **Game Panel**: Top-left (10, 10)
- **Minimap Panel**: Right of Game panel (320, 10)

You can:
- Drag panels anywhere
- Collapse/expand sections
- Adjust car max speed in real-time
- Monitor wolf status during gameplay

## Future Enhancements

Possible additions:
1. **FPS counter** - Already have delta time available
2. **Performance graphs** - Plot frame times
3. **Entity counts** - Show active bears, mushrooms, bullets
4. **Camera controls** - Pan/zoom minimap
5. **Collision visualizer** - Show collision radii on minimap
6. **Wolf AI state** - Show target offsets, velocities
7. **Settings panel** - Adjust game parameters live
8. **Debug overlays** - Toggle wireframe, collision spheres in 3D view

## Technical Notes

### ImGui Version:
- Using ImGui v1.92.5 from official repository
- Docking branch features disabled (not in this version)
- GLFW + OpenGL3 backends

### Include Paths:
ImGui finds GLFW via threepp's bundled version:
```
cmake-build-debug-visual-studio/_deps/threepp-src/src/external/glfw/include
```

### Friend Class Pattern:
`DebugUI` is a friend of `Game` to access private members directly. This avoids creating many public getters for debug-only access.

### Memory Management:
- `debugUI_` is a `std::unique_ptr` - automatically cleaned up
- ImGui context is created in `DebugUI` constructor
- Properly shut down in destructor

## Troubleshooting

### If ImGui doesn't show:
1. Check that `canvas_.windowPtr()` returns non-null
2. Verify GLFW headers are found (should compile without errors)
3. Ensure `newFrame()` is called before `render()`
4. Check that rendering happens after 3D scene render

### If UI is unresponsive:
1. Make sure `ImGui_ImplGlfw_InitForOpenGL(window, true)` second param is `true`
2. This allows ImGui to install GLFW callbacks

### Performance:
- ImGui is very lightweight
- Minimap only draws car and wolves (minimal entities)
- No performance impact from UI rendering

## Files Modified

### New Files:
- `include/DebugUI.hpp`
- `src/DebugUI.cpp`

### Modified Files:
- `src/CMakeLists.txt` - Added ImGui sources and includes
- `CMakeLists.txt` - Updated executable target
- `include/Game.hpp` - Added DebugUI member and friend declaration
- `src/Game.cpp` - Initialize and render DebugUI
- `include/WorldManager.hpp` - Added getters for mushrooms/bears
- `src/WorldManager.cpp` - Implemented getters

All changes are backwards compatible - the game functions identically with or without the UI visible.

