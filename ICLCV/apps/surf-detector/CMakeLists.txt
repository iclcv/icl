# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME surf-detector
           SOURCES surf-detector.cpp
           LIBRARIES ICLMarkers)
