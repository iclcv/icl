# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME lens-undistortion-calibration
          SOURCES lens-undistortion-calibration.cpp
                  UndistortionUtil.cpp
          LIBRARIES ICLMarkers)

        # the UndistortionUtil.h header?
