# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME point-cloud-define-world-frame
          SOURCES point-cloud-define-world-frame.cpp
          LIBRARIES ICLGeom)

