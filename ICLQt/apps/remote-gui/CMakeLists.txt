# ---- Include ICL macros first ----
INCLUDE(ICLHelperMacros)

# ---- Examples ----
BUILD_APP(NAME remote-gui
          SOURCES remote-gui.cpp
          LIBRARIES ICLQt)
