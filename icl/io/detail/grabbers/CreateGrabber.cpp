// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Viktor Richter

#include <icl/io/detail/grabbers/CreateGrabber.h>
#include <icl/utils/prop/Constraints.h>
#include <icl/io/TestImages.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl::io {
  const ImgBase* CreateGrabber::acquireDisplay(){
    if(m_updateTimeStamp){
      m_image -> setTime();
    }
    return m_image;
  }

  CreateGrabber::CreateGrabber(const std::string &what){
    m_updateTimeStamp = true;
    m_image = TestImages::create(what);
    if(!m_image) throw ICLException("unable to create a 'CreateGrabber' from given description '"+what+"'");
    addProperty("format", prop::Info{}, "RGB", 0, "");
    addProperty("size", prop::Info{}, "512x512", 0, "");
    addProperty("update timestamp", prop::Flag{}, m_updateTimeStamp, 0, "Whether the timestamp of the image should be set everytime an the image is grabbed.");
    registerCallback([this](const utils::Configurable::Property &p){ processPropertyChange(p); });
  }

  CreateGrabber::~CreateGrabber(){
    ICL_DELETE(m_image);
  }

  void CreateGrabber::processPropertyChange(const utils::Configurable::Property &prop){
    if(prop.name == "update timestamp") {
      m_updateTimeStamp = prop.as<bool>();
    }
  }

  REGISTER_CONFIGURABLE(CreateGrabber, return new CreateGrabber("parrot"));

  Grabber* createCreateGrabber(const std::string &param){
    return new CreateGrabber(param);
  }

  const std::vector<GrabberDeviceDescription>& getCreateDeviceList(std::string hint, bool rescan){
    // Rebuilt on each call: the test-image registry is populated by
    // static-init-time REGISTER_TEST_IMAGE(...) invocations in sibling
    // TUs, and new backends may register after the first call here.
    static std::vector<GrabberDeviceDescription> deviceList;
    deviceList.clear();
    for(const std::string &name : testImageRegistry().keys()){
      deviceList.push_back(GrabberDeviceDescription("create", name,
                  std::string("built-in test image '") + name + "'"));
    }
    return deviceList;
  }

  REGISTER_GRABBER(create, createCreateGrabber, getCreateDeviceList,
                   "create:parrot|lena|cameraman|mandril|flowers|windows|women|tree|house:built-in test image");

  } // namespace icl::io