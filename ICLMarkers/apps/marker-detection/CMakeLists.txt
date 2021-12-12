# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME marker-detection
          SOURCES marker-detection.cpp
          LIBRARIES ICLMarkers)
