#include <ProximityOp.h>
#include <Img.h>

#ifdef WITH_IPP_OPTIMIZATION
namespace icl {

  namespace{

    template <typename T, IppStatus (IPP_DECL *ippiFunc) (const T*, int, IppiSize, const T*, int, IppiSize, icl32f*, int)>
    inline void ippiCall(const Img<T> *src1, const Img<T> *src2, Img32f *dst){
      // {{{ open
      for (int c=src1->getChannels()-1; c >= 0; --c) {
        ippiFunc (src1->getROIData (c), src1->getLineStep(),
                  src1->getROISize(),
                  src2->getROIData (c), src2->getLineStep(),
                  src2->getROISize(),
                  dst->getROIData (c), dst->getLineStep());
      }
    }
    // }}}
    
    template<class T, ProximityOp::optype ot, ProximityOp::applymode>
    struct ProximityOpTemplate{
      // {{{ open

      static void apply(const Img<T> *poSrc1,const Img<T> *poSrc2, Img32f *poDst){
        (void)poSrc1; (void)poSrc2; (void)poDst;
      }
    };

    // }}}
    
    // {{{ ProximityOpTemplate specialization (each calling a specific ipp function)
    /** Description: the following macro costructs a full template spcialization for
        the ProximityOpTemplate class. The parameters contain all necessary information
        for the ICL variable names as well as the IPP function name definition.
        
        the ipp function name, inserted into the ippCall function template above, 
        is constructed as follows:
        
        ippi<IPPOT_A><IPPAM>_<IPPOT_B>_<IPPDEPTH>_C1R(..)
        
        where 
        -<b>IPPOT_A</b>  is the first part of operation description:
                         "SqrDistance" or "CrossCorr"
        -<b>IPPAM  </b>  specifies the ipp apply mode
                         "Full", "Valid" or "Same"
        -<b>IPPOT_B</b>  is the second part of the operation description:
                         "Norm" or "NormLevel"
        -<b>IPPDEPTH</b> specifies ipps depth token
                         "32f" or "8u32f"
        
        the other macro parameters with ICL-prefix are used the build the correct ICL
        enum names, used the for the template specialization.
    */
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

    // }}}

    template<class T>
    void proximity_apply(const Img<T> *poSrc1, 
                         const Img<T> *poSrc2, 
                         Img32f *poDst, 
                         ProximityOp::optype ot, 
                         ProximityOp::applymode am){
      // {{{ open

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

    // }}}
    
  }// anonymous namespace
  
  void ProximityOp::apply(const ImgBase *poSrc1, const ImgBase *poSrc2, ImgBase **ppoDst){
    // {{{ open

    ICLASSERT_RETURN( poSrc1 && poSrc2 );
    ICLASSERT_RETURN( poSrc1->getChannels() == poSrc2->getChannels() );
    ICLASSERT_RETURN( poSrc1->getDepth() == poSrc2->getDepth() );
    
    if(poSrc1->getDepth() != depth8u && poSrc1->getDepth() != depth32f){
      poSrc1 = m_poImageBuffer = poSrc1->convert(m_poImageBuffer);      
      poSrc2 = m_poTemplateBuffer = poSrc2->convert(m_poTemplateBuffer);
    }
    
    
    /// set up dst image in depth, channel count and size
    ensureDepth(ppoDst,depth32f);

    (*ppoDst)->setChannels(poSrc1->getChannels());
    
    switch(m_eApplyMode){
      case full: (*ppoDst)->setSize(poSrc1->getSize()+poSrc2->getSize()-Size(1,1)); break;
      case same: (*ppoDst)->setSize(poSrc1->getSize()); break;
      case valid:(*ppoDst)->setSize(poSrc1->getSize()-poSrc2->getSize()+Size(1,1)); break;
    }

    switch(poSrc1->getDepth()){
      case depth8u:
        proximity_apply(poSrc1->asImg<icl8u>(),poSrc2->asImg<icl8u>(),(*ppoDst)->asImg<icl32f>(), m_eOpType, m_eApplyMode); 
        break;
      case depth32f:
        proximity_apply(poSrc1->asImg<icl32f>(),poSrc2->asImg<icl32f>(),(*ppoDst)->asImg<icl32f>(), m_eOpType, m_eApplyMode); 
        break;
      default:
        ICL_INVALID_DEPTH;
    }
  }

  // }}}
#endif
}
