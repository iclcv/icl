#*********************************************************************
#**                Image Component Library (ICL)                    **
#**                                                                 **
#** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
#**                         Neuroinformatics Group                  **
#** Website: www.iclcv.org and                                      **
#**          http://opensource.cit-ec.de/projects/icl               **
#**                                                                 **
#** File   : cmake/Modules/CurrentDate.cmake                        **
#** Module : CurrentDate                                            **
#** Authors: Johannes Wienke                                        **
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

# - A macro returning the current date.

FUNCTION(CURRENT_DATE RESULT)

    IF (WIN32)
        SET(ARGS "/T")
        SET(REPLACE "(..)/(..)/..(..).*" "\\3\\2\\1")
    ELSEIF(UNIX)
        SET(ARGS "+%d/%m/%Y")
        SET(REPLACE "(..)/(..)/..(..).*" "\\3\\2\\1")
    ELSE()
        MESSAGE(WARNING "date not implemented")
        SET(${RESULT} 000000 PARENT_SCOPE)
        RETURN()
    ENDIF()

    EXECUTE_PROCESS(COMMAND "date" ${ARGS}
                    OUTPUT_VARIABLE DATE_OUTPUT
                    RESULT_VARIABLE DATE_RESULT)

    IF(NOT DATE_RESULT EQUAL 0)
        MESSAGE(WARNING "error calling date command")
        SET(${RESULT} 000000 PARENT_SCOPE)
        RETURN()
    ENDIF()

    STRING(REGEX REPLACE ${REPLACE} DATE_STRING ${DATE_OUTPUT})
    SET(${RESULT} ${DATE_STRING} PARENT_SCOPE)

ENDFUNCTION()
