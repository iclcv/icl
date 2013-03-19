#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : doc/CMakeLists.txt                                     **
#** Module :                                                        **
#** Authors: Michael Goetting, Christof Elbrechter                  **
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

FIND_PACKAGE(Sphinx)
FIND_PACKAGE(Doxygen)

SET(REPLACE_SOURCE "doc/icl-manual/doc/icl-api/" CACHE INTERNAL "")
SET(REPLACE_DESTINATION "doc/icl-api/" CACHE INTERNAL "")

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/doc/icl-manual/js.rst.in
               "${CMAKE_SOURCE_DIR}/doc/icl-manual/js.rst"
	       @ONLY)

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/doc/icl-manual/conf.py.in
               "${CMAKE_BINARY_DIR}/doc/icl-manual/conf.py"
	       @ONLY)

FILE(COPY "${CMAKE_SOURCE_DIR}/doc/icl-manual/js.rst" DESTINATION "${CMAKE_BINARY_DIR}/doc/icl-manual/")
FILE(COPY "${CMAKE_SOURCE_DIR}/doc/icl-manual/logo.png" DESTINATION "${CMAKE_BINARY_DIR}/doc/icl-manual/")
FILE(COPY "${CMAKE_SOURCE_DIR}/doc/icl-manual/favicon.ico" DESTINATION "${CMAKE_BINARY_DIR}/doc/icl-manual/")
FILE(COPY "${CMAKE_SOURCE_DIR}/doc/icl-manual/exts" DESTINATION "${CMAKE_BINARY_DIR}/doc/icl-manual/")
FILE(COPY "${CMAKE_SOURCE_DIR}/doc/icl-manual/js" DESTINATION "${CMAKE_BINARY_DIR}/doc/icl-manual/html/")

IF(DOXYGEN_FOUND)
  ADD_SUBDIRECTORY(icl-api)
  MESSAGE(STATUS "adding target 'api'")
  FILE(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/doc/icl-api/html")
ELSEIF()
  ADD_CUSTOM_TARGET(api echo "api cannot be build due to missing dependencies")
ENDIF()

IF(SPHINX_FOUND)
  MESSAGE(STATUS "adding target 'manual'")
  ADD_CUSTOM_TARGET(manual ${SPHINXBUILD_EXECUTABLE} -b html -c "${CMAKE_BINARY_DIR}/doc/icl-manual" 
                    ${CMAKE_SOURCE_DIR}/doc/icl-manual "${CMAKE_BINARY_DIR}/doc/icl-manual/html")
ELSE()
  ADD_CUSTOM_TARGET(tutorial echo "tutorial cannot be build due to missing dependencies")
ENDIF()

IF(SPHINX_FOUND AND DOXYGEN_FOUND)
  MESSAGE(STATUS "adding target 'doc'")
  ADD_CUSTOM_TARGET(doc ${SPHINXBUILD_EXECUTABLE} -b html -c "${CMAKE_BINARY_DIR}/doc/icl-manual" 
                    ${CMAKE_SOURCE_DIR}/doc/icl-manual "${CMAKE_BINARY_DIR}/doc/icl-manual/html"  	
		    DEPENDS api)
ELSE()
  ADD_CUSTOM_TARGET(manual echo "manual cannot be build due to missing dependencies")
ENDIF()

INSTALL(DIRECTORY "${CMAKE_BINARY_DIR}//doc/icl-api/html" DESTINATION "share/icl-${SO_VERSION}/doc/icl-api")
INSTALL(DIRECTORY "${CMAKE_BINARY_DIR}/doc/icl-manual/html" DESTINATION "share/icl-${SO_VERSION}/doc/icl-manual")

# does this work?
SET_DIRECTORY_PROPERTIES(PROPERTIES ADDITIONAL_MAKE_CLEAN_FILES "exts;js;html") 
