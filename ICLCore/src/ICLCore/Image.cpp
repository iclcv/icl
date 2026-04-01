// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <ICLCore/Image.h>
#include <ICLCore/ImgBase.h>
#include <ICLCore/Img.h>
#include <ICLCore/CoreFunctions.h>
#include <iostream>

namespace icl {
  namespace core {

    // --- Constructors ---

    Image::Image() = default;

    Image::Image(const utils::Size &s, depth d, int channels, format fmt)
      : m_impl(imgNew(d, s, channels, fmt))
    {}

    Image::Image(ImgBase *p)
      : m_impl(p)
    {}

    Image::Image(const std::shared_ptr<ImgBase> &p)
      : m_impl(p)
    {}

    Image::Image(const ImgBase &img)
      : m_impl(const_cast<ImgBase&>(img).shallowCopy())
    {}

    // --- Null check ---

    bool Image::isNull() const { return !m_impl; }
    Image::operator bool() const { return !!m_impl; }

    // --- Metadata ---

    utils::Size Image::getSize() const { return m_impl->getSize(); }
    int Image::getWidth() const { return m_impl->getWidth(); }
    int Image::getHeight() const { return m_impl->getHeight(); }
    int Image::getDim() const { return m_impl->getDim(); }
    utils::Rect Image::getImageRect() const { return utils::Rect(utils::Point::null, getSize()); }
    int Image::getChannels() const { return m_impl->getChannels(); }
    depth Image::getDepth() const { return m_impl->getDepth(); }
    format Image::getFormat() const { return m_impl->getFormat(); }
    utils::Time Image::getTime() const { return m_impl->getTime(); }
    void Image::setTime(const utils::Time &t) { m_impl->setTime(t); }
    void Image::setFormat(format fmt) { m_impl->setFormat(fmt); }

    // --- ROI ---

    utils::Rect Image::getROI() const { return m_impl->getROI(); }
    utils::Size Image::getROISize() const { return m_impl->getROISize(); }
    utils::Point Image::getROIOffset() const { return m_impl->getROIOffset(); }
    void Image::setROI(const utils::Rect &roi) { m_impl->setROI(roi); }
    void Image::setROI(const utils::Point &offset, const utils::Size &size) {
      m_impl->setROI(offset, size);
    }
    void Image::setFullROI() { m_impl->setFullROI(); }
    bool Image::hasFullROI() const { return m_impl->hasFullROI(); }

    // --- Buffer management ---

    void Image::ensureCompatible(depth d, const utils::Size &s,
                                 int channels, format fmt) {
      if(!m_impl || m_impl->getDepth() != d){
        m_impl.reset(imgNew(d, s, channels, fmt));
      } else {
        // Same depth — batch-update params in one call
        m_impl->setParams(ImgParams(s, channels, fmt));
      }
    }

    void Image::ensureCompatible(const Image &other) {
      if(other.isNull()) return;
      ensureCompatible(other.getDepth(), other.getSize(),
                       other.getChannels(), other.getFormat());
    }

    void Image::setSize(const utils::Size &s) { m_impl->setSize(s); }
    void Image::setChannels(int n) { m_impl->setChannels(n); }

    // --- Data independence ---

    void Image::detach(int channel) { m_impl->detach(channel); }
    bool Image::isIndependent() const { return m_impl->isIndependent(); }

    // --- Equality ---

    bool Image::operator==(const Image &other) const {
      if(isNull() && other.isNull()) return true;
      if(isNull() || other.isNull()) return false;
      if(getDepth() != other.getDepth()) return false;
      return visit([&](const auto &a) {
        using T = typename std::remove_reference_t<decltype(a)>::type;
        return a == other.as<T>();
      });
    }

    // --- Copy / Convert ---

    Image Image::deepCopy() const {
      if(isNull()) return Image();
      return Image(m_impl->deepCopy());
    }

    Image Image::convert(depth d) const {
      if(isNull()) return Image();
      return Image(m_impl->convert(d));
    }

    void Image::convertTo(Image &dst) const {
      if(isNull()) return;
      m_impl->convert(dst.ptr());
    }

    void Image::convertROITo(Image &dst) const {
      if(isNull()) return;
      m_impl->convertROI(dst.ptr());
    }

    Image Image::scaledCopy(const utils::Size &newSize, scalemode sm) const {
      if(isNull()) return Image();
      return Image(m_impl->scaledCopy(newSize, sm));
    }

    // --- Channel operations ---

    Image Image::selectChannel(int c) const {
      if(isNull()) return Image();
      return Image(std::shared_ptr<ImgBase>(m_impl->selectChannel(c)));
    }

    Image Image::selectChannels(const std::vector<int> &channels) const {
      if(isNull()) return Image();
      return Image(std::shared_ptr<ImgBase>(m_impl->selectChannels(channels)));
    }

    // --- In-place operations ---

    void Image::clear(int channel, icl64f val) {
      m_impl->clear(channel, val, false);
    }

    void Image::mirror(axis a) { m_impl->mirror(a); }

    Image Image::mirrored(axis a) const {
      Image dst = deepCopy();
      dst.mirror(a);
      return dst;
    }

    void Image::scale(const utils::Size &s, scalemode sm) {
      m_impl->scale(s, sm);
    }

    void Image::normalizeAllChannels(const utils::Range<icl64f> &dstRange) {
      m_impl->normalizeAllChannels(dstRange);
    }

    // --- Min / Max ---

    icl64f Image::getMin(int channel) const { return m_impl->getMin(channel); }
    icl64f Image::getMax(int channel) const { return m_impl->getMax(channel); }
    utils::Range<icl64f> Image::getMinMax(int channel) const {
      return m_impl->getMinMax(channel);
    }

    // --- Raw pointer access ---

    ImgBase* Image::ptr() { return m_impl.get(); }
    const ImgBase* Image::ptr() const { return m_impl.get(); }

    // --- Swap ---

    void Image::swap(Image &other) { m_impl.swap(other.m_impl); }

    // --- Debug ---

    void Image::print(const std::string &title) const {
      if(isNull()){
        std::cout << title << ": [null image]" << std::endl;
      } else {
        m_impl->print(title);
      }
    }

    // --- Stream output ---

    std::ostream& operator<<(std::ostream &s, const Image &img) {
      if(img.isNull()){
        return s << "[null image]";
      }
      return s << img.getSize() << "x" << img.getChannels() << "ch "
               << img.getDepth() << " " << img.getFormat();
    }

  } // namespace core
} // namespace icl
