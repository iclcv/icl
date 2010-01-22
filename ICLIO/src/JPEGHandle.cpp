#include <ICLIO/JPEGHandle.h>

namespace icl{
#ifdef HAVE_LIBJPEG
  void icl_jpeg_error_exit (j_common_ptr cinfo) {
    /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
    struct icl_jpeg_error_mgr* err = (struct icl_jpeg_error_mgr*) cinfo->err;
    
    /* Always display the message. */
    /* We could postpone this until after returning, if we chose. */
    (*cinfo->err->output_message) (cinfo);
    
    /* Return control to the setjmp point */
    longjmp(err->setjmp_buffer, 1);
  }
#endif // HAVE_LIBJPEG
}
