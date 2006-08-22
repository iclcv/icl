#include "ImgBorder.h"

namespace icl{

  template<class T>
  void ImgBorder::fixed(Img<T> *im, T* val){
    FUNCTION_LOG("");
    ICLASSERT_RETURN( im );
    Rect roi = im->getROI();
    Size s = im->getSize();
     for(int c=0;c<im->getChannels();c++){
      // top
      clearChannelROI<T>(im,c,val[c], Point(0,roi.top()),
                         Size(s.width,s.height-roi.top()));
      // bottom
      clearChannelROI<T>(im,c,val[c], Point::zero,
                         Size(s.width,roi.bottom()));
      // left
      clearChannelROI<T>(im,c,val[c], Point(0,roi.bottom()),
                         Size(roi.left(),roi.height));
      // right
      clearChannelROI<T>(im,c,val[c], roi.lr(),
                         Size(s.width-roi.right(),roi.height) );

    }    
  }
  
  // export these functions
  template void ImgBorder::fixed<icl8u>(Img<icl8u> *im,icl8u *val);
  template void ImgBorder::fixed<icl32f>(Img<icl32f> *im,icl32f *val);

    
  template<class T>
  inline void _copy_border(Img<T> *poImage){
    Rect im = Rect(Point::zero,poImage->getSize());
    Rect roi = poImage->getROI();

    Rect aR[4] = { 
      Rect(Point(0,roi.ul().y), Size(roi.x,im.top()-roi.top())),                          // upper left
      Rect(Point(roi.ur().x,roi.ur().y),Size(im.right()-roi.right(),im.top()-roi.top())), // upper right
      Rect(Point(roi.lr().x,0),Size(im.right()-roi.right(),roi.y)),                       // lower right
      Rect(Point::zero,Size(roi.x,roi.y))                                                 // lower left
    };
    
    
    T aPix[4];
    for(int c=0;c<poImage->getChannels();c++)
      {      
        aPix[0] = (*poImage)(roi.ul().x,roi.ul().y,c); // upper left
        aPix[1] = (*poImage)(roi.ur().x,roi.ur().y,c); // upper right
        aPix[2] = (*poImage)(roi.lr().x,roi.lr().y,c); // lower right
        aPix[3] = (*poImage)(roi.ll().x,roi.ll().y,c); // lower left

        // clear the corners
        for(int i=0;i<4;i++)
          {
            clearChannelROI<T>(poImage,c,aPix[i],aR[i].ll(),aR[i].size());
          }
        
        // copy the borders
        // <left>
        Point srcOffs(roi.ll());
        Size srcDstSize(1,roi.height);
        for(Point p(0,roi.y);p.x!=roi.x;p.x++){
          deepCopyChannelROI(poImage,c, srcOffs, srcDstSize, poImage,c,p,srcDstSize);
        }
        
        // <right>
        srcOffs.x=roi.right();
        for(Point p(srcOffs.x+1,srcOffs.y);p.x!=im.right();p.y++){
          deepCopyChannelROI(poImage,c, srcOffs, srcDstSize, poImage,c,p,srcDstSize);
        }
        
        // <top>
        srcOffs = roi.ul();
        srcDstSize = Size(roi.width,1);
        for(Point p(roi.ul());p.y!=im.top();p.x++){
          deepCopyChannelROI(poImage,c, srcOffs, srcDstSize, poImage,c,p,srcDstSize);
        }
        
        // <bottom>
        srcOffs = Point(roi.ll());
        for(Point p(roi.ll());p.y!=-1;p.y--){
          deepCopyChannelROI(poImage,c, srcOffs, srcDstSize, poImage,c,p,srcDstSize);
        }
      }
  }
  
  void ImgBorder::copy(ImgI *poImage){
    FUNCTION_LOG("");
    ICLASSERT_RETURN(poImage);
    if(poImage->hasFullROI()) return;
    
    if(poImage->getDepth() == depth8u){
#ifdef WITH_IPP_OPTIMIZATION
    for(int c=0;c<poImage->getChannels();c++){
      ippiCopyReplicateBorder_8u_C1IR(poImage->asImg<icl8u>()->getROIData(c),poImage->getLineStep(),
                                      poImage->getROISize(),poImage->getSize(), 
                                      poImage->getROIOffset().x,poImage->getROIOffset().y);
    }
#else
    _copy_border(poImage->asImg<icl8u>());
#endif
  }else{
    _copy_border(poImage->asImg<icl32f>());
  }
}
  
  void ImgBorder::fromOther(ImgI *poImage, ImgI* poOther){
    FUNCTION_LOG("");
    ICLASSERT_RETURN( poImage && poOther );
    ICLASSERT_RETURN( poImage->getSize() ==  poOther->getSize() );
    printf("not yet implemented \n");
  }

}
