# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME camera-calibration-planar
          SOURCES camera-calibration-planar.cpp
          LIBRARIES ICLMarkers)

