#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/ICLHeplerMacros.cmake                    **
#** Module : ICLHeplerMacros                                        **
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

MACRO(BUILD_DEMO NAME)
  SET(BINARY "${NAME}-demo")
  LIST(APPEND DEMOS ${BINARY})
  ADD_EXECUTABLE(${BINARY} ${ARGN})

  INCLUDE_DIRECTORIES(${CMAKE_SOURCE_DIR}/ICLUtils/src
                      ${CMAKE_SOURCE_DIR}/ICLMath/src
		      ${CMAKE_SOURCE_DIR}/ICLCore/src
		      ${CMAKE_SOURCE_DIR}/ICLFilter/src
		      ${CMAKE_SOURCE_DIR}/ICLIO/src
		      ${CMAKE_SOURCE_DIR}/ICLQt/src
		      ${CMAKE_SOURCE_DIR}/ICLCV/src
		      ${CMAKE_SOURCE_DIR}/ICLGeom/src
		      ${CMAKE_SOURCE_DIR}/ICLMarkers/src)

  TARGET_LINK_LIBRARIES(${BINARY} ICLMarkers)
ENDMACRO()


MACRO(BUILD_APP NAME)
  SET(BINARY "icl-${NAME}")
  LIST(APPEND EXAMPLES ${BINARY})
  ADD_EXECUTABLE(${BINARY} ${ARGN})
  TARGET_LINK_LIBRARIES(${BINARY} ICLMarkers)
ENDMACRO()
