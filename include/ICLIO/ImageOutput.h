/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/ImageOutput.h                            **
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
#include <ICLIO/ImageCompressor.h>

namespace icl{
  namespace io{
    
    /// Minimal interface for image output classes
    /** The image output is used as generic interface for image sinks.
        Usually, it is recommended to use the GenericImageOutput class
        tha provides a string-based interface to set up the output
        backend. 
  
        \section CMP Compression 
        A few outputs do also support generic image compression, while
        other implementation provide output dependend compression parameters.
        E.g. shared memory or RSB-based network output streams use
        the inherited ImageCompressor to compress sent data. The file- our
        video image output of course use the used video/file formats compression
        mechanism.
    */
    struct ImageOutput : protected ImageCompressor{
      /// virtual destructor
      virtual ~ImageOutput() {}
  
      /// ImageOutput instances must implement this method
      virtual void send(const core::ImgBase *image) = 0;
  
      /// provide the protectedly inherited image compressor options here
      using ImageCompressor::getCompression;
  
      /// provide the protectedly inherited image compressor options here
      using ImageCompressor::setCompression;
    };
  } // namespace io
}

