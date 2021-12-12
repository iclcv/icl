# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME point-cloud-tests
          SOURCES point-cloud-tests.cpp
          LIBRARIES ICLMarkers)
