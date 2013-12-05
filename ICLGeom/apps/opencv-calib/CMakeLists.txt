# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME opencv-calib
          SOURCES opencv-calib.cpp
          LIBRARIES ICLMarkers)

