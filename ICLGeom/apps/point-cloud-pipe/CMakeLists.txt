# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME point-cloud-pipe
          SOURCES point-cloud-pipe.cpp
          LIBRARIES ICLMarkers)
