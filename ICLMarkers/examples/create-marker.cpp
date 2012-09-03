#include <ICLQt/Common.h>
#include <ICLMarkers/FiducialDetector.h>

int main(int n, char **ppc){
  pa_explain
  ("-i","the first sub-argument defines the marker type (one of bch, icl1 and art). The 2nd sub-argument "
   "defines which marker to create (this is the marker ID in case of type bch and icl1 and "
   "in case of makrer type art, an image filename is expected")
  ("-b","this is only allowed for bch markers (the detectors default value is 2)")
  ("-r","this is only allowed for art markers (the detectors default is value is 0.4)")
  ("-o","optionally given output filename. If this is not given, the marker image is shown instead")
  ("-s","output size of the marker image");
  

  pa_init(n,ppc,"[m]-id|-i(type,int) -border-size|-b(int=2) "
          "-border-ration|-r(float=0.4) -output|-o(filename) "
          "-size|-s(size=300x300)");

  
  FiducialDetector d(*pa("-i"));
  ParamList params;
  if(*pa("-i") == "art"){
    params = ParamList("border ratio",*pa("-r"));
  }else if(*pa("-i") == "bch"){
    params = ParamList("border width",*pa("-b"));
  }
  Img8u image = d.createMarker(*pa("-i",1), pa("-s"), params);

  if(pa("-o")){
    save(image,*pa("-o"));
  }else{
    show(image);
  }

}


#if 0
int main(int n, char **ppc){
  painit(n,ppc,"-id|-i(int=0) -border|-b(int=2) -output|-o(filename) -show -size|-s(size=0x0)");
  Img8u m = create_bch_marker_image(pa("-i"),pa("-b"),pa("-s"));
  if(pa("-o")){
    save(m,*pa("-o"));
  }
  if(pa("-show")){
    m.scale(Size(500,500));
    show(m);
  }
}
#endif
