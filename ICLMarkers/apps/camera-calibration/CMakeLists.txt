# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME camera-calibration
          SOURCES camera-calibration.cpp CameraCalibrationUtils.cpp
          LIBRARIES ICLMarkers)

