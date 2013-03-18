# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME undistortion
           SOURCES undistortion.cpp
           LIBRARIES ICLMarkers)

