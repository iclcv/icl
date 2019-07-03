/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/SharedMemoryPublisher.cpp              **
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

#include <ICLIO/SharedMemoryPublisher.h>
#include <ICLUtils/StringUtils.h>
#include <ICLCore/ImageSerializer.h>
#include <ICLCore/Img.h>
#include <ICLIO/ImageCompressor.h>

using namespace icl::utils;
using namespace icl::core;


namespace icl{
  namespace io{

    static std::string ICL_IMGBASE_STREAM_PREPEND = "icl.core.imgbase.";

    struct SharedMemoryPublisher::Data{
      std::string name;
      SharedMemorySegment mem;
    };

    SharedMemoryPublisher::SharedMemoryPublisher(const std::string &memorySegmentName){
      m_data = new Data;
      createPublisher(memorySegmentName);
    }

    SharedMemoryPublisher::~SharedMemoryPublisher(){
      delete m_data;
    }

    void SharedMemoryPublisher::createPublisher(const std::string &memorySegmentName){
      m_data->name = memorySegmentName;
      m_data->mem.reset(ICL_IMGBASE_STREAM_PREPEND + memorySegmentName);

      Img8u tmp;
      tmp.setTime();
      publish(&tmp);
    }

    void SharedMemoryPublisher::publish(const ImgBase *image){
      if(!image) return;
      CompressedData data = compress(image, false); // why was this set to true?
                                                    // why did we skip meta data?

      SharedMemorySegmentLocker l(m_data->mem,data.len);
      if(m_data->mem.getSize() < data.len){
        DEBUG_LOG("segment too small " << m_data->mem.getSize() << "-" << data.len)
            return;
      }
      std::copy(data.bytes, data.bytes+data.len,(icl8u*)m_data->mem.data());
    }

    std::string SharedMemoryPublisher::getMemorySegmentName() const{
      return m_data->mem.getName();
    }

  } // namespace io
}


