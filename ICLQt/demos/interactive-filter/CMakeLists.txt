# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME interactive-filter
           SOURCES interactive-filter.cpp
           LIBRARIES ICLMarkers)

