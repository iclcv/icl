# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME octree
           SOURCES octree.cpp
           LIBRARIES ICLMarkers)
