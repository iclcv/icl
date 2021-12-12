# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME fix-kinect-calibration
          SOURCES fix-kinect-calibration.cpp
          LIBRARIES ICLMarkers)
