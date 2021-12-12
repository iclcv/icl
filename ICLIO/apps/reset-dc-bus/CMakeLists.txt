# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME reset-dc-bus
          SOURCES reset-dc-bus.cpp
          LIBRARIES ICLMarkers)
