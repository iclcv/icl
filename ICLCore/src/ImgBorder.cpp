#include <ImgBorder.h>

namespace icl{

  template<class T>
  void ImgBorder::fixed(Img<T> *im, T* val){
    // {{{ open
  /** clear strategy:
              (top)
       tttttttttttttttttttttt
       tttttttttttttttttttttt
       lll.................rr
 (left)lll.....(image-.....rr(right)
       lll......ROI).......rr 
       lll.................rr
       bbbbbbbbbbbbbbbbbbbbbb
              (bottom)
  */
    FUNCTION_LOG("");
    ICLASSERT_RETURN( im );
    Rect roi = im->getROI();
    Size s = im->getSize();
     for(int c=0;c<im->getChannels();c++){
       // top
       clearChannelROI<T>(im,c,val[c], Point::zero,        
                          Size(s.width,roi.top()));
       // bottom
       clearChannelROI<T>(im,c,val[c], Point(0,roi.bottom()),
                          Size(s.width,s.height-roi.bottom()));
       // left
       clearChannelROI<T>(im,c,val[c], Point(0,roi.top()),
                          Size(roi.left(),roi.height));
       // right
       clearChannelROI<T>(im,c,val[c], roi.ur(),
                          Size(s.width-roi.right(),roi.height) );
     
    }    
  }
  // }}}
  
  // export these functions
  template void ImgBorder::fixed<icl8u>(Img<icl8u> *im,icl8u *val);
  template void ImgBorder::fixed<icl32f>(Img<icl32f> *im,icl32f *val);

  template<class T>
  inline void _copy_border(Img<T> *poImage){
    // {{{ open

    Rect im = Rect(Point::zero,poImage->getSize());
    Rect roi = poImage->getROI();
    
    Rect aR[4] = { 
      Rect(0,            0,             roi.x+1               ,roi.y+1),                  // upper left
      Rect(roi.right()-1,0,             im.width-roi.right()+1,roi.y+1),                  // upper right
      Rect(roi.right()-1,roi.bottom()-1,im.width-roi.right()+1,im.height-roi.bottom()+1), // lower right
      Rect(0,            roi.bottom()-1,roi.x+1,               im.height-roi.bottom()+1)  // lower left
    };
    
    T aPix[4];
    for(int c=0;c<poImage->getChannels();c++)
      {      
        aPix[0] = (*poImage)(roi.x,         roi.y,          c);// = 255; // upper left
        aPix[1] = (*poImage)(roi.right()-1, roi.y,          c);// = 255; // upper right
        aPix[2] = (*poImage)(roi.right()-1, roi.bottom()-1, c);// = 255; // lower right
        aPix[3] = (*poImage)(roi.x,         roi.bottom()-1, c);// = 255; // lower left

        // clear the corners
        for(int i=0;i<4;i++)
          {
            clearChannelROI<T>(poImage,c,aPix[i],aR[i].ul(),aR[i].size());
          }
        // copy the borders
        // <left>
        Point srcOffs;
        Size srcDstSize;

        srcOffs = Point(roi.x,roi.y+1);
        srcDstSize = Size(1,roi.height-2);
        if(roi.x>0){
          for(Point p(0,roi.y+1);p.x!=roi.x;p.x++){
            deepCopyChannelROI(poImage,c, srcOffs, srcDstSize, poImage,c,p,srcDstSize);
          }
        }
        
        
        // <right>
        srcOffs.x=roi.right()-1;
        if(roi.right() < im.right()){
          for(Point p(srcOffs.x+1,srcOffs.y);p.x<im.width;p.x++){
            deepCopyChannelROI(poImage,c, srcOffs, srcDstSize, poImage,c,p,srcDstSize);
          }
        }
        
        
        // <top>
        srcOffs = Point(roi.x+1,roi.y);
        srcDstSize = Size(roi.width-2,1);
        if(roi.y>0){
          for(Point p(srcOffs.x,roi.y+1);p.y>=0;p.y--){
            deepCopyChannelROI(poImage,c, srcOffs, srcDstSize, poImage,c,p,srcDstSize);
          }
        }
       
        
        // <bottom>
        if(roi.bottom()<im.bottom()){
          srcOffs.y = roi.bottom();
          for(Point p(srcOffs.x,roi.bottom()+1);p.y<im.height;p.y++){
            deepCopyChannelROI(poImage,c, srcOffs, srcDstSize, poImage,c,p,srcDstSize);
          }
        }
      }
  }
  // }}}
  
  void ImgBorder::copy(ImgBase *poImage){
    // {{{ open

 /** copy strategy:
              (top)
       tttttttttttttttttttttt
       tttttttttttttttttttttt
       lll.................rr
 (left)lll.....(image-.....rr(right)
       lll......ROI).......rr 
       lll.................rr
       bbbbbbbbbbbbbbbbbbbbbb
              (bottom)
  */
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
#ifdef WITH_IPP_OPTIMIZATION
      for(int c=0;c<poImage->getChannels();c++){ /// icl32f-case using Ipp32s method
        ippiCopyReplicateBorder_32s_C1IR((Ipp32s*)(poImage->asImg<icl8u>()->getROIData(c)),poImage->getLineStep(),
                                         poImage->getROISize(),poImage->getSize(), 
                                         poImage->getROIOffset().x,poImage->getROIOffset().y);
      }
#else
      _copy_border(poImage->asImg<icl32f>());
#endif
    }
  }

  // }}}
 
  void ImgBorder::fromOther(ImgBase *dst, ImgBase* src){
    // {{{ open

    FUNCTION_LOG("");
    ICLASSERT_RETURN( dst && src );
    ICLASSERT_RETURN( dst->isEqual(src->getSize(),src->getChannels()) );

    Rect roi = dst->getROI();
    Size s = dst->getSize();
    
    Point offs[4] = { 
      Point::zero,              // top
      Point(0,roi.bottom()),    // bottom
      Point(0,roi.top()),       // left
      roi.ur()                  // right
    };
    Size size[4] = {                
      Size(s.width,roi.top()),               // top
      Size(s.width,s.height-roi.bottom()),   // bottom
      Size(roi.left(),roi.height),           // left
      Size(s.width-roi.right(),roi.height)   // right
    };
    
    if(src->getDepth() == depth8u){
      if(dst->getDepth() == depth8u){
        for(int c=0;c<dst->getChannels();c++){
          for(int i=0;i<4;i++){
            deepCopyChannelROI<icl8u,icl8u>(src->asImg<icl8u>(),c,offs[i],size[i],dst->asImg<icl8u>(),c,offs[i],size[i]);
          }
        }
      }else{
        for(int c=0;c<dst->getChannels();c++){
          for(int i=0;i<4;i++){
            deepCopyChannelROI<icl8u,icl32f>(src->asImg<icl8u>(),c,offs[i],size[i],dst->asImg<icl32f>(),c,offs[i],size[i]);
          }
        }
      }
    }else{ // src is depth32f
      if(dst->getDepth() == depth8u){
        for(int c=0;c<dst->getChannels();c++){
          for(int i=0;i<4;i++){
            deepCopyChannelROI<icl32f,icl8u>(src->asImg<icl32f>(),c,offs[i],size[i],dst->asImg<icl8u>(),c,offs[i],size[i]);
          }
        }
      }else{
        for(int c=0;c<dst->getChannels();c++){
          for(int i=0;i<4;i++){
            deepCopyChannelROI<icl32f,icl32f>(src->asImg<icl32f>(),c,offs[i],size[i],dst->asImg<icl32f>(),c,offs[i],size[i]);
          }
        }
      }
    } 
  }

  // }}}

}
