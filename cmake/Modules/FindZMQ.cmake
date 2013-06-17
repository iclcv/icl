#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindZMQ.cmake                            **
#** Module : FindZMQ                                                **
#** Authors: Christof Elbrechter                                    **
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

IF(NOT ZMQ_ROOT)
  SET(ZMQ_ROOT "/usr")
ENDIF()

FIND_PATH(ZMQ_INCLUDE_DIR
  NAMES zmq.hpp
  PATHS ${ZMQ_ROOT}
  PATH_SUFFIXES include
  DOC "The path to ZMQ C++-header files")
#  NO_DEFAULT_PATH)

FIND_LIBRARY(ZMQ_LIBRARY  
  NAMES zmq
  PATHS ${ZMQ_ROOT}
  PATH_SUFFIXES lib)
#  NO_DEFAULT_PATH)

FIND_PACKAGE_HANDLE_STANDARD_ARGS(ZMQ REQUIRED_VARS 
				  ZMQ_LIBRARY
				  ZMQ_INCLUDE_DIR)

IF(ZMQ_FOUND)
  SET(ZMQ_INCLUDE_DIRS ${ZMQ_INCLUDE_DIR})
  SET(ZMQ_LIBRARIES ${ZMQ_LIBRARY})
ENDIF()

MARK_AS_ADVANCED(ZMQ_INCLUDE_DIR)
