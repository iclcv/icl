# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME show-extrinsic-calibration-grid
          SOURCES show-extrinsic-calibration-grid.cpp
          LIBRARIES ICLMarkers)

