# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME physics-paper3
           SOURCES physics-paper3 DefaultPhysicsScene.h DefaultGroundObject.h
           LIBRARIES ICLPhysics)

