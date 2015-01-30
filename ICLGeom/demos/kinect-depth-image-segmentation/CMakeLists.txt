# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME kinect-depth-image-segmentation
           SOURCES kinect-depth-image-segmentation.cpp
           LIBRARIES ICLGeom)

