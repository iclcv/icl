# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME region-detection
           SOURCES region-detection.cpp
           LIBRARIES ICLMarkers)
