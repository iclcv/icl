# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME physics-paper3
           SOURCES physics-paper3 SceneMultiCamCapturer.h SceneMultiCamCapturer.cpp PhysicsPaper3MouseHandler.h PhysicsPaper3MouseHandler.cpp Menu.h Menu.cpp DefaultPhysicsScene.h DefaultGroundObject.h
           LIBRARIES ICLPhysics)

