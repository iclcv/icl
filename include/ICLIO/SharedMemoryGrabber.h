/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/SharedMemoryGrabber.h                    **
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

#ifndef ICL_SHARED_MEMORY_GRABBER_H
#define ICL_SHARED_MEMORY_GRABBER_H

#include <ICLIO/GrabberHandle.h>

namespace icl{
  
  /// Grabber class that grabs images from QSharedMemory instances
  /** Images that are published using the SharedMemoryPublisher can 
      be grabbed with this grabber type. Please don't use this
      Grabber class directly, but instantiate GenericGrabber with
      Devide type 'sm'.
  */
  class SharedMemoryGrabberImpl : public Grabber {
    /// Internal Data storage class
    struct Data;
    
    /// Hidden Data container
    Data *m_data;


    /// Creates a new SharedMemoryGrabber instance (please use the GenericGrabber instead)
    SharedMemoryGrabberImpl(const std::string &sharedMemorySegmentID="") throw(ICLException);
    
    /// Connects an unconnected grabber to given shared memory segment
    void init(const std::string &sharedMemorySegmentID) throw (ICLException);
    
    public:
    
    /// Only the 'real' graber can instantiate the -Impl
    friend class SharedMemoryGrabber;
    
    /// Destructor
    ~SharedMemoryGrabberImpl();
    
    /// returns a list of all available shared-memory image-streams
    /** evaluates the special memory segment named 
        'icl-shared-mem-grabbers' in order to find 
        out which devices are available 
    */
    static const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan);

    /// grabbing function  
    /** \copydoc icl::Grabber::grab(ImgBase**)  **/    
    virtual const ImgBase* grabUD(ImgBase **ppoDst=0);

    /** @{ @name properties and parameters */
#if 0
    CURRENTLY, THIS GRABBER TYPE DOES NOT SUPPORT PROPERTIES
    
    /// interface for the setter function for video device properties 
    /** \copydoc icl::Grabber::setProperty(const std::string&,const std::string&) **/
    virtual void setProperty(const std::string &property, const std::string &value);
    
    /// returns a list of properties, that can be set usingsetProperty
    /** @return list of supported property names **/
    virtual std::vector<std::string> getPropertyList();
    
    /// get type of property
    /** \copydoc icl::Grabber::getType(const std::string &)*/
    virtual std::string getType(const std::string &name);

    /// get information of a property valid values
    /** \copydoc icl::Grabber::getInfo(const std::string &)*/
    virtual std::string getInfo(const std::string &name);

    /// returns the current value of a property or a parameter
    virtual std::string getValue(const std::string &name);
    /** @} */
#endif
  };


  /// Grabber class that grabs images from QSharedMemory instances
  /** for more details: @see SharedMemoryGrabberImpl */
  class SharedMemoryGrabber : public GrabberHandle<SharedMemoryGrabberImpl>{
    public:
    
    /// returns a list of available pwc devices 
    /** @see SharedMemoryGrabberImpl for more details*/
    static inline const std::vector<GrabberDeviceDescription> &getDeviceList(bool rescan){
      return SharedMemoryGrabberImpl::getDeviceList(rescan);
    }
    
    /// creates a new PWCGrabber instance
    /** @see SharedMemoryGrabberImpl for more details */
    inline SharedMemoryGrabber(const std::string &memorySegmentName){
      if(isNew(memorySegmentName)){
        initialize(new SharedMemoryGrabberImpl(memorySegmentName),memorySegmentName);
      }else{
        initialize(memorySegmentName);
      }
    }
    /// empty constructor (initialize late using init())
    /** @see SharedMemoryGrabberImpl for more details */
    inline SharedMemoryGrabber(){}
    
    /// for deferred connection to (other) shared memory segment
    /** @see SharedMemoryGrabberImpl for more details */
    inline void init(const std::string &memorySegmentName) throw (ICLException){
      if(isNew(memorySegmentName)){
        initialize(new SharedMemoryGrabberImpl(memorySegmentName),memorySegmentName);
      }else{
        initialize(memorySegmentName);
      }
    }
    
    /// resets the internal list of 'shared-grabbers'
    static void resetBus();
  };


  
}

#endif
