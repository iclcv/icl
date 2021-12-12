# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME phyisics-car
           SOURCES phyisics-car.cpp
           LIBRARIES ICLPhysics)
