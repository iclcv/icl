# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME kinect-segmentation
           SOURCES kinect-segmentation.cpp
           LIBRARIES ICLMarkers)
