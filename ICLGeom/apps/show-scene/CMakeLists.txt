# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME show-scene
          SOURCES show-scene.cpp
          LIBRARIES ICLMarkers)
