# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME ray-cast-octree
           SOURCES ray-cast-octree.cpp
           LIBRARIES ICLMarkers)

