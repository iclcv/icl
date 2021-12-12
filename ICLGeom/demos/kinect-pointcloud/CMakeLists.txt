# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME kinect-pointcloud
           SOURCES kinect-pointcloud.cpp
           LIBRARIES ICLMarkers)
