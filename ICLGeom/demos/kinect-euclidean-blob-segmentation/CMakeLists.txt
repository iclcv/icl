# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME kinect-euclidean-blob-segmentation
           SOURCES kinect-euclidean-blob-segmentation.cpp
           LIBRARIES ICLGeom)
