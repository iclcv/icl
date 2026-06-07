// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Viktor Richter

#include <icl/io/detail/sources/CreateSource.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/io/source/TestImages.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::io {
  Image CreateSource::acquireImage(){
    if(m_updateTimeStamp){
      m_image.ptr()->setTime();
    }
    return m_image;
  }

  CreateSource::CreateSource(const std::string &what){
    m_updateTimeStamp = true;
    m_image = TestImages::create(what);
    if(m_image.isNull()) throw ICLException("unable to create a 'CreateSource' from given description '"+what+"'");
    addProperty("format", prop::Info{}, "RGB", "");
    addProperty("size", prop::Info{}, "512x512", "");
    addProperty("update timestamp", prop::Flag{}, m_updateTimeStamp, "Whether the timestamp of the image should be set everytime an the image is grabbed.");
    registerCallback([this](const utils::Configurable::Property &p){ processPropertyChange(p); });
  }

  CreateSource::~CreateSource() = default;

  void CreateSource::processPropertyChange(const utils::Configurable::Property &prop){
    if(prop.name == "update timestamp") {
      m_updateTimeStamp = prop.as<bool>();
    }
  }

  REGISTER_CONFIGURABLE(CreateSource, return new CreateSource("parrot"));

  SourceBackend* createCreateSource(const std::string &param){
    return new CreateSource(param);
  }

  const std::vector<DeviceDescription>& getCreateDeviceList(std::string hint, bool rescan){
    // Rebuilt on each call: the test-image registry is populated by
    // static-init-time REGISTER_TEST_IMAGE(...) invocations in sibling
    // TUs, and new backends may register after the first call here.
    static std::vector<DeviceDescription> deviceList;
    deviceList.clear();
    for(const std::string &name : testImageRegistry().keys()){
      deviceList.push_back(DeviceDescription("create", name,
                  std::string("built-in test image '") + name + "'"));
    }
    return deviceList;
  }

  REGISTER_SOURCE_BACKEND(create, createCreateSource, getCreateDeviceList,
                   "parrot|lena|cameraman|mandril|flowers|windows|women|tree|house~built-in test image");

  } // namespace icl::io