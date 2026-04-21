// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/core/Image.h>
#include <icl/core/ImgBase.h>
#include <icl/core/Img.h>
#include <icl/core/CoreFunctions.h>
#include <iostream>
#include <cstring>
#include <map>

namespace icl::core {
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
  bool Image::isExclusivelyOwned() const { return m_impl && m_impl.use_count() == 1; }

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

  Image Image::shallowCopy() const {
    if(isNull()) return Image();
    return Image(m_impl->shallowCopy());
  }

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

  // --- Memory ---

  size_t Image::memoryUsage() const {
    if(isNull()) return 0;
    return size_t(getDim()) * getChannels() * getSizeOf(getDepth());
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

  // --- addLabel --- ASCII-art text label rendered into the upper-left corner.
  namespace {
    using utils::Point;
    using utils::Rect;

    struct OffsPtr {
      OffsPtr(const Point &offs = Point::null, icl8u *img = 0, int width = 0)
        : offs(offs), img(img), width(width) {}
      Point  offs;
      icl8u *img;
      int    width;
    };

    template<class T>
    void drawLabel(Img<T> *image, const char *txt,
                   const std::map<char, OffsPtr> &m)
    {
      static const Point OFS(5, 5);

      Rect roi = Rect(OFS.x - 1, OFS.y - 1,
                      int(strlen(txt)) * (7 + 2), 7 + 2);
      if(Rect(Point::null, image->getSize()).contains(roi)){
        Img<T> *boxImage = image->shallowCopy(roi);
        boxImage->clear();
        delete boxImage;
      }

      int letterIdx = 0;
      for(const char *p = txt; *p; p++){
        auto it = m.find(*p);
        if(it == m.end()) continue;
        const OffsPtr &op = it->second;

        for(int c = 0; c < image->getChannels(); c++){
          int xStartLetter = op.offs.x;
          int xEndLetter   = xStartLetter + 7;
          int xStartImage  = OFS.x + letterIdx * (7 + 2);
          for(int xL = xStartLetter, xI = xStartImage;
              xL < xEndLetter; xI++, xL++){
            if(xI < image->getWidth()){
              int yStartLetter = 0;
              int yEndLetter   = 7;
              int yStartImage  = OFS.y;
              for(int yL = yStartLetter, yI = yStartImage;
                  yL < yEndLetter; yI++, yL++){
                if(yI < image->getHeight()){
                  (*image)(xI, yI, c) =
                    (char(op.img[xL + yL * op.width]) == '#') ? T(255) : T(0);
                }
              }
            }
          }
        }
        letterIdx++;
      }
    }

    const std::map<char, OffsPtr>& getFontMap() {
      static char ABC_AM[] =
      " ##### ######  ############ ############## ##### #     ################     ##      #     #"
      "#     ##     ##      #     ##      #      #     ##     #   #         ##    # #      ##   ##"
      "#     ##     ##      #     ##      #      #      #     #   #         ##   #  #      # # # #"
      "#     ####### #      #     ################  ###########   #         #####   #      #  #  #"
      "########     ##      #     ##      #      #     ##     #   #         ##   #  #      #     #"
      "#     ##     ##      #     ##      #      #     ##     #   #         ##    # #      #     #"
      "#     #######  ############ ########       ##### #     ############## #     #########     #";

      static char ABC_NZ[] =
      "#     # ##### ######  ##### ######  ##### ########     ##     ##     ##     ##     ########"
      "##    ##     ##     ##     ##     ##     #   #   #     ##     ##     # #   #  #   #      # "
      "# #   ##     ##     ##     ##     ##         #   #     # #   # #     #  # #    # #      #  "
      "#  #  ##     ####### #     #######  #####    #   #     # #   # #  #  #   #      #      #   "
      "#   # ##     ##      # ### ##   #        #   #   #     #  # #  # # # #  # #     #     #    "
      "#    ###     ##      #    ###    # #     #   #   #     #  # #  ##   ## #   #    #    #     "
      "#     # ##### #       ##### #     # #####    #    #####    #   #     ##     #   #   #######";

      static char ABC_09[] =
      " #####      ## #####  ##### #   #  ####### ##### ####### #####  ##### "
      "#     #   ## ##     ##     ##   #  #      #     #      ##     ##     #"
      "#     # ##   #      #      ##   #  #      #           # #     ##     #"
      "#     ##     #  ####  ##### ############# ######  ###### #####  ######"
      "#     #      ###           #    #        ##     #   #   #     #      #"
      "#     #      ##      #     #    #  #     ##     #  #    #     ##     #"
      " #####       ######## #####     #   #####  #####  #      #####  ##### ";

      static char ABC_EX[] =
      "          #    #   #  #   #  #####  #    # ###      #       #    #     # # #                                   #"
      "          #    #   # ########  #  ## #  # #   #     #      #      #     ###     #                             # "
      "          #           #   #    #  # #  #   ###             #      #    #####    #                            #  "
      "          #           #   #  #####    #    # #            #        #    ###   #####         #####           #   "
      "          #           #   # #  #     #  # #   ##           #      #    # # #    #       #                  #    "
      "                     ########  #  # #  # ##   ##           #      #             #      #             ##   #     "
      "          #           #   #  ##### #    #  #### #           #    #                    #              ##  #      ";

      static std::map<char, OffsPtr> m = [&]{
        std::map<char, OffsPtr> r;
        for(int c = 'a', xoffs = 0; c <= 'm'; c++, xoffs += 7){
          r[c]      = OffsPtr(Point(xoffs, 0), reinterpret_cast<icl8u*>(ABC_AM), 13 * 7);
          r[c - 32] = OffsPtr(Point(xoffs, 0), reinterpret_cast<icl8u*>(ABC_AM), 13 * 7);
        }
        for(int c = 'n', xoffs = 0; c <= 'z'; c++, xoffs += 7){
          r[c]      = OffsPtr(Point(xoffs, 0), reinterpret_cast<icl8u*>(ABC_NZ), 13 * 7);
          r[c - 32] = OffsPtr(Point(xoffs, 0), reinterpret_cast<icl8u*>(ABC_NZ), 13 * 7);
        }
        for(int c = '0', xoffs = 0; c <= '9'; c++, xoffs += 7){
          r[c] = OffsPtr(Point(xoffs, 0), reinterpret_cast<icl8u*>(ABC_09), 10 * 7);
        }
        for(int c = ' ', xoffs = 0; c <= '/'; c++, xoffs += 7){
          r[c] = OffsPtr(Point(xoffs, 0), reinterpret_cast<icl8u*>(ABC_EX), 16 * 7);
        }
        return r;
      }();
      return m;
    }
  } // anonymous namespace

  void Image::addLabel(const std::string &label) {
    ICLASSERT_RETURN(!isNull());
    ICLASSERT_RETURN(label.length());

    ImgBase *image = m_impl.get();
    const auto &m = getFontMap();
    switch(image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) \
      case depth##D: drawLabel<icl##D>(image->asImg<icl##D>(), label.c_str(), m); break;
      ICL_INSTANTIATE_ALL_DEPTHS
#undef ICL_INSTANTIATE_DEPTH
    }
  }

  } // namespace icl::core