/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/src/ICLFilter/ProximityOp.cpp                **
** Module : ICLFilter                                              **
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

#include <ICLFilter/ProximityOp.h>
#include <ICLCore/CoreFunctions.h>
#include <ICLCore/Img.h>
#include <ICLUtils/StringUtils.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl {

// ippiSqrDistanceFull/Same/Valid_Norm_* and ippiCrossCorrFull/Same/Valid_Norm_*
// removed from modern IPP (oneAPI 2022+). TODO: update to ippiSqrDistanceNorm_*.
#if 0 // was: ICL_HAVE_IPP — proximity Full/Same/Valid variants removed
  namespace utils{
    template<> inline std::string str(const filter::ProximityOp::optype &t){
      return (t == filter::ProximityOp::sqrDistance ? "sqrDistance" :
              t == filter::ProximityOp::crossCorr ? "crossCorr" :
              "crossCorrCoeff");
    }

    template<> inline filter::ProximityOp::optype parse(const std::string &s){
      return (s == "sqrDistance" ? filter::ProximityOp::sqrDistance :
                s == "crossCorr" ? filter::ProximityOp::crossCorr :
              filter::ProximityOp::crossCorrCoeff);
    }
    template<> std::string str(const filter::ProximityOp::applymode &t){
      return (t == filter::ProximityOp::full ? "full" :
              t == filter::ProximityOp::valid ? "valid" :
              "same");
    }

    template<> filter::ProximityOp::applymode parse(const std::string &s){
      return (s == "full" ? filter::ProximityOp::full :
              s == "valid" ? filter::ProximityOp::valid :
              filter::ProximityOp::same);
    }
  }

  namespace filter{
    ProximityOp::ProximityOp(optype ot, applymode am):
      m_poImageBuffer(0),m_poTemplateBuffer(0){
      addProperty("operation type","menu","sqrDistance,crossCorr,crossCorrCoeff",ot,0,
                  "Proximity measurement type (square distance, cross correlation,\n"
                  "and cross correlation coefficient)");
      addProperty("apply mode","menu","full,valid,same",am,0,
                  "Defines on what part of the input image the proximity\n"
                  "measurement is applied:\n"
                  "'full':  means, the images are compared in every configuration,\n"
                  "         where the two images have at least one pixel overlap.\n"
                  "         (the result image becomes larger)\n"
                  "'same':  the pattern is centered at every pixel of the\n"
                  "         source image. (The result image size is identical to\n"
                  "'valid': the pattern is only matched agains the source\n"
                  "         image where the full pattern fits into it.\n"
                  "         (The result image becomes smaller than the\n"
                  "         source image");
    }

    ProximityOp::~ProximityOp(){
      delete m_poImageBuffer;
      delete m_poTemplateBuffer;
    }

    void ProximityOp::setOpType(optype ot){
      setPropertyValue("operation type",ot);
    }

    void ProximityOp::setApplyMode(applymode am){
      setPropertyValue("apply mode",am);
    }

    ProximityOp::optype ProximityOp::getOpType() const{
      return const_cast<ProximityOp*>(this)->getPropertyValue("operation type");
    }

    ProximityOp::applymode ProximityOp::getApplyMode() const{
      return const_cast<ProximityOp*>(this)->getPropertyValue("apply mode");
    }

    namespace{

      template <typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, IppiSize, const T*, int, IppiSize, icl32f*, int)>
      inline void ippiCall(const Img<T> *src1, const Img<T> *src2, Img32f *dst){
        for (int c=src1->getChannels()-1; c >= 0; --c) {
          ippiFunc (src1->getROIData (c), src1->getLineStep(),
                    src1->getROISize(),
                    src2->getROIData (c), src2->getLineStep(),
                    src2->getROISize(),
                    dst->getROIData (c), dst->getLineStep());
        }
      }

      template<class T, ProximityOp::optype ot, ProximityOp::applymode>
      struct ProximityOpTemplate{
        static void apply(const Img<T> *poSrc1,const Img<T> *poSrc2, Img32f *poDst){
          (void)poSrc1; (void)poSrc2; (void)poDst;
        }
      };

  #define CREATE_TEMPLATE(ICLDEPTH,ICLOT,ICLAM,IPPOT_A,IPPAM,IPPOT_B,IPPDEPTH)                           \
      template<> struct ProximityOpTemplate<icl##ICLDEPTH,ProximityOp::ICLOT,ProximityOp::ICLAM> {       \
        static void apply(const Img<icl##ICLDEPTH> *src1,const Img<icl##ICLDEPTH> *src2, Img32f *dst){   \
          ippiCall<icl##ICLDEPTH,ippi##IPPOT_A##IPPAM##_##IPPOT_B##_##IPPDEPTH##_C1R>(src1,src2,dst);    \
        }                                                                                                \
      }

  #define CREATE_TEMPLATE_ALL_AM(ICLDEPTH,ICLOT,IPPOT_A,IPPOT_B,IPPDEPTH)   \
      CREATE_TEMPLATE(ICLDEPTH,ICLOT,full,IPPOT_A,Full,IPPOT_B,IPPDEPTH);   \
      CREATE_TEMPLATE(ICLDEPTH,ICLOT,same,IPPOT_A,Same,IPPOT_B,IPPDEPTH);   \
      CREATE_TEMPLATE(ICLDEPTH,ICLOT,valid,IPPOT_A,Valid,IPPOT_B,IPPDEPTH)

      CREATE_TEMPLATE_ALL_AM(8u,sqrDistance,SqrDistance,Norm,8u32f);
      CREATE_TEMPLATE_ALL_AM(32f,sqrDistance,SqrDistance,Norm,32f);

      CREATE_TEMPLATE_ALL_AM(8u,crossCorr,CrossCorr,Norm,8u32f);
      CREATE_TEMPLATE_ALL_AM(32f,crossCorr,CrossCorr,Norm,32f);

      CREATE_TEMPLATE_ALL_AM(8u,crossCorrCoeff,CrossCorr,NormLevel,8u32f);
      CREATE_TEMPLATE_ALL_AM(32f,crossCorrCoeff,CrossCorr,NormLevel,32f);

  #undef CREATE_TEMPLATE
  #undef CREATE_TEMPLATE_ALL_AM

      template<class T>
      void proximity_apply(const Img<T> *poSrc1,
                           const Img<T> *poSrc2,
                           Img32f *poDst,
                           ProximityOp::optype ot,
                           ProximityOp::applymode am){
        switch(ot){
          case ProximityOp::sqrDistance:
            switch(am){
              case ProximityOp::full:
                ProximityOpTemplate<T,ProximityOp::sqrDistance,ProximityOp::full>::apply(poSrc1,poSrc2,poDst);
                break;
              case ProximityOp::same:
                ProximityOpTemplate<T,ProximityOp::sqrDistance,ProximityOp::same>::apply(poSrc1,poSrc2,poDst);
                break;
              case ProximityOp::valid:
                ProximityOpTemplate<T,ProximityOp::sqrDistance,ProximityOp::valid>::apply(poSrc1,poSrc2,poDst);
                break;
            }
            break;
          case ProximityOp::crossCorr:
            switch(am){
              case ProximityOp::full:
                ProximityOpTemplate<T,ProximityOp::crossCorr,ProximityOp::full>::apply(poSrc1,poSrc2,poDst);
                break;
              case ProximityOp::same:
                ProximityOpTemplate<T,ProximityOp::crossCorr,ProximityOp::same>::apply(poSrc1,poSrc2,poDst);
                break;
              case ProximityOp::valid:
                ProximityOpTemplate<T,ProximityOp::crossCorr,ProximityOp::valid>::apply(poSrc1,poSrc2,poDst);
                break;
            }
            break;
          case ProximityOp::crossCorrCoeff:
            switch(am){
              case ProximityOp::full:
                ProximityOpTemplate<T,ProximityOp::crossCorrCoeff,ProximityOp::full>::apply(poSrc1,poSrc2,poDst);
                break;
              case ProximityOp::same:
                ProximityOpTemplate<T,ProximityOp::crossCorrCoeff,ProximityOp::same>::apply(poSrc1,poSrc2,poDst);
                break;
              case ProximityOp::valid:
                ProximityOpTemplate<T,ProximityOp::crossCorrCoeff,ProximityOp::valid>::apply(poSrc1,poSrc2,poDst);
                break;
            }
            break;
        }
      }

    }// anonymous namespace

    void ProximityOp::apply(const Image &src1, const Image &src2, Image &dst){
      ICLASSERT_RETURN( !src1.isNull() && !src2.isNull() );
      ICLASSERT_RETURN( src1.getChannels() == src2.getChannels() );
      ICLASSERT_RETURN( src1.getDepth() == src2.getDepth() );

      const ImgBase *p1 = src1.ptr(), *p2 = src2.ptr();

      if(p1->getDepth() != depth8u && p1->getDepth() != depth32f){
        p1 = m_poImageBuffer = p1->convert(m_poImageBuffer);
        p2 = m_poTemplateBuffer = p2->convert(m_poTemplateBuffer);
      }

      applymode am = getPropertyValue("apply mode");
      optype ot = getPropertyValue("operation type");

      Size dstSize;
      switch(am){
        case full:  dstSize = p1->getSize() + p2->getSize() - Size(1,1); break;
        case same:  dstSize = p1->getSize(); break;
        case valid: dstSize = p1->getSize() - p2->getSize() + Size(1,1); break;
      }

      prepare(dst, depth32f, dstSize, formatMatrix, p1->getChannels(),
              Rect(Point::null, dstSize));

      switch(p1->getDepth()){
        case depth8u:
          proximity_apply(p1->asImg<icl8u>(), p2->asImg<icl8u>(),
                          dst.ptr()->asImg<icl32f>(), ot, am);
          break;
        case depth32f:
          proximity_apply(p1->asImg<icl32f>(), p2->asImg<icl32f>(),
                          dst.ptr()->asImg<icl32f>(), ot, am);
          break;
        default:
          ICL_INVALID_DEPTH;
      }
    }

#endif // ICL_HAVE_IPP (disabled — old proximity APIs removed from modern IPP)

} // namespace icl
