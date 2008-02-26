#include "iclUsefulFunctions.h"
#include <iclProximityOp.h>

namespace icl{

  namespace{
    template<int N>
    inline void apply_inplace_threshold(Img8u image,int dim, icl8u thresh){
      icl8u *pc[N];
      for(int j=0;j<N;++j){
        pc[j] = image.getData(j);
      }
      
      for(int i=0;i<dim;i++){
        for(int j=0;j<N;++j){
          pc[0][i] &= pc[j][0] > thresh;
        }
      }          
    }
  }
  
  std::vector<Rect> iclMatchTemplate(const Img8u &src, 
                                     const Img8u &templ, 
                                     float significance,
                                     Img8u *bufferGiven,
                                     ImgRegionDetector *rdGiven){    
    ImgBase *useBuffer = bufferGiven ? bufferGiven : new Img8u;
    ImgRegionDetector *useRD = rdGiven ? rdGiven : new ImgRegionDetector;

    ProximityOp po(ProximityOp::crossCorrCoeff,ProximityOp::valid);
    po.apply(&src,&templ,&useBuffer);
    
    // now search for all maxima in the image that are within the 
    // given significance
    
    Img8u &m = *useBuffer->asImg<icl8u>();
    icl8u t = float(255)*significance;
    
    switch(m.getChannels()){
      case 1: 
        apply_inplace_threshold<1>(m,m.getDim(),t); 
        break;
      case 2: 
        apply_inplace_threshold<2>(m,m.getDim(),t); 
        break;
      case 3: 
        apply_inplace_threshold<3>(m,m.getDim(),t); 
        break;
      default: 
        ERROR_LOG("this function is only supported for 1,2 and 3 channel images");
        return std::vector<Rect>();
    }

    const ImgBase *c0 = useBuffer->shallowCopy(useBuffer->getImageRect(),
                                               std::vector<int>(1,0),
                                               formatMatrix);
    useRD->setRestrictions(0,2<<30,1,255);
    const std::vector<BlobData> &blobData = useRD->detect(c0);

    
    std::vector<Rect> resultVec(blobData.size());
    Rect templRect(Point::null,templ.getROISize());
    for(unsigned int i=0;i<blobData.size();i++){
      Point c =blobData[i].getCOG();
      
      resultVec[i] = (templRect+c) & src.getImageRect();
    }
    
    
    delete c0;
    if(!rdGiven) delete useRD;
    if(!bufferGiven) delete useBuffer;
       
  }
                                     
  
  
}

