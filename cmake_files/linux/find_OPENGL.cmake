icl_check_external_package(OPENGL "gl.h;glu.h" "GL;GLU" lib include/GL /usr HAVE_OPENGL_COND TRUE)
icl_check_external_package(GLX "glx.h" "GL" lib include/GL /usr HAVE_GLX_COND TRUE)

if(${HAVE_OPENGL_COND})
  set(OPENGL_LIBS_l GL GLU)
endif()
