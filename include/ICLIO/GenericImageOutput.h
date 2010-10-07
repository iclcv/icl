/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
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
#include <ICLIO/ImageOutput.h>
#include <ICLUtils/SmartPtr.h>

namespace icl{
  
  /// Generic Sink for images
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
    
    /// initialize this instance
    /** Like the GenericGrabber, this 'generic tool' can also be set up by
        two string specifiers 
        Possible types are:
        - "file" (description=filepattern)
        - "video" (description=output-video-filename,CODEC-FOURCCC=DIV3,VideoSize=VGA,FPS=24)
        - "sm" (SharedMemory output, description=memory-segment-ID)
        - "xcfp" (XCF Publisher output, description=stream-name)
    */
    void init(const std::string &type, const std::string &description);
    
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
  };
}

#endif
