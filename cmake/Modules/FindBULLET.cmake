#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindGLEW.cmake                           **
#** Module : FindGLEW                                               **
#** Authors: Michael Goetting                                       **
#**                                                                 **
#**                                                                 **
#** GNU LESSER GENERAL PUBLIC LICENSE                               **
#** This file may be used under the terms of the GNU Lesser General **
#** Public License version 3.0 as published by the                  **
#**                                                                 **
#** Free Software Foundation and appearing in the file LICENSE.LGPL **
#** included in the packaging of this file.  Please review the      **
#** following information to ensure the license requirements will   **
#** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
#**                                                                 **
#** The development of this software was supported by the           **
#** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
#** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
#** Forschungsgemeinschaft (DFG) in the context of the German       **
#** Excellence Initiative.                                          **
#**                                                                 **
#*********************************************************************

INCLUDE(FindPackageHandleStandardArgs)

# ---------------------------------------------------------------------
# Start main part here
# ---------------------------------------------------------------------

IF(WIN32)
  # Ask the root directory of bullet
  SET( BULLET_ROOT BULLET_ROOT CACHE PATH "Root directory of bullet")
ENDIF()

# Search BULLET_ROOT first if it is set.
IF(BULLET_ROOT)
  SET(_BULLET_SEARCH_ROOT PATHS ${BULLET_ROOT} NO_DEFAULT_PATH)
  LIST(APPEND _BULLET_SEARCHES ${_BULLET_SEARCH_ROOT})
ENDIF()

IF(UNIX)
  # Normal search.
  SET(_BULLET_SEARCH_NORMAL
       PATHS "/usr" "/usr/local"
     )
     
  LIST(APPEND _BULLET_SEARCHES ${_BULLET_SEARCH_NORMAL})
ENDIF()

# find old style libraries
FOREACH(D ${_BULLET_SEARCHES})
  IF(EXISTS "${D}/lib/cmake/bullet/BulletConfig.cmake")
    INCLUDE("${D}/lib/cmake/bullet/BulletConfig.cmake")
    SET(BULLET_TEMP_LIBRARIES "")
	IF(WIN32)
		FOREACH(L LinearMath BulletCollision BulletDynamics BulletSoftBody)
			IF(EXISTS "${BULLET_LIBRARY_DIRS}/${L}.lib")
				LIST(APPEND BULLET_TEMP_LIBRARIES "${BULLET_LIBRARY_DIRS}/${L}.lib")
			ENDIF()
			IF(EXISTS "${BULLET_LIBRARY_DIRS}/${L}_Debug.lib")
				LIST(APPEND BULLET_TEMP_LIBRARIES "${BULLET_LIBRARY_DIRS}/${L}_Debug.lib")
			ENDIF()
		ENDFOREACH()
	ELSE(WIN32)
		FOREACH(L LinearMath BulletCollision BulletDynamics BulletSoftBody)
			IF(EXISTS "${BULLET_LIBRARY_DIRS}/lib${L}.lib")
				LIST(APPEND BULLET_TEMP_LIBRARIES "${BULLET_LIBRARY_DIRS}/lib${L}.so")
			ENDIF()
		ENDFOREACH()
	ENDIF(WIN32)
    SET(BULLET_LIBRARIES ${BULLET_TEMP_LIBRARIES})
  ENDIF()
ENDFOREACH()


FIND_PACKAGE_HANDLE_STANDARD_ARGS(BULLET REQUIRED_VARS
                                  BULLET_LIBRARIES
                                  BULLET_INCLUDE_DIRS)
