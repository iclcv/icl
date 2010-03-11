/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLIO module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

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
