# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME convert
          SOURCES convert.cpp
          LIBRARIES ICLMarkers)

