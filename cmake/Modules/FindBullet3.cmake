# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.
# 
# REMARK: This version has been adapted for finding Bullet3 (Bullet >= 2.82)
#
#.rst:
# FindBullet
# ----------
#
# Try to find the Bullet physics engine
#
#
#
# ::
#
#   This module defines the following variables
#
#
#
# ::
#
#   BULLET_FOUND - Was bullet found
#   BULLET_INCLUDE_DIRS - the Bullet include directories
#   BULLET_LIBRARIES - Link to this, by default it includes
#                      all bullet components (Dynamics,
#                      Collision, LinearMath, & SoftBody)
#
#
#
# ::
#
#   This module accepts the following variables
#
#
#
# ::
#
#   BULLET_ROOT - Can be set to bullet install path or Windows build path

INCLUDE(FindPackageHandleStandardArgs)

macro(_FIND_BULLET_LIBRARY _var)
  find_library(${_var}
     NAMES
        ${ARGN}
     HINTS
        ${BULLET_ROOT}
        ${BULLET_ROOT}/lib/Release
        ${BULLET_ROOT}/lib/Debug
        ${BULLET_ROOT}/out/release8/libs
        ${BULLET_ROOT}/out/debug8/libs
     PATH_SUFFIXES lib
     NO_DEFAULT_PATH
  )
  find_library(${_var}
     NAMES ${ARGN}
     PATH_SUFFIXES lib
  )
  mark_as_advanced(${_var})
endmacro()

macro(_BULLET_APPEND_LIBRARIES _list _release)
   set(_debug ${_release}_DEBUG)
   if(${_debug})
      set(${_list} ${${_list}} optimized ${${_release}} debug ${${_debug}})
   else()
      set(${_list} ${${_list}} ${${_release}})
   endif()
endmacro()

# This file is NOT available in Bullet 2.81 and below
# first check without default path to prevent system wide versions to be found
find_path(BULLET_INCLUDE_DIR NAMES BulletDynamics/ConstraintSolver/btNNCGConstraintSolver.h
  HINTS
    ${BULLET_ROOT}/include
    ${BULLET_ROOT}/src
    NO_DEFAULT_PATH
    PATH_SUFFIXES bullet
)

# as a fallback also include default path now; note that bullet wont be reset if it has been found already
find_path(BULLET_INCLUDE_DIR NAMES BulletDynamics/ConstraintSolver/btNNCGConstraintSolver.h
	PATH_SUFFIX bullet
)

# Find the libraries
_FIND_BULLET_LIBRARY(BULLET_DYNAMICS_LIBRARY        BulletDynamics)
_FIND_BULLET_LIBRARY(BULLET_DYNAMICS_LIBRARY_DEBUG  BulletDynamics_Debug BulletDynamics_d)
_FIND_BULLET_LIBRARY(BULLET_COLLISION_LIBRARY       BulletCollision)
_FIND_BULLET_LIBRARY(BULLET_COLLISION_LIBRARY_DEBUG BulletCollision_Debug BulletCollision_d)
_FIND_BULLET_LIBRARY(BULLET_MATH_LIBRARY            BulletMath LinearMath)
_FIND_BULLET_LIBRARY(BULLET_MATH_LIBRARY_DEBUG      BulletMath_Debug BulletMath_d LinearMath_Debug LinearMath_d)
_FIND_BULLET_LIBRARY(BULLET_SOFTBODY_LIBRARY        BulletSoftBody)
_FIND_BULLET_LIBRARY(BULLET_SOFTBODY_LIBRARY_DEBUG  BulletSoftBody_Debug BulletSoftBody_d)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(Bullet3 DEFAULT_MSG
    BULLET_DYNAMICS_LIBRARY BULLET_COLLISION_LIBRARY BULLET_MATH_LIBRARY
    BULLET_SOFTBODY_LIBRARY BULLET_INCLUDE_DIR)

if(BULLET3_FOUND)
   set(BULLET_INCLUDE_DIRS ${BULLET_INCLUDE_DIR})
   _BULLET_APPEND_LIBRARIES(BULLET_LIBRARIES BULLET_DYNAMICS_LIBRARY)
   _BULLET_APPEND_LIBRARIES(BULLET_LIBRARIES BULLET_COLLISION_LIBRARY)
   _BULLET_APPEND_LIBRARIES(BULLET_LIBRARIES BULLET_MATH_LIBRARY)
   _BULLET_APPEND_LIBRARIES(BULLET_LIBRARIES BULLET_SOFTBODY_LIBRARY)
endif()
