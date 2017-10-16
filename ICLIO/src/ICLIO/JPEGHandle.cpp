/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/JPEGHandle.cpp                         **
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

#include <ICLIO/JPEGHandle.h>

namespace icl{
  namespace io{
  #ifdef ICL_HAVE_LIBJPEG
    void icl_jpeg_error_exit (j_common_ptr cinfo) {
      /* cinfo->err really points to a my_error_mgr struct, so coerce pointer */
      struct icl_jpeg_error_mgr* err = (struct icl_jpeg_error_mgr*) cinfo->err;

      /* Always display the message. */
      /* We could postpone this until after returning, if we chose. */
      (*cinfo->err->output_message) (cinfo);

      /* Return control to the setjmp point */
      longjmp(err->setjmp_buffer, 1);
    }
  #endif // ICL_HAVE_LIBJPEG
  } // namespace io
}
