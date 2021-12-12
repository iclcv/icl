# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME chromaticity-space
           SOURCES chromaticity-space.cpp
           LIBRARIES ICLMarkers)
