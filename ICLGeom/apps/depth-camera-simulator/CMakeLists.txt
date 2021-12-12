# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME depth-camera-simulator
          SOURCES depth-camera-simulator.cpp
          LIBRARIES ICLMarkers)
