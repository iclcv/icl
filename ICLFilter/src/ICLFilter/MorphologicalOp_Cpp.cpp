#include <ICLFilter/MorphologicalOp.h>
#include <ICLCore/Img.h>
#include <ICLUtils/Range.h>
#include <ICLCore/ImgBorder.h>
#include <ICLFilter/BinaryArithmeticalOp.h>
#include <ICLCore/Image.h>
#include <functional>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;

namespace {

  using MOp = filter::MorphologicalOp;

  // ================================================================
  // C++ morphological kernel
  // ================================================================

  template<class T, typename cmp_func>
  void morph_cpp(const Img<T> &src, Img<T> &dst, MOp &op,
                 T init, cmp_func cmp, const icl8u *mask){
    const int srcW = src.getWidth(), dstW = dst.getWidth();
    const int roiW = dst.getROIWidth(), roiH = dst.getROIHeight();
    const int maskW = op.getMaskSize().width, maskH = op.getMaskSize().height;
    const Point anchor = op.getAnchor();
    const Point roiOff = op.getROIOffset();
    const T *srcData;
    T *dstROI;

    for(int c = 0; c < src.getChannels(); ++c){
      srcData = src.getData(c);
      dstROI = dst.getROIData(c);
      for(int y = 0; y < roiH; ++y){
        T *dstRow = dstROI + y * dstW;
        for(int x = 0; x < roiW; ++x){
          const icl8u *m = mask;
          T best = init;
          for(int my = 0; my < maskH; ++my){
            const T *row = srcData + (roiOff.y - anchor.y + y + my) * srcW
                           + (roiOff.x - anchor.x + x);
            for(int mx = 0; mx < maskW; ++mx, ++m){
              if(*m && cmp(row[mx], best)) best = row[mx];
            }
          }
          dstRow[x] = best;
        }
      }
    }
  }

  static Rect shrink_roi(Rect roi, const Size &maskSize){
    int dx = (maskSize.width-1)/2;
    int dy = (maskSize.height-1)/2;
    roi.x+=dx;
    roi.y+=dy;
    roi.width -= 2*dx;
    roi.height -= 2*dy;
    return roi;
  }

  template<class T>
  static void rep_border(Img<T> image, const Size &maskSize){
    Rect roi = shrink_roi(image.getImageRect(),maskSize);
    image.setROI(roi);
    ImgBorder::copy(&image);
  }

  // ================================================================
  // C++ typed apply — basic ops + composite ops via nested instances
  // ================================================================

  template<class T>
  void apply_t(const Image &srcImg, Image &dstImg, MOp &op){
    const Img<T> &src = srcImg.as<T>();
    Img<T> &dst = dstImg.as<T>();
    Range<T> limits = Range<T>::limits();
    Size sizeSave;
    std::vector<icl8u> maskSave;
    auto ot = op.getOptype();
    if(ot == MOp::dilate3x3 || ot == MOp::erode3x3){
      sizeSave = op.getMaskSize();
      std::copy(op.getMask(),op.getMask()+sizeSave.getDim(),back_inserter(maskSave));
      op.setMask(Size(3,3));
    }
    switch (ot){
      case MOp::dilate:
      case MOp::dilate3x3:
      case MOp::dilateBorderReplicate:
        morph_cpp(src,dst,op,limits.minVal,std::greater<T>(),op.getMask());
        break;
      case MOp::erode:
      case MOp::erode3x3:
      case MOp::erodeBorderReplicate:
        morph_cpp(src,dst,op,limits.maxVal,std::less<T>(),op.getMask());
        break;
      case MOp::tophatBorder:
      case MOp::blackhatBorder:{
        MOp sub_op(ot==MOp::tophatBorder ? MOp::openBorder : MOp::closeBorder,
                   op.getMaskSize(),op.getMask());
        sub_op.setClipToROI(op.getClipToROI());
        sub_op.setCheckOnly(op.getCheckOnly());
        sub_op.apply(srcImg, op.openingBuffer());
        filter::BinaryArithmeticalOp sub(filter::BinaryArithmeticalOp::subOp);
        sub.setClipToROI(op.getClipToROI());
        sub.setCheckOnly(op.getCheckOnly());

        Rect roi = srcImg.getROI();
        roi = shrink_roi(roi,op.getMaskSize());
        roi = shrink_roi(roi,op.getMaskSize());
        Image srcROIAdapted = srcImg.deepCopy();
        srcROIAdapted.setROI(roi);

        if(ot == MOp::tophatBorder){
          sub.apply(srcROIAdapted, op.openingBuffer(), dstImg);
        }else{
          sub.apply(op.openingBuffer(), srcROIAdapted, dstImg);
        }
        break;
      }
      case MOp::gradientBorder:{
        MOp sub_op(MOp::closeBorder,op.getMaskSize(),op.getMask());
        sub_op.setClipToROI(op.getClipToROI());
        sub_op.setCheckOnly(op.getCheckOnly());
        sub_op.apply(srcImg, op.gradientBuffer1());
        sub_op.setOptype(MOp::openBorder);
        sub_op.apply(srcImg, op.gradientBuffer2());
        filter::BinaryArithmeticalOp sub(filter::BinaryArithmeticalOp::subOp);
        sub.setClipToROI(op.getClipToROI());
        sub.setCheckOnly(op.getCheckOnly());

        sub.apply(op.gradientBuffer1(), op.gradientBuffer2(), dstImg);
        break;
      }
      case MOp::openBorder:
      case MOp::closeBorder:{
        MOp sub_op(ot==MOp::openBorder ? MOp::erode : MOp::dilate,
                   op.getMaskSize(),op.getMask());
        sub_op.setClipToROI(op.getClipToROI());
        sub_op.setCheckOnly(op.getCheckOnly());
        sub_op.apply(srcImg, op.openingBuffer());
        sub_op.setOptype(ot==MOp::openBorder ? MOp::dilate : MOp::erode);
        sub_op.apply(op.openingBuffer(), dstImg);
        break;
      }
      default:
        ERROR_LOG("invalid optype: " << static_cast<int>(ot));
    }
    if(!op.getClipToROI() && (ot == MOp::erodeBorderReplicate || ot == MOp::dilateBorderReplicate) ){
      rep_border(dst,op.getMaskSize());
    }else if (ot == MOp::erodeBorderReplicate || ot == MOp::dilateBorderReplicate){
      ERROR_LOG("border replication does not work if clipToROI is set [operation was applied, border replication was skipped]");
    }
    if(ot == MOp::dilate3x3 || ot == MOp::erode3x3){
      op.setMask(sizeSave,maskSave.data());
    }
  }

  void cpp_morph(const Image &src, Image &dst, MOp &op) {
    switch(src.getDepth()){
      case depth8u:  apply_t<icl8u>(src, dst, op); break;
      case depth32f: apply_t<icl32f>(src, dst, op); break;
      default: ICL_INVALID_DEPTH; break;
    }
  }

  static int _reg = [] {
    using Op = MOp::Op;
    auto& proto = MOp::prototype();
    proto.addBackend<MOp::MorphSig>(Op::apply, Backend::Cpp, cpp_morph,
                                     "C++ morphological ops");
    return 0;
  }();

} // anonymous namespace
