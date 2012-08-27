/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/GenericImageOutput.h                     **
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

#ifndef ICL_GENERIC_IMAGE_OUTPUT_H
#define ICL_GENERIC_IMAGE_OUTPUT_H

#include <ICLCore/ImgBase.h>
#include <ICLUtils/ProgArg.h>
#include <ICLIO/ImageOutput.h>
#include <ICLUtils/SmartPtr.h>

namespace icl{
  namespace io{
    
    /// Generic Sink for images
    /** Like the GenericGrabber, the GenericImageOutput provides a string-configurable
        interface for arbitrary image sinks. 
        
        \section BACK Supported Backends
        
        Supported Backends are:
          - "file" (description=filepattern)
          - "video" (description=output-video-filename,CODEC-FOURCCC=DIV3,VideoSize=VGA,FPS=24)
          - "sm" (SharedMemory output, description=memory-segment-ID)
          - "xcfp" (XCF Publisher output, description=stream-name)
          - "rsb" (Robotics Service Bus Output), description=[comma-sep. transport-list=spread]:scope)
  
        \section META Image Meta Data
        
        Only a few backends do actually support sending also image meta data. So far,
        this is only supported by the RSB and by the shared memory backend, however, 
        we plan to add this feature at least for the .icl-file format. The corresponding
        GenericGrabber backends for these types are also able to deserialize the images meta data
    */
    
    class GenericImageOutput : public ImageOutput{
      std::string type;
      std::string description;
      SmartPtr<ImageOutput> impl;
  
      public:
      
      /// Null constructor
      GenericImageOutput(){}
      
      /// Create and initialize
      /** @see init */
      GenericImageOutput(const std::string &type, const std::string &description);
      
      /// Create from given program argument
      GenericImageOutput(const ProgArg &pa);
      
      /// initialize this instance
      void init(const std::string &type, const std::string &description);
  
      /// initialization method (from given progarg)
      void init(const ProgArg &pa);
  
      
      /// sends a new image
      virtual void send(const ImgBase *image){
        if(impl) impl->send(image);
        else{
          ERROR_LOG("unable to send image with a NULL output");
        }
      }
      
      /// returns whether this instance was already initialized
      inline bool isNull() const { return !impl; };
      
      /// retusn current type string
      inline const std::string &getType() const { return type; }
  
      /// retusn current description string
      inline const std::string &getDescription() const { return description; }
  
      /// sets the implementations compression options
      virtual void setCompression(const ImageCompressor::CompressionSpec &spec){
        ICLASSERT_THROW(impl,ICLException("GenericImageOutput:setCompression: impl was null"));
        impl->setCompression(spec);
      }
      
      /// returns the implementation's current compression type (result.first) and quality (result.second)
      virtual CompressionSpec getCompression() const{
        ICLASSERT_THROW(impl,ICLException("GenericImageOutput:getCompression: impl was null"));
        return impl->getCompression();
      }
    };
  } // namespace io
}

#endif
