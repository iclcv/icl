#include <ICLMarkers/MarkerMetricsICL1.h>
#include <ICLUtils/Exception.h>
#include <ICLUtils/StringUtils.h>

namespace icl{
  
  MarkerMetricsICL1::MarkerMetricsICL1(const MarkerCodeICL1 &c, const Size32f &markerSizeMM):
    MarkerCodeICL1(c),root(markerSizeMM){

    const float u = root.width/13.0;
    const float v = root.height/17.0;
    
    for(int i=0;i<4;++i){
      CR &cr = crs[i];
      cr.x = u;
      cr.y = v * (i*4+1);
      cr.width = 11*u;
      cr.height = 3*v;
      cr.ccrs.resize(c[i]);

      float ccry = (i*4+2)*v;
      for(int j=0;j<c[i];++j){
        Rect32f &ccr = cr.ccrs[j];

        switch(c[i]){
          case 1: ccr.x = 6*u; break;
          case 2: ccr.x = (2+j*8)*u; break;
          case 3: ccr.x = (2+j*4)*u; break;
          case 4: ccr.x = (j<2?(2+j*2):(4+j*2))*u; break;
          case 5: ccr.x = (2+j*2)*u; break;
          default:
            throw ICLException(":MarkerMetricsICL1: invalid code found! " + str(c));
        }
        ccr.y = ccry;
        ccr.width = u;
        ccr.height = v;
      }
    }
  }
}
