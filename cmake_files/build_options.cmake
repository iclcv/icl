# check system bits (32 or 64)
if(CMAKE_SIZEOF_VOID_P MATCHES "8")
    set(ICL_64BIT TRUE)
    add_definitions( -DICL_64BIT)
    message("64bit system detected")
    if(UNIX)
      set(ICL_REL_LIB_DIR_SUFFIX "x86_64-linux-gnu")
    endif()
else()
    add_definitions( -DICL_32BIT)
    message("32bit system detected")
    if(UNIX)
      set(ICL_REL_LIB_DIR_SUFFIX "i386-linux-gnu")
    endif()
endif()


# set compiler options
if(ICL_VAR_DEBUG_MODE)
  if(ICL_VAR_ARCHITECTURE_DEPENDENT_BUILD_ON)
    set(CMAKE_CXX_FLAGS "-Wall -O0 -g3 -march=native")
  else()
    set(CMAKE_CXX_FLAGS "-Wall -O0 -g3")
  endif()
  set(CMAKE_BUILD_TYPE "Debug" CACHE INTERNAL "")
  set(DEBUG_POSTFIX "-debug")
  set(PC_FILE_NAME "icl-${VERSION}-debug.pc")
else()
  if(ICL_VAR_ARCHITECTURE_DEPENDENT_BUILD_ON)
    set(CMAKE_CXX_FLAGS "-O3 -march=native")
  else()
    set(CMAKE_CXX_FLAGS "-O3")
  endif()
  set(CMAKE_BUILD_TYPE "Release" CACHE INTERNAL "")
  set(DEBUG_POSTFIX "")
  set(PC_FILE_NAME "icl-${VERSION}.pc")
endif()

if(ICL_VAR_USE_OMP)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fopenmp")
  add_definitions( -DUSE_OPENMP)
endif()

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")

#where to install ICL
set(ICL_VAR_INSTALL_PREFIX "/vol/nivision" CACHE PATH "")
set(CMAKE_INSTALL_PREFIX "${ICL_VAR_INSTALL_PREFIX}" CACHE INTERNAL "")
#set(CMAKE_ICL_INSTALL_FOLDER_NAME "abdce" CACHE INTERNAL "")
set(CMAKE_CXX_FLAGS "-pthread ${CMAKE_CXX_FLAGS}")

message(STATUS "Install Path: ${CMAKE_INSTALL_PREFIX}")


# set system indicators and add makefile conditionals
if(APPLE)
  add_definitions( -DICL_SYSTEM_APPLE)
  set(SYSTEM_APPLE_COND TRUE)
elseif(UNIX)
  add_definitions( -DICL_SYSTEM_LINUX)
  set(SYSTEM_LINUX_COND TRUE)
endif()



#runtimepath options
# use, i.e. don't skip the full RPATH for the build tree
SET(CMAKE_SKIP_BUILD_RPATH  FALSE)

# when building, don't use the install RPATH already
# (but later on when installing)
SET(CMAKE_BUILD_WITH_INSTALL_RPATH FALSE) 

# the RPATH to be used when installing
SET(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib/icl-${VERSION}${DEBUG_POSTFIX}")

# add the automatically determined parts of the RPATH
# which point to directories outside the build tree to the install RPATH
SET(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
