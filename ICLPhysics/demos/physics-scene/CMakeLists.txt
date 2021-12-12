# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME physics-scene
           SOURCES physics-scene.cpp
           LIBRARIES ICLPhysics)
