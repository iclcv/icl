# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME rgbd-mapping
           SOURCES rgbd-mapping.cpp
           LIBRARIES ICLMarkers)
