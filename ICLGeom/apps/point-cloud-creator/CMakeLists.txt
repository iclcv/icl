# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME point-cloud-creator
          SOURCES point-cloud-creator.cpp
          LIBRARIES ICLGeom ICLIO)
