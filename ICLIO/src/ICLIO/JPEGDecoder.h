/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/JPEGDecoder.h                          **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/File.h>
#include <ICLUtils/Exception.h>
#include <ICLCore/Types.h>

namespace icl{
  namespace io{
    /// Utility class for decoding JPEG-Data streams (with ICL_HAVE_LIBJPEG only)
    class ICLIO_API JPEGDecoder{
      public:
      /// Decode JPEG-File (E.g. used for FileGrabberPluginJPEG)
      /** @param file must be opened in mode readBinary or not opend 
          @param dst image, which is adapted to the found image parameters
      */
      static void decode(utils::File &file, core::ImgBase **dst) throw (utils::InvalidFileFormatException);
      
      /// Decode a data stream (E.g. used for Decoding Motion-JPEG streams in unicap's DefaultConvertEngine)
      /** @param data jpeg data stream (must be valid, otherwise unpredictable behaviour occurs
          @param maxDataLen length of the given data pointer
                            <b>Note:</b>This is just an upper limit to avoid segmentation faults on
                            corrupted jpeg data (e.g. end-of-image-marker is missing). The given data
                            pointer can be much longer then the actual jpeg data. If that is the case,
                            libjpeg obviously reads only necessary bytes.
          @param dst destination image, which is adapted to the found images parameters */
      static void decode(const unsigned char *data,unsigned int maxDataLen,core::ImgBase **dst);
      
      private:
      /// internal utility function, which does all the work
      static void decode_internal(utils::File *file,const unsigned char *data, 
                                  unsigned int maxDataLen, core::ImgBase **dst) throw (utils::InvalidFileFormatException);
    };
  } // namespace io
}

