// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLIO/SharedMemoryPublisher.h>
#include <ICLUtils/StringUtils.h>
#include <ICLCore/ImageSerializer.h>
#include <ICLCore/Img.h>
#include <ICLIO/ImageCompressor.h>

using namespace icl::utils;
using namespace icl::core;


namespace icl::io {
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
    std::copy(data.bytes, data.bytes+data.len,static_cast<icl8u*>(m_data->mem.data()));
  }

  std::string SharedMemoryPublisher::getMemorySegmentName() const{
    return m_data->mem.getName();
  }

  } // namespace icl::io