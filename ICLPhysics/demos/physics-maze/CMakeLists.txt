# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME physics-maze
           SOURCES physics-maze.cpp MazeObject.h MazeObject.cpp HoleObject.h
           LIBRARIES ICLPhysics)