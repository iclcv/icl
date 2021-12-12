# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME physics-paper
           SOURCES physics-paper.cpp DefaultGroundObject.h DefaultPhysicsScene.h InteractiveFoldLine.h SceneMultiCamCapturer.cpp SceneMultiCamCapturer.h
           LIBRARIES ICLPhysics)
