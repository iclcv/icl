# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME create-marker-grid-svg
          SOURCES create-marker-grid-svg.cpp
          LIBRARIES ICLMarkers)

