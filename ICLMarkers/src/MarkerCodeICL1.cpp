#include <ICLMarkers/MarkerCodeICL1.h>
#include <ICLUtils/Macros.h>

#include <algorithm>

namespace icl{
  
  const int MarkerCodeICL1::P;

  MarkerCodeICL1::MarkerCodeICL1(icl8u rootColor):id(-1),rootColor(rootColor){
    n[0] = n[1] = n[2] = n[3] = 0;
  }

  MarkerCodeICL1::MarkerCodeICL1(const int ns[4], icl8u rootColor):rootColor(rootColor){
    for(int i=0;i<4;++i) n[i] = ns[i];
    id = n[0] + P*n[1] + (P*P)*n[2] + (P*P*P)*n[3];
  }

  MarkerCodeICL1::MarkerCodeICL1(int id, icl8u rootColor):id(id),rootColor(rootColor){
    n[3] = iclMin(P,id/(P*P*P));
    id -= n[3]*(P*P*P);
    n[2] = iclMin(P,id/(P*P));
    id -= n[2]*(P*P);
    n[1] = iclMin(P,id/(P));
    id -= n[1]*(P);
    n[0] = id;
  }

  int MarkerCodeICL1::operator-(const MarkerCodeICL1 &p) const {
    int e = 0;
    for(int i=0;i<4;++i) e += ::abs(n[i]+p.n[i]);
    return e;
  }


  std::ostream &operator<<(std::ostream &str, const MarkerCodeICL1 &c){
    return str << "("<<c[0]<<","<<c[1]<<","<<c[2]<<","<<c[3]<<")["<<c.id<<"]";
  }

  static bool hamming_ok(const MarkerCodeICL1 &a, const std::vector<MarkerCodeICL1> &ps){
    for(unsigned int x=0;x<ps.size();++x){
      if( (a - ps[x]) < 2){
        return false;
      }
    }
    return true;
  }
  
  const std::vector<MarkerCodeICL1> &MarkerCodeICL1::generate(){
    static std::vector<MarkerCodeICL1> ps;
    if(ps.size()) return ps;
    
    for(int i=0;i<=P;++i){
      for(int j=0;j<=P;++j){
        for(int k=0;k<=P;++k){
          for(int l=0;l<=P;++l){
            if(i<=j && j<=k && k<=l && (i!=l || j!=k)){
              int ns[]={i,j,k,l};
              MarkerCodeICL1 t(ns);
              if(hamming_ok(t, ps)){
                ps.push_back(t);
              }
            }
          }
        }
      }
    }
    std::reverse(ps.begin(),ps.end());
    return ps;
  }
  


}
