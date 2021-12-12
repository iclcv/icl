# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME point-cloud-primitive-filter
          SOURCES point-cloud-primitive-filter.cpp
          LIBRARIES ICLMarkers)
