# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME color-picker
          SOURCES color-picker.cpp
          LIBRARIES ICLMarkers)

