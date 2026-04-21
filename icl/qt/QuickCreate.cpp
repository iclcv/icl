// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/QuickCreate.h>
#include <icl/qt/QuickContext.h>
#include <icl/io/FileGrabber.h>
#include <icl/io/GenericGrabber.h>
#include <icl/io/TestImages.h>
#include <icl/core/ImgBase.h>
#include <icl/core/CCFunctions.h>
#include <icl/core/Img.h>

#include <memory>

using namespace icl::core;
using namespace icl::utils;
using namespace icl::io;

namespace icl::qt {

  Image zeros(int width, int height, int channels, depth d) {
    Image img = activeContext().getBuffer(d, Size(width, height), channels);
    img.clear(-1, 0);
    return img;
  }

  Image ones(int width, int height, int channels, depth d) {
    Image img = activeContext().getBuffer(d, Size(width, height), channels);
    img.clear(-1, 1);
    return img;
  }

  Image load(const std::string &filename) {
    FileGrabber g(filename);
    try {
      return g.grabImage();
    } catch(const ICLException &ex) {
      ERROR_LOG("exception: " << ex.what());
      return Image();
    }
  }

  Image load(const std::string &filename, format fmt) {
    Image img = load(filename);
    if(img.isNull()) return img;

    Image dst = activeContext().getBuffer(img.getDepth(),
        ImgParams(img.getSize(), getChannelsOfFormat(fmt), fmt));
    Image src = img;
    if(src.getFormat() == formatMatrix && src.getChannels() == 1)
      src.setFormat(formatGray);
    icl::core::cc(src.ptr(), dst.ptr());
    return dst;
  }

  Image create(const std::string &name, format fmt, std::optional<depth> d) {
    // TestImages::create returns a new ImgBase* (caller owns it)
    std::unique_ptr<ImgBase> raw(TestImages::create(name, fmt));
    if(!raw) {
      ERROR_LOG("unable to create test image: \"" << name << "\"");
      return Image();
    }
    depth outDepth = d.value_or(raw->getDepth());
    Image dst = activeContext().getBuffer(outDepth,
        ImgParams(raw->getSize(), raw->getChannels(), raw->getFormat()));
    ImgBase *dstPtr = dst.ptr();
    raw->convert(dstPtr);
    return dst;
  }

  Image grab(const std::string &dev, const std::string &devSpec,
             const Size &size, format fmt, bool releaseGrabber) {
    auto &ctx = activeContext();

    // releaseGrabber=true: one-shot, don't cache the device connection
    std::shared_ptr<GenericGrabber> tmp;
    if(releaseGrabber) {
      tmp = std::make_shared<GenericGrabber>();
      tmp->init(dev, devSpec);
    }
    auto &g = releaseGrabber ? *tmp : *ctx.getGrabber(dev, devSpec);

    if(size != Size::null) {
      g.useDesired(size);
      g.useDesired(fmt);
    }

    Image img = g.grabImage();

    if(img.getFormat() != fmt) {
      Image dst = ctx.getBuffer(img.getDepth(),
          ImgParams(img.getSize(), getChannelsOfFormat(fmt), fmt));
      Image src = img;
      if(src.getFormat() == formatMatrix && src.getChannels() == 1)
        src.setFormat(formatGray);
      icl::core::cc(src.ptr(), dst.ptr());
      return dst;
    }
    return img;
  }

} // namespace icl::qt
