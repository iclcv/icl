#include <iclImageSplitter.h>
#include <math.h>

using std::vector;

namespace icl{
  
  //      10------------------------50
  //n=4   10     20     30    40    50
  //
  
  void ImageSplitter::splitImage(const ImgBase *src, vector<const ImgBase*> &parts){
    Rect r = src->getROI();
    int n = (int)parts.size();
    ICLASSERT_RETURN(n);
    ICLASSERT_RETURN(r.getDim());
    ICLASSERT_RETURN(r.height >= n);

    float dh = float(r.height)/n;
    vector<int> hs;
    for(int i=0;i<n;++i){
      int yStart = r.y+(int)round(dh*i);
      int yEnd = r.y+(int)round(dh*(i+1));
      int dy = yEnd - yStart;
      parts[i] = src->shallowCopy(Rect(r.x,yStart,r.width,dy));
    }      
  }
  
  
  class ImageSplitterImpl{
  public:
    ImageSplitterImpl(const ImgBase *src, int n){
      ICLASSERT_RETURN(src);
      m_vecParts.resize(n);
      ImageSplitter::splitImage(src,m_vecParts);
    }
    ~ImageSplitterImpl(){
      for(int i=0;i<getPartCount();++i){
        delete m_vecParts[i];
      }
    }
    int getPartCount(){
      return (int)m_vecParts.size();
    }
    const ImgBase *get(int idx){
      return m_vecParts[idx];
    }
  private:
    vector<const ImgBase*> m_vecParts;
  };
  
  void ImageSplitterImplDelOp::delete_func(ImageSplitterImpl *impl){
    ICL_DELETE(impl);
  }
  


  ImageSplitter::ImageSplitter():
    ShallowCopyable<ImageSplitterImpl,ImageSplitterImplDelOp>(0){
  }
  ImageSplitter::ImageSplitter(const ImgBase *src, int n):
    ShallowCopyable<ImageSplitterImpl,ImageSplitterImplDelOp>(new ImageSplitterImpl(src,n)){
  }
  
  int ImageSplitter::getPartCount(){
    ICLASSERT_RETURN_VAL(!isNull(),0);
    return impl->getPartCount();
  }
  
  const ImgBase *ImageSplitter::operator[](int idx){
    ICLASSERT_RETURN_VAL(!isNull(),0);
    ICLASSERT_RETURN_VAL(idx < impl->getPartCount(),0);
    return impl->get(idx);
  }
}

