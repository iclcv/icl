# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME gui-assignment-info
          SOURCES gui-assignment-info.cpp
          LIBRARIES ICLMarkers)
