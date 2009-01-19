#include <iclTestImages.h>
#include <iclImg.h>
#include <iclQuick.h>

int main(){ 
  
  std::string names[]={"women","tree","house","windows","flowers","parrot"};
  bool horz[] = {1,1,1,0,0,1};
  ImgQ r(Size(1,1),formatRGB);
  for(int i=0;i<6;++i){
    ImgQ im = create(names[i]);
    if(horz[i]){
      r = (r,im);
    }else{
      r = (r%im);
    }
  }
  show(r);

  /** older (deprecated) version of this file
      ImgBase *a = TestImages::create("women");
      ImgBase *b = TestImages::create("tree");
      ImgBase *c = TestImages::create("house");
      
      ImgBase *d = TestImages::create("windows");
      ImgBase *e = TestImages::create("parrot");
      ImgBase *f = TestImages::create("flowers");
      
      
      TestImages::xv(a,"tmp1.ppm",500);
      TestImages::xv(b,"tmp2.ppm",500);
      TestImages::xv(c,"tmp3.ppm",500);
      TestImages::xv(d,"tmp4.ppm",500);
      TestImages::xv(e,"tmp5.ppm",500);
      TestImages::xv(f,"tmp6.ppm",500);
  */
}
