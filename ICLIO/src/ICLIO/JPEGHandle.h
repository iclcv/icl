// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#pragma once

#ifdef ICL_HAVE_LIBJPEG

#include <ICLUtils/CompatMacros.h>
#include <stdio.h>
#include <jerror.h>
#include <jpeglib.h>
#include <setjmp.h>


/** \cond  this is not commented, because this are only support structs and functions */
namespace icl{
  namespace io{

    // returns controll to the caller
    struct icl_jpeg_error_mgr : jpeg_error_mgr {
      jmp_buf setjmp_buffer;
    };

    // passes controll back to the caller
    ICLIO_API void icl_jpeg_error_exit(j_common_ptr cinfo);



    /// Handles JPEG info and error manager
    struct ICLIO_API JPEGDataHandle{
      inline JPEGDataHandle(){
        info.err = jpeg_std_error(&em);
        em.error_exit = icl_jpeg_error_exit;
      }
      struct jpeg_decompress_struct info;
      struct icl_jpeg_error_mgr     em;
    };

  } // namespace io
}// namespace icl

/** \endcond */

#else // not ICL_HAVE_LIBJPEG
namespace icl{
  namespace io{
    /** \cond */
    struct JPEGDataHandle{};
    /** \endcond */
  } // namespace io
}
#endif // not ICL_HAVE_LIBJPEG
