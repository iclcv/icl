# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME heart-rate-detector
          SOURCES heart-rate-detector.cpp
          LIBRARIES ICLMarkers)
