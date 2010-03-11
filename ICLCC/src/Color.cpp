/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLCC module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#include <ICLCC/Color.h>
#include <map>
#include <ctype.h>
#include <ICLUtils/StringUtils.h>

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
