#*********************************************************************
#**                                                                 **
#** File   : src/CMakeLists.txt                                     **
#** Authors: Viktor Richter                                         **
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
#*********************************************************************

# get all header files
SET(HEADERS HelloWorld.h
#           AnotherHeader.h
   )

# set all sources
SET(SOURCES HelloWorld.cpp
#           AnotherSource.cpp
   )

# add headers
include_directories("${PROJECT_SOURCE_DIR}/src/")

# add icl headers
include_directories(${ICL_INCLUDE_DIRS})

# add icl defines
add_definitions(${ICL_DEFINITIONS})

#create library
add_library(${PROJECT_NAME} SHARED ${SOURCES})

target_link_libraries(${PROJECT_NAME}
                      ${ICL_LIBRARIES}
)

# set lib version
#SET_TARGET_PROPERTIES(${PROJECT_NAME}
#                      PROPERTIES 
#                      VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}")

# install library
INSTALL(TARGETS ${PROJECT_NAME}
        RUNTIME DESTINATION bin
        LIBRARY DESTINATION lib
        ARCHIVE DESTINATION lib
        PERMISSIONS OWNER_READ OWNER_WRITE GROUP_READ GROUP_WRITE
        )
