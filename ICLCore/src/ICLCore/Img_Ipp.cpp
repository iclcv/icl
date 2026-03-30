#include <ICLCore/ImgOps.h>
#include <ICLCore/Img.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  // ---- mirror (in-place) ----
  void ipp_mirror(ImgBase& img, axis a, bool roiOnly) {
    const int lineStep = img.getLineStep();
    const Point& offset = roiOnly ? img.getROIOffset() : Point::null;
    const Size& size = roiOnly ? img.getROISize() : img.getSize();

    for(int c = 0; c < img.getChannels(); ++c) {
      switch(img.getDepth()) {
        case depth8u:
          ippiMirror_8u_C1IR(img.asImg<icl8u>()->getROIData(c, offset),
                             lineStep, size, static_cast<IppiAxis>(a));
          break;
        case depth16s:
          ippiMirror_16u_C1IR(reinterpret_cast<Ipp16u*>(img.asImg<icl16s>()->getROIData(c, offset)),
                              lineStep, size, static_cast<IppiAxis>(a));
          break;
        case depth32s:
          ippiMirror_32s_C1IR(img.asImg<icl32s>()->getROIData(c, offset),
                              lineStep, size, static_cast<IppiAxis>(a));
          break;
        case depth32f:
          ippiMirror_32s_C1IR(reinterpret_cast<Ipp32s*>(img.asImg<icl32f>()->getROIData(c, offset)),
                              lineStep, size, static_cast<IppiAxis>(a));
          break;
        default: break;
      }
    }
  }

  // ---- clearChannelROI ----
  void ipp_clearChannelROI(ImgBase& img, int ch, icl64f val,
                           const Point& offs, const Size& size) {
    switch(img.getDepth()) {
      case depth8u:
        ippiSet_8u_C1R(static_cast<icl8u>(val),
                       img.asImg<icl8u>()->getROIData(ch, offs),
                       img.getLineStep(), size);
        break;
      case depth16s:
        ippiSet_16s_C1R(static_cast<icl16s>(val),
                        img.asImg<icl16s>()->getROIData(ch, offs),
                        img.getLineStep(), size);
        break;
      case depth32s:
        ippiSet_32s_C1R(static_cast<icl32s>(val),
                        img.asImg<icl32s>()->getROIData(ch, offs),
                        img.getLineStep(), size);
        break;
      case depth32f:
        ippiSet_32f_C1R(static_cast<icl32f>(val),
                        img.asImg<icl32f>()->getROIData(ch, offs),
                        img.getLineStep(), size);
        break;
      default: break;
    }
  }

  // ---- lut (icl8u only) ----
  void ipp_lut(ImgBase& srcBase, const void* lutPtr, ImgBase& dstBase, int bits) {
    auto& src = *srcBase.asImg<icl8u>();
    auto& dst = *dstBase.asImg<icl8u>();
    const icl8u* lut = static_cast<const icl8u*>(lutPtr);
    for(int c = src.getChannels()-1; c >= 0; --c) {
      ippiLUTPalette_8u_C1R(src.getROIData(c), src.getLineStep(),
                             dst.getROIData(c), dst.getLineStep(),
                             src.getROISize(), lut, bits);
    }
  }

  // ---- getMax ----
  icl64f ipp_getMax(ImgBase& img, int ch, Point* coords) {
    switch(img.getDepth()) {
      case depth8u: {
        icl8u vMax = 0;
        if(coords)
          ippiMaxIndx_8u_C1R(img.asImg<icl8u>()->getROIData(ch), img.getLineStep(),
                              img.getROISize(), &vMax, &coords->x, &coords->y);
        else
          ippiMax_8u_C1R(img.asImg<icl8u>()->getROIData(ch), img.getLineStep(),
                          img.getROISize(), &vMax);
        return static_cast<icl64f>(vMax);
      }
      case depth16s: {
        icl16s vMax = 0;
        if(coords)
          ippiMaxIndx_16s_C1R(img.asImg<icl16s>()->getROIData(ch), img.getLineStep(),
                               img.getROISize(), &vMax, &coords->x, &coords->y);
        else
          ippiMax_16s_C1R(img.asImg<icl16s>()->getROIData(ch), img.getLineStep(),
                           img.getROISize(), &vMax);
        return static_cast<icl64f>(vMax);
      }
      case depth32f: {
        icl32f vMax = 0;
        if(coords)
          ippiMaxIndx_32f_C1R(img.asImg<icl32f>()->getROIData(ch), img.getLineStep(),
                               img.getROISize(), &vMax, &coords->x, &coords->y);
        else
          ippiMax_32f_C1R(img.asImg<icl32f>()->getROIData(ch), img.getLineStep(),
                           img.getROISize(), &vMax);
        return static_cast<icl64f>(vMax);
      }
      default: return 0;
    }
  }

  // ---- getMin ----
  icl64f ipp_getMin(ImgBase& img, int ch, Point* coords) {
    switch(img.getDepth()) {
      case depth8u: {
        icl8u vMin = 0;
        if(coords)
          ippiMinIndx_8u_C1R(img.asImg<icl8u>()->getROIData(ch), img.getLineStep(),
                              img.getROISize(), &vMin, &coords->x, &coords->y);
        else
          ippiMin_8u_C1R(img.asImg<icl8u>()->getROIData(ch), img.getLineStep(),
                          img.getROISize(), &vMin);
        return static_cast<icl64f>(vMin);
      }
      case depth16s: {
        icl16s vMin = 0;
        if(coords)
          ippiMinIndx_16s_C1R(img.asImg<icl16s>()->getROIData(ch), img.getLineStep(),
                               img.getROISize(), &vMin, &coords->x, &coords->y);
        else
          ippiMin_16s_C1R(img.asImg<icl16s>()->getROIData(ch), img.getLineStep(),
                           img.getROISize(), &vMin);
        return static_cast<icl64f>(vMin);
      }
      case depth32f: {
        icl32f vMin = 0;
        if(coords)
          ippiMinIndx_32f_C1R(img.asImg<icl32f>()->getROIData(ch), img.getLineStep(),
                               img.getROISize(), &vMin, &coords->x, &coords->y);
        else
          ippiMin_32f_C1R(img.asImg<icl32f>()->getROIData(ch), img.getLineStep(),
                           img.getROISize(), &vMin);
        return static_cast<icl64f>(vMin);
      }
      default: return 0;
    }
  }

  // ---- getMinMax ----
  void ipp_getMinMax(ImgBase& img, int ch,
                     icl64f* minVal, icl64f* maxVal,
                     Point* minCoords, Point* maxCoords) {
    switch(img.getDepth()) {
      case depth8u: {
        if(minCoords) {
          icl32f mn, mx;
          ippiMinMaxIndx_8u_C1R(img.asImg<icl8u>()->getROIData(ch), img.getLineStep(),
                                 img.getROISize(), &mn, &mx, minCoords, maxCoords);
          *minVal = static_cast<icl64f>(mn);
          *maxVal = static_cast<icl64f>(mx);
        } else {
          icl8u mn, mx;
          ippiMinMax_8u_C1R(img.asImg<icl8u>()->getROIData(ch), img.getLineStep(),
                             img.getROISize(), &mn, &mx);
          *minVal = static_cast<icl64f>(mn);
          *maxVal = static_cast<icl64f>(mx);
        }
        break;
      }
      case depth32f: {
        if(minCoords) {
          icl32f mn, mx;
          ippiMinMaxIndx_32f_C1R(img.asImg<icl32f>()->getROIData(ch), img.getLineStep(),
                                  img.getROISize(), &mn, &mx, minCoords, maxCoords);
          *minVal = static_cast<icl64f>(mn);
          *maxVal = static_cast<icl64f>(mx);
        } else {
          icl32f mn, mx;
          ippiMinMax_32f_C1R(img.asImg<icl32f>()->getROIData(ch), img.getLineStep(),
                              img.getROISize(), &mn, &mx);
          *minVal = static_cast<icl64f>(mn);
          *maxVal = static_cast<icl64f>(mx);
        }
        break;
      }
      default: *minVal = *maxVal = 0; break;
    }
  }

  // ---- normalize (icl32f only — MulC + AddC) ----
  void ipp_normalize(ImgBase& img, int iChannel,
                     icl64f srcMin, icl64f srcMax,
                     icl64f dstMin, icl64f dstMax) {
    auto& im = *img.asImg<icl32f>();
    icl32f srcLen = static_cast<icl32f>(srcMax - srcMin);
    icl32f fScale = static_cast<icl32f>(dstMax - dstMin) / srcLen;
    icl32f fShift = static_cast<icl32f>(srcMax * dstMin - srcMin * dstMax) / srcLen;

    for(int c = im.getStartIndex(iChannel); c < im.getEndIndex(iChannel); ++c) {
      ippiMulC_32f_C1IR(fScale, im.getROIData(c), im.getLineStep(), im.getROISize());
      if(fShift != 0) {
        ippiAddC_32f_C1IR(fShift, im.getROIData(c), im.getLineStep(), im.getROISize());
      }
    }
  }

  // ---- flippedCopyChannelROI (8u, 32f via 32s reinterpret) ----
  void ipp_flippedCopy(axis eAxis,
                       ImgBase& srcBase, int srcC, const Point& srcOffs, const Size& srcSize,
                       ImgBase& dstBase, int dstC, const Point& dstOffs, const Size& dstSize) {
    switch(srcBase.getDepth()) {
      case depth8u: {
        auto* src = srcBase.asImg<icl8u>();
        auto* dst = dstBase.asImg<icl8u>();
        ippiMirror_8u_C1R(src->getROIData(srcC, srcOffs), src->getLineStep(),
                          dst->getROIData(dstC, dstOffs), dst->getLineStep(),
                          srcSize, static_cast<IppiAxis>(eAxis));
        break;
      }
      case depth32f: {
        auto* src = srcBase.asImg<icl32f>();
        auto* dst = dstBase.asImg<icl32f>();
        ippiMirror_32s_C1R(reinterpret_cast<const Ipp32s*>(src->getROIData(srcC, srcOffs)),
                           src->getLineStep(),
                           reinterpret_cast<Ipp32s*>(dst->getROIData(dstC, dstOffs)),
                           dst->getLineStep(),
                           srcSize, static_cast<IppiAxis>(eAxis));
        break;
      }
      default: break;
    }
  }


  // ---- Registration ----

  // ---- channelMean ----
  icl64f ipp_channelMean(ImgBase& img, int channel, bool roiOnly) {
    const auto* data = roiOnly ? img.getROIData(channel) : img.getData(channel);
    int lineStep = img.getLineStep();
    Size size = roiOnly ? img.getROISize() : img.getSize();
    icl64f m = 0;
    switch(img.getDepth()) {
      case depth8u:
        ippiMean_8u_C1R(static_cast<const Ipp8u*>(data), lineStep, size, &m);
        break;
      case depth16s:
        ippiMean_16s_C1R(static_cast<const Ipp16s*>(data), lineStep, size, &m);
        break;
      case depth32f:
        ippiMean_32f_C1R(static_cast<const Ipp32f*>(data), lineStep, size, &m, ippAlgHintAccurate);
        break;
      default: break;
    }
    return m;
  }

  // ---- replicateBorder ----
  void ipp_replicateBorder(ImgBase& img) {
    for(int c = 0; c < img.getChannels(); ++c) {
      switch(img.getDepth()) {
        case depth8u:
          ippiCopyReplicateBorder_8u_C1IR(
            img.asImg<icl8u>()->getROIData(c), img.getLineStep(),
            img.getROISize(), img.getSize(),
            img.getROIOffset().x, img.getROIOffset().y);
          break;
        case depth32f:
          ippiCopyReplicateBorder_32f_C1IR(
            img.asImg<icl32f>()->getROIData(c), img.getLineStep(),
            img.getROISize(), img.getSize(),
            img.getROIOffset().x, img.getROIOffset().y);
          break;
        default: break;
      }
    }
  }

  // ---- Direct registration into ImgOps singleton ----

  static int _reg = [] {
    using Op = ImgOps::Op;
    auto& ops = ImgOps::instance();
    ops.getSelector<ImgOps::MirrorSig>(Op::mirror).add(Backend::Ipp, ipp_mirror, applicableToBase<icl8u, icl16s, icl32s, icl32f>, "IPP ippiMirror (8u/16s/32s/32f)");
    ops.getSelector<ImgOps::ClearChannelROISig>(Op::clearChannelROI).add(Backend::Ipp, ipp_clearChannelROI, applicableToBase<icl8u, icl16s, icl32s, icl32f>, "IPP ippiSet (8u/16s/32s/32f)");
    ops.getSelector<ImgOps::LutSig>(Op::lut).add(Backend::Ipp, ipp_lut, applicableToBase<icl8u>, "IPP ippiLUTPalette (8u)");
    ops.getSelector<ImgOps::GetMaxSig>(Op::getMax).add(Backend::Ipp, ipp_getMax, applicableToBase<icl8u, icl16s, icl32f>, "IPP ippiMax/MaxIndx (8u/16s/32f)");
    ops.getSelector<ImgOps::GetMinSig>(Op::getMin).add(Backend::Ipp, ipp_getMin, applicableToBase<icl8u, icl16s, icl32f>, "IPP ippiMin/MinIndx (8u/16s/32f)");
    ops.getSelector<ImgOps::GetMinMaxSig>(Op::getMinMax).add(Backend::Ipp, ipp_getMinMax, applicableToBase<icl8u, icl32f>, "IPP ippiMinMax/MinMaxIndx (8u/32f)");
    ops.getSelector<ImgOps::NormalizeSig>(Op::normalize).add(Backend::Ipp, ipp_normalize, applicableToBase<icl32f>, "IPP ippiMulC+AddC (32f)");
    ops.getSelector<ImgOps::FlippedCopySig>(Op::flippedCopy).add(Backend::Ipp, ipp_flippedCopy, applicableToBase<icl8u, icl32f>, "IPP ippiMirror (8u/32f)");
    ops.getSelector<ImgOps::ChannelMeanSig>(Op::channelMean).add(Backend::Ipp, ipp_channelMean, applicableToBase<icl8u, icl16s, icl32f>, "IPP ippiMean (8u/16s/32f)");
    ops.getSelector<ImgOps::ReplicateBorderSig>(Op::replicateBorder).add(Backend::Ipp, ipp_replicateBorder, applicableToBase<icl8u, icl32f>, "IPP ippiCopyReplicateBorder (8u/32f)");

} // anonymous namespace
