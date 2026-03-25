/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLCore/src/ICLCore/Image.cpp                          **
** Module : ICLCore                                                **
** Authors: Christof Elbrechter                                    **
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
********************************************************************/

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
      if(!m_impl){
        m_impl.reset(imgNew(d, s, channels, fmt));
      } else if(m_impl->getDepth() != d){
        // Depth change requires new allocation.
        // Other shared holders keep the old buffer.
        m_impl.reset(imgNew(d, s, channels, fmt));
      } else {
        // Same depth — adapt in-place (may reallocate channels internally)
        if(m_impl->getSize() != s) m_impl->setSize(s);
        if(m_impl->getChannels() != channels) m_impl->setChannels(channels);
        if(m_impl->getFormat() != fmt) m_impl->setFormat(fmt);
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

    // --- Copy / Convert ---

    Image Image::deepCopy() const {
      if(isNull()) return Image();
      return Image(m_impl->deepCopy());
    }

    Image Image::convert(depth d) const {
      if(isNull()) return Image();
      return Image(m_impl->convert(d));
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
