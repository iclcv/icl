#include <RunLengthEncoder.h>
#include <RegionDetectorTools.h>


namespace icl{

  using namespace region_detector_2_tools;
  
  RunLengthEncoder::RunLengthEncoder(const Size &sizeHint):
    m_data(sizeHint.getDim()),m_ends(sizeHint.height,(WLS*)0),m_imageSize(sizeHint){}
  
  void RunLengthEncoder::prepareForImageSize(const Size &size){
    if(size == m_imageSize) return;
    if(size.getDim() != m_imageSize.getDim()){
      m_data.resize(size.getDim());
    }
    if(size.height != (int)m_ends.size()){
      m_ends.resize(size.height,0);
    }
    m_imageSize = size;
  }

  void RunLengthEncoder::resetLineSegments(){
    for(int y=0;y<m_imageSize.height;++y){
      std::for_each(begin(y),end(y),std::mem_fun_ref(&WorkingLineSegment::reset));
    }
  }
  
  template<class T, bool useOffset>
  void RunLengthEncoder::encode_internal(const Img<T> &image, const Point &roiOffset){
    
  
    const int w = m_imageSize.width;
    const int h = m_imageSize.height;
  
    WLS *sldata(m_data.data()), *sls(0);

    const T *p = image.begin(0);
    const T *pEnd(0),*pLast(0),*pBegin(0);
    T curr(0);
    int yROI=0;
    
    for(int y=0;y<h;++y){
      pBegin = p;   // pixel pointer to current image line begin
      pEnd = p+w;   // pixel pointer to current image line end
      curr = *p++;  
      sls = sldata+w*y;
      pLast = p-1;
      if(useOffset){
        yROI = y+roiOffset.y;
      }
      while(p<pEnd){
        p = find_first_not(p,pEnd,curr);
        if(useOffset){
          sls->init( roiOffset.x + (int)(pLast-pBegin), yROI ,roiOffset.x + (int)(p-pBegin),curr); 
        }else{
          sls->init((int)(pLast-pBegin), y,(int)(p-pBegin),curr); 
        }
        ++sls;
        curr = *p; 
        pLast = p;
      }
      m_ends[y] = sls;
    }
  }
  
  void RunLengthEncoder::encode(const ImgBase *image, const Point &roiOffset){
    ICLASSERT_THROW(image,ICLException(" RunLengthEncoder::encode :image is NULL"));

    resetLineSegments();
    prepareForImageSize(image->getSize());
    
    switch(image->getDepth()){
#define ICL_INSTANTIATE_DEPTH(D) \
      case depth##D: \
      if(roiOffset==Point::null){                               \
        encode_internal<icl##D,false>(*image->asImg<icl##D>(),roiOffset); \
      }else{                                                    \
        encode_internal<icl##D,true>(*image->asImg<icl##D>(),roiOffset);  \
      }                                                         \
      break;
      ICL_INSTANTIATE_DEPTH(8u);
      ICL_INSTANTIATE_DEPTH(16s);
      ICL_INSTANTIATE_DEPTH(32s);
#undef ICL_INSTANTIATE_DEPTH
      default:
        ICL_INVALID_DEPTH;
    }
  }

}
