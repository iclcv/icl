#include "iclColor.h"
#include <map>
#include <ctype.h>
#include <algorithm>

namespace icl{
  
  namespace{
    struct ColorMap : public std::map<std::string,Color>{
      ColorMap(){
        ColorMap &t = *this;
        t["black"] = Color(0,0,0);
        t["white"] = Color(255,255,255);

        t["red"] = Color(255,0,0);
        t["green"] = Color(0,255,0);
        t["blue"] = Color(0,0,255);


        t["cyan"] = Color(0,255,255);
        t["magenta"] = Color(255,0,255);
        t["yellow"] = Color(255,255,0);

        t["gray200"] = Color(200,200,200);
        t["gray150"] = Color(150,150,150);
        t["gray100"] = Color(100,100,100);
        t["gray50"] = Color(50,50,50);
      }
    };
    void to_lower2(char &c){
      c = tolower(c);
    }
  }
  
  const Color &iclCreateColor(std::string str){
    std::for_each(str.begin(),str.end(),to_lower2);
    static ColorMap cm;
    ColorMap::iterator it = cm.find(str);
    if(it != cm.end()) return  it->second;
    else{
      static Color &black = cm["black"];
      return black;
    }  
  }
  

  Color translateColor(const std::string &s){
    if(s.size()<2){
      ERROR_LOG("unable to translate " << s << " into GeneralColor type");
      return Color(icl8u(0));
    }
    Color color;
    std::vector<icl8u> v = icl::parseVecStr<icl8u>(s.substr(1,s.length()-2),",");
    std::copy(v.begin(),v.begin()+iclMin(3,(int)v.size()),color.data());
    return color;
  }

  std::string translateColor(const Color &color){
    std::ostringstream s; 
    s << '(';
    for(unsigned int i=0;i<color.dim()-1;++i){
      s << (int)color[i] << ',';
    }
    s << (int)color[color.dim()-1] << ')';
    return s.str();
  }

 
}
