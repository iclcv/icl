/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLIO/SharedMemoryPublisher.h                  **
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

#ifndef ICL_SHARED_MEMORY_PUBLISHER_H
#define ICL_SHARED_MEMORY_PUBLISHER_H

#include <ICLCore/ImgBase.h>
#include <ICLIO/ImageOutput.h>
#include <ICLUtils/Uncopyable.h>

namespace icl{

  /// Publisher, that can be used to publish images via Qt's QSharedMemory
  /** The publisher automatically creates a 2nd memory segment named
      'icl-shared-mem-grabbers' that is set up to contain a list of
      all available ICL shared memory streams 
      If two publishers are set up to publish to one memory, the result
      is undetermined.
  */
  class SharedMemoryPublisher : public ImageOutput, public Uncopyable{
    struct Data;  //!< intenal data
    Data *m_data; //!< intenal data
    
    public:
    
    /// Creates a new publisher instance
    /** If memorySegmentName is "", no connection is performed */
    SharedMemoryPublisher(const std::string &memorySegmentName="") throw (ICLException);
    
    /// Destructor
    ~SharedMemoryPublisher();
    
    /// sets the publisher to use a new segment
    void createPublisher(const std::string &memorySegmentName="") throw (ICLException);
    
    /// publishs given image
    void publish(const ImgBase *image);
    
    /// wraps publish to implement ImageOutput interface
    virtual void send(const ImgBase *image) { publish(image); }

    /// returns current memory segment name
    std::string getMemorySegmentName() const throw (ICLException);
  };
}
#endif
