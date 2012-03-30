icl_check_external_package(OPENGL "gl.h;glu.h" "libGL.dylib;libGLU.dylib" Libraries Headers /Developer/SDKs/MacOSX10.6.sdk/System/Library/Frameworks/OpenGL.framework HAVE_OPENGL_COND TRUE)

if(${HAVE_OPENGL_COND})
  #special linker flags on OS X
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -F/Developer/SDKs/MacOSX10.6.sdk/System/Library/Frameworks/ -framework OpenGL")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS}")
endif()
