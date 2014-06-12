# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME lens-undistortion-calibration
          SOURCES lens-undistortion-calibration.cpp
          LIBRARIES ICLMarkers)

