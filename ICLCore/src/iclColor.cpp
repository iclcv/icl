#include "iclColor.h"
#include <map>
#include <ctype.h>
#include <algorithm>

namespace icl{
  
  namespace{
    struct ColorMap : public std::map<std::string,GeneralColor<icl8u,3> >{
      ColorMap(){
        ColorMap &t = *this;
        t["black"] = GeneralColor<icl8u,3>(0,0,0);
        t["white"] = GeneralColor<icl8u,3>(255,255,255);

        t["red"] = GeneralColor<icl8u,3>(255,0,0);
        t["green"] = GeneralColor<icl8u,3>(0,255,0);
        t["blue"] = GeneralColor<icl8u,3>(0,0,255);


        t["cyan"] = GeneralColor<icl8u,3>(0,255,255);
        t["magenta"] = GeneralColor<icl8u,3>(255,0,255);
        t["yellow"] = GeneralColor<icl8u,3>(255,255,0);

        t["gray200"] = GeneralColor<icl8u,3>(200,200,200);
        t["gray150"] = GeneralColor<icl8u,3>(150,150,150);
        t["gray100"] = GeneralColor<icl8u,3>(100,100,100);
        t["gray50"] = GeneralColor<icl8u,3>(50,50,50);
      }
    };
    void to_lower2(char &c){
      c = tolower(c);
    }
  }


  
  const GeneralColor<icl8u,3> &iclCreateColor(std::string str){
    std::for_each(str.begin(),str.end(),to_lower2);
    static ColorMap cm;
    ColorMap::iterator it = cm.find(str);
    if(it != cm.end()) return  it->second;
    else{
      static GeneralColor<icl8u,3> &black = cm["black"];
      return black;
    }  
  }
  
 
}
