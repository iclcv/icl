# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME orb-feature-detection
           SOURCES orb-feature-detection.cpp
           LIBRARIES ICLMarkers)
