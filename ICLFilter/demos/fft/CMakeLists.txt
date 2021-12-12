# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME fft
           SOURCES fft.cpp
           LIBRARIES ICLMarkers)
