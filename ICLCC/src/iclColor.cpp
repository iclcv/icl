#include "iclColor.h"
#include <map>
#include <ctype.h>
#include <iclStringUtils.h>

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
  }
  
  static const Color *create_named_color(std::string str){
    toLowerI(str);
    static ColorMap cm;
    ColorMap::iterator it = cm.find(str);
    if(it != cm.end()) return  &it->second;
    else return 0;
  }
  
  Color color_from_string(const std::string &name){
    const Color *col = create_named_color(name);
    if(col) return *col;
    return parse<Color>(name);
  }

 
 
}
