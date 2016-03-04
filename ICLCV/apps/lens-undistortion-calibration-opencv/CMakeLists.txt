# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME lens-undistortion-calibration-opencv
          SOURCES lens-undistortion-calibration-opencv.cpp
          LIBRARIES ICLMarkers)

