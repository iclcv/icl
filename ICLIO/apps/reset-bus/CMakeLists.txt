# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME reset-bus
          SOURCES reset-bus.cpp
          LIBRARIES ICLMarkers)
