/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/apps/create-marker-grid-svg/                **
**          create-marker-grid-svg.cpp                             **
** Module : ICLMarkers                                             **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLQt/Common.h>
#include <ICLMarkers/FiducialDetector.h>
#include <fstream>

std::string toBase64(const void *data, size_t len){
  static const std::string lut = ("ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                  "abcdefghijklmnopqrstuvwxyz"
                                  "0123456789+/");

  const icl8u *src = (icl8u*)data;
  std::ostringstream ret;
  int i = 0;
  int j = 0;
  unsigned char c3[3];
  unsigned char c4[4];
  
  while (len--) {
    c3[i++] = *(src++);
    if (i == 3) {
      c4[0] = (c3[0] & 0xfc) >> 2;
      c4[1] = ((c3[0] & 0x03) << 4) + ((c3[1] & 0xf0) >> 4);
      c4[2] = ((c3[1] & 0x0f) << 2) + ((c3[2] & 0xc0) >> 6);
      c4[3] = c3[2] & 0x3f;
      
      for(i = 0; (i <4) ; i++)
        ret << lut[c4[i]];
      i = 0;
    }
  }

  if (i){
    for(j = i; j < 3; j++){
      c3[j] = '\0';
    }
    
    c4[0] = (c3[0] & 0xfc) >> 2;
    c4[1] = ((c3[0] & 0x03) << 4) + ((c3[1] & 0xf0) >> 4);
    c4[2] = ((c3[1] & 0x0f) << 2) + ((c3[2] & 0xc0) >> 6);
    c4[3] = c3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++){
      ret << lut[c4[j]];
    }

    while((i++ < 3)){
      ret << '=';
    }

  }

  return ret.str();
}


std::string getPngDataBase64(const Img8u &m){
  std::string fn="/tmp/image.png";
  save(m,fn);
  std::string data = File::read_file(fn,false);
  return toBase64(data.c_str(), data.length());
}


std::string create_marker_text(const Rect32f &r, int id){
  static FiducialDetector fd("bch", Any(),ParamList("size","1x1"));
  core::Img8u m = fd.createMarker(id, Size(300,300), ParamList("border width",2));
  std::ostringstream str;
  str << "<image x=\"" << r.x << "mm\" y=\"" << r.y << "mm\" id=\"marker_"
      << id << "\" style=\"image-rendering:optimizeSpeed\" "
      << "preserveAspectRatio=\"none\" width=\"" << r.width
      << "mm\" height=\"" << r.height << "mm\" xlink:href=\""
      << "data:image/png;base64," << getPngDataBase64(m) << "\"/>" << std::endl;
  return str.str();
}

int main(int n, char **ppc){
  pa_explain("-m","margin between markers if <=0, a 10th of the marker-dim is used"); 
  pa_init(n,ppc,
          "-offset|-ofs(x=12,y=12) "
          "-size|-s(size=30x21) "
          "-output|-o(filename) "
          "-marker-dim|-d(dim=30mm) "
          "-margin|-m(margin=-1) "
          "-marker-resolution|-r(size=300x300)");
  float ox = pa("-ofs",0);
  float oy = pa("-ofs",1);
  float nx = pa("-s").as<Size32f>().width;
  float ny = pa("-s").as<Size32f>().height;
  float md = pa("-d");
  float m = pa("-m");
  if(m < 0){
    m = md/10;
  }
  
  std::ofstream out((*pa("-o")).c_str());

  std::string encoding = ( "<?xml version=\"1.0\" encoding=\"UTF-8\" "
                           "standalone=\"no\"?>");
  std::string svg =  ( "<svg version=\"1.1\" id=\"svg2\" "
                       "width=\"297mm\" height=\"210mm\" "
                       "xmlns:xlink=\"http://www.w3.org/1999/xlink\">" );
  std::string group = "<g transform=\"translate(0,0)\" id=\"layer1\">";
  
  out << encoding << std::endl
      << svg << std::endl
      << group << std::endl;
  
  int idx = -1;
  for(int y=0;y<ny;++y){
    for(int x=0;x<nx;++x){
      Rect32f bounds(ox+x*(md+m), oy+y*(md+m), md, md);
      out << create_marker_text(bounds, ++idx);
    }
  }
  out << "</g> </svg>";
}
