# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME physics-test
           SOURCES physics-test.cpp
           LIBRARIES ICLPhysics)

