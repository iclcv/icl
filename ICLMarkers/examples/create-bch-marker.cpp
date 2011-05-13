#include <ICLQuick/Common.h>
#include <ICLMarkers/BCHCode.h>

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
