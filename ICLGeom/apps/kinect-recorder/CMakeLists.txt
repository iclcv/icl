# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME kinect-recorder
          SOURCES kinect-recorder.cpp
          LIBRARIES ICLMarkers)

