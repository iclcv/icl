# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_DEMO(NAME corner-detection-css
           SOURCES corner-detection-css.cpp
           LIBRARIES ICLMarkers)
