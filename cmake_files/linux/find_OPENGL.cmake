icl_check_external_package(OPENGL "gl.h;glu.h" "GL;GLU" lib include/GL HAVE_OPENGL_COND TRUE)
icl_check_external_package(GLX "glx.h" "GL" lib include/GL HAVE_GLX_COND TRUE)
