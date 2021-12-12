# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME rectify-image
          SOURCES rectify-image.cpp
          LIBRARIES ICLMarkers)
