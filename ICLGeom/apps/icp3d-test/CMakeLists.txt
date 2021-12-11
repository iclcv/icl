# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME icp3d-test
          SOURCES icp3d-test.cpp
          LIBRARIES ICLGeom ICLMath ICLUtils)
