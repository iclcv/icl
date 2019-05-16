#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/FindSphinx.cmake                         **
#** Module : FindSphinx                                             **
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

MACRO(FIND_PROGRAM_MANUAL NAME VAR)
  FIND_PROGRAM(${VAR}_EXECUTABLE ${NAME})
  IF(${${VAR}_EXECUTABLE} STREQUAL "${${VAR}_EXECUTABLE}-NOTFOUND")
    MESSAGE(STATUS "Not Found ${NAME} executable")
    SET(${VAR}_FOUND FALSE)
  ELSE()
    MESSAGE(STATUS "Found ${NAME}: ${${VAR}_EXECUTABLE}")
    SET(${VAR}_FOUND TRUE)
  ENDIF()
ENDMACRO()

FIND_PROGRAM_MANUAL(sphinx-build SPHINXBUILD)

find_package (Python3 COMPONENTS Interpreter Development)

IF(Python3_Interpreter_FOUND AND SPHINXBUILD_FOUND)
  MESSAGE(STATUS "Found python: ${Python3_EXECUTABLE}")
  FILE(WRITE ${CMAKE_SOURCE_DIR}/manual/pyparsing_check.py
       "import pyparsing")
  EXECUTE_PROCESS(COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/manual/pyparsing_check.py RESULT_VARIABLE PYPARSING_TEST_OUT OUTPUT_QUIET ERROR_QUIET)
  FILE(REMOVE ${CMAKE_SOURCE_DIR}/manual/pyparsing_check.py)

  IF(NOT PYPARSING_TEST_OUT)
    MESSAGE(STATUS "Found python module: pyparsing")
    SET(SPHINX_FOUND TRUE)
  ELSE()
    MESSAGE(STATUS "Not Found python module: pyparsing")
    SET(SPHINX_FOUND FALSE)
  ENDIF()
ENDIF()
