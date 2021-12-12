# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME offscreen-rendering
           SOURCES offscreen-rendering.cpp
           LIBRARIES ICLMarkers)
