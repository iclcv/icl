# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME kinect-normals
           SOURCES kinect-normals.cpp
           LIBRARIES ICLMarkers)
