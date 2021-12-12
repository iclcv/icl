# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME image-compare
          SOURCES image-compare.cpp
          LIBRARIES ICLMarkers)
