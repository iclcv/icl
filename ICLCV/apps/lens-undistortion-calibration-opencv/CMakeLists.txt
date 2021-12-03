# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
IF(OpenCV_VERSION VERSION_LESS "4.0")
  BUILD_APP(NAME lens-undistortion-calibration-opencv
            SOURCES lens-undistortion-calibration-opencv.cpp
            LIBRARIES ICLMarkers)
ENDIF()
