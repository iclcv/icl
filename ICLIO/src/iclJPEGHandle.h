#ifndef ICL_JPEG_HANDLE_H
#define ICL_JPEG_HANDLE_H

#ifdef HAVE_LIBJPEG

#include <stdio.h>
#include <jerror.h>
#include <jpeglib.h>
#include <setjmp.h>


/** \cond  this is not commented, because this are only support structs and functions */ 
namespace icl{

  // returns controll to the caller
  struct icl_jpeg_error_mgr : jpeg_error_mgr {
    jmp_buf setjmp_buffer; 
  };
  
  // passes controll back to the caller
  void icl_jpeg_error_exit (j_common_ptr cinfo);
  
  // }}}

  
  /// Handles JPEG info and error manager
  struct JPEGDataHandle{
    inline JPEGDataHandle(){
      info.err = jpeg_std_error(&em);
      em.error_exit = icl_jpeg_error_exit;
    }
    struct jpeg_decompress_struct info;
    struct icl_jpeg_error_mgr     em;
  };

}// namespace icl

/** \endcond */

#else // not HAVE_LIBJPEG
namespace icl{
  /** \cond */
  struct JPEGDataHandle{};
  /** \endcond */
}
#endif // not HAVE_LIBJPEG
#endif // GUARDIAN
