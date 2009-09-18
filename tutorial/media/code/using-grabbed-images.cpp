#include <iclGenericGrabber.h>
#include <iclProgArg.h>
#include <iclQuick.h>

using namespace icl;
// utility function
inline static void thresh_inplace(icl8u &p){
  p = 255*(p>127);
}
// utility function
inline static icl8u thresh(const icl8u &p){
  return 255*(p>127);
}

int main(int n, char **ppc){
  pa_init(n,ppc,"-input(2)");
  
  GenericGrabber g(FROM_PROGARG("-input"));
  g.setDesiredSize(Size::VGA);
  g.setDesiredDepth(depth8u);
  
// FIRST EXAMPLE:
  const ImgBase *i = g.grab();
  // now we dont have write access
  // but we can still use functions with
  // source destination fashion 
  Img8u dest(i->getParams());
  i->asImg<icl8u>()->transform(::thresh,dest);

  
  show(label(cvt(dest),"dest")); // from ICLQuick package

// 2nd EXAMPLE:
  Img8u *a  = g.grab()->convert<icl8u>();
  for(int x=0;x<a->getWidth();++x){
    for(int y=0;y<a->getHeight();++y){
      for(int c=0;c<a->getChannels();++c){
        (*a)(x,y,c) = 255 * ((*a)(x,y,c)>127);
      }
    }
  }
  show(label(cvt(a),"a")); // again ICLQuick !
  delete a;

// 3rd EXAMPLE:  
  // destination image, that must be adapted/
  // created by the grabber instance
  ImgBase *dst = 0;
  g.grab(&dst);

  // check if the grabber was able to exploit
  // given destination image
  ICLASSERT_RETURN_VAL(dst,-1); 
  
  // shallowly cast this image into it's
  // actual type Img8u
  Img8u &b = *dst->asImg<icl8u>();

  // now we have read/write access to b's pixels
  // so we can e.g. apply an inplace threshold
  b.forEach(::thresh_inplace);
  show(label(cvt(b),"b"));
  delete dst;

// 4th EXAMPLE:
  // or use ICLQuick directly
  show(label(icl::thresh(cvt(g.grab()),127),"quick"));
  
}
