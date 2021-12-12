# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME color-segmentation
          SOURCES color-segmentation.cpp
          LIBRARIES ICLMarkers)
