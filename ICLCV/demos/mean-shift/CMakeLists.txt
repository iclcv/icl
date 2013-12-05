# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME mean-shift
           SOURCES mean-shift.cpp
           LIBRARIES ICLMarkers)

