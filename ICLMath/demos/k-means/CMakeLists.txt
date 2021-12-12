# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME k-means
           SOURCES k-means.cpp
           LIBRARIES ICLMarkers)
