/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/JPEGEncoder.h                            **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#pragma once

#include <ICLCore/ImgBase.h>
#include <ICLUtils/Uncopyable.h>

#ifndef HAVE_LIBJPEG
#warning "libjpeg is not available, therefore, this header should not be included"
#endif

namespace icl{
  namespace io{
    /// encoding class for data-to-data jpeg compression
    class JPEGEncoder : public utils::Uncopyable{
      struct Data;  //!< pimpl type
      Data *m_data; //!< pimpl pointer
  
      public:
      /// constructor with given jpeg quality
      /** The quality value is always given in percet (1-100)*/
      JPEGEncoder(int quality=90);
      
      /// Destructor
      ~JPEGEncoder();
  
      /// sets the compression quality level
      void setQuality(int quality);
      
      /// encoded data type
      struct EncodedData{
        icl8u *bytes; //!< byte pointer
        int len;      //!< number of bytes used
      };
      
      /// encodes a given core::ImgBase * (only depth8u is supported natively)
      /** non-depth8u images are automatically converted before compression.
          This might lead to loss of data*/
      const EncodedData &encode(const core::ImgBase *image);    
      
      /// first encodes the jpeg in memory and then write the whole memory chunk to disc
      void writeToFile(const core::ImgBase *image, const std::string &filename);
    };
    
  } // namespace io
}
