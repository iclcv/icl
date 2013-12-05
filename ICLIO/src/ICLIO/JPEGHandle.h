/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/JPEGHandle.h                           **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#pragma once

#ifdef HAVE_LIBJPEG

#include <stdio.h>
#include <jerror.h>
#include <jpeglib.h>
#include <setjmp.h>
#include <ICLUtils/CompatMacros.h>


/** \cond  this is not commented, because this are only support structs and functions */ 
namespace icl{
  namespace io{
  
    // returns controll to the caller
    struct ICL_IO_API icl_jpeg_error_mgr : jpeg_error_mgr {
      jmp_buf setjmp_buffer; 
    };
    
    // passes controll back to the caller
    ICL_IO_API void icl_jpeg_error_exit(j_common_ptr cinfo);
    
    // }}}
  
    
    /// Handles JPEG info and error manager
    struct ICL_IO_API JPEGDataHandle{
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

#else // not HAVE_LIBJPEG
namespace icl{
  namespace io{
    /** \cond */
    struct JPEGDataHandle{};
    /** \endcond */
  } // namespace io
}
#endif // not HAVE_LIBJPEG
