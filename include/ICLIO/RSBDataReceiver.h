/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/RSBDataReceiver.h                        **
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

#ifndef ICL_RSB_DATA_RECEIVER_H
#define ICL_RSB_DATA_RECEIVER_H

#include <ICLIO/RSBGrabber.h>
#include <ICLUtils/Function.h>


#if !defined(HAVE_RSB) || !defined(HAVE_PROTOBUF)
#warning "This Header should not be included if HAVE_RSB or HAVE_PROTOBUF is not set"
#endif

namespace icl{
  /// Utility class for simple and out-of-the-box sending data +image-attachments via RSB
  /** In contrast to the icl::GenericImageOutput/icl::RSBImageOutput, the RSBDataReceiver
      is mainly designed to send data with an optional image attachment. Internally,
      the image is sent in the same way, the icl::RSBImageOutput sends images. By these
      means, the sent images can also be received with a standard GenericGrabber and
      therefore also with icl-camviewer 
      
      \section _MODES_ Use Cases
      
      There are two different ways of how to use the RSBDataReceiver class
      -# you can 
      
      
  */
  class RSBDataReceiver : public RSBGrabberImpl{
    struct Data;  //!< pimpl type
    Data *m_data; //!< pimpl-pointer
    
    protected:
    /// overwritten from the RSBGrabber class
    virtual void newDataAvailable(const ImgBase *image, const std::string &metadata);

    public:
    /// callback type for callback based usage of this class
    typedef Function<void,const ImgBase*,const std::string&> callback;
    
    /// empty constructor
    RSBDataReceiver();
    
    /// constructor with given scope definition and trasport list
    /** @see RSBImageOutput constructor for more details */
    RSBDataReceiver(const std::string &scope, const std::string &transportList="spread", bool storeLast=true);

    /// Destructor
    ~RSBDataReceiver();

    /// initialization 
    void init(const std::string &scope, const std::string &transportList="spread", bool storeLast=true);
    
    /// send string data plus an optional image attachment
    const ImgBase *receive(std::string *meta=0);
    
    /// registers a given callback that is called everytime, new data is available
    void registerCallback(callback cb);
    
    /// removes all callbacks
    void removeCallbacks();
    
    /// sets whether received meta data is buffered internally
    void setStoreLastEnabled(bool enabled);
  };
}

#endif
