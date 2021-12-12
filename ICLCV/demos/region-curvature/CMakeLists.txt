# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME region-curvature
           SOURCES region-curvature.cpp
           LIBRARIES ICLMarkers)
