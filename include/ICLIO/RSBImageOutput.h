/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/RSBImageOutput.h                         **
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

#ifndef ICL_RSB_IMAGE_OUTPUT_H
#define ICL_RSB_IMAGE_OUTPUT_H

#include <ICLIO/GenericImageOutput.h>

#if !defined(HAVE_RSB) || !defined(HAVE_PROTOBUF)
#warning "This header should only be included if HAVE_RSB and HAVE_PROTOBUF are defined and available in ICL"
#endif
namespace icl{
  
  /// image output implementation using the "Robotics Service Bus (RSB)"
  class RSBImageOutput : public ImageOutput{
    struct Data;  //!< pimpl type
    Data *m_data; //!< pimpl pointer
    
    protected:
    /// this can be overwritten in subclasses, that also provide an interface for sending meta-data
    /** This classes send-method will always 'ask' getMetaData() whether there is extra data that
        has to be attached to the image. Note, that the RSBGrabber (and also the GenericGrabber)
        does not have an interface for accessing meta data. Use the RSBDataReceiver instead  */
    virtual const std::string *getMetaData() { return 0; }

    public:
    /// creates a null instance
    RSBImageOutput();

    /// Destructor
    ~RSBImageOutput();
    
    /// create RSBImageOutput with given scope and comma separated transport list
    /** supported transports are socket, spread and inprocess. Please note, that
        the spread-transport requires spread running. */
    RSBImageOutput(const std::string &scope, const std::string &transportList="spread");
    
    /// deferred initialization (see RSBImageOutput(const string&,const string&)
    /** supported transports are socket, spread and inprocess. Please note, that
        the spread-transport requires spread running. */
    void init(const std::string &scope, const std::string &transportList="spread");
    
    /// sets the current compression type
    /** Valid compression modes are
        - off no compression (default, possible for all data types)
        - rle run lenght encoding (quality can be 1, 4 or 6, it defines 
          the number of bits, that are used for the value domain. 
          Default value is 1, which is used for binary images)
        - jpeg jpeg encoding (quality is default jpeg quality 1% - 100%)
    */
    void setCompression(const std::string &compression, const std::string &quality="default");
    
    /// returns the current compression type (first) and quality (second)
    std::pair<std::string,std::string> getCompression() const;
    
    /// sender method
    virtual void send(const ImgBase *image);
    
    /// returns whether this is a null instance
    inline bool isNull() const { return !m_data; }
    
    /// returns whether this is not a null instance
    inline operator bool() const { return static_cast<bool>(m_data); }
  };
}

#endif
