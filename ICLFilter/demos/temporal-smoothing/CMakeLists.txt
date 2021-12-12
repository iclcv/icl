# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME temporal-smoothing
           SOURCES temporal-smoothing.cpp
           LIBRARIES ICLMarkers)
