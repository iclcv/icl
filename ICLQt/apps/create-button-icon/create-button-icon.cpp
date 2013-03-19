/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/apps/create-button-icon/create-button-icon.cpp   **
** Module : ICLQt                                                  **
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
#include <ICLQt/IconFactory.h>
#include <ICLIO/GenericImageOutput.h>

int main(int n, char **a){
  pa_explain("-o","output filename (should be some format, that supports alpha channel such as png)\n"
             "or, if no output is given, the image is just show");
  pa_init(n,a,"-icon-name|-i(iconname=empty) -output|-o(2) -image-file-to-c++-array|-ita(input-file-name)");
  
  if(pa("-ita")){
    Img8u image = load<icl8u>(pa("-ita"));
    const int w=image.getWidth(), h = image.getHeight(), c = image.getChannels();
    std::cout << "static icl8u data_XYZ["<<w<<"]["<<h<<"]["<<c<<"]={" << std::endl; 
    for(int y=0;y<h;++y){
      std::cout << "{";
      for(int x=0;x<w;++x){
        std::cout << "{";
        for(int i=0;i<c;++i){
          std::cout << (int)image(x,y,i) << (i<c-1?",":"}");
        }
        std::cout << (x<w-1?",":"}");
      }
      std::cout << (y<h-1?",":"};") << std::endl;
    }
    std::cout << "//#include <ICLCore/Img.h>" << std::endl;
    std::cout << "//#include <ICLCC/CCFunctions.h>" << std::endl;
    std::cout << "const Img8u& load_data_XYZ(){" << std::endl;
    std::cout << "  static SmartPtr<Img8u> image;" << std::endl;
    std::cout << "  if(!image){" << std::endl;
    std::cout << "     image = new Img8u(Size(" << w << "," << h << ")," << c << ");" << std::endl;
    std::cout << "     interleavedToPlanar((const icl8u*)data_XYZ, image.get());" << std::endl;
    std::cout << "  }" << std::endl;
    std::cout << "  return *image;" << std::endl;
    std::cout << "}" << std::endl;
  }else{
    const Img8u &image = IconFactory::create_image(pa("-i"));
    
    if(pa("-o")){
      GenericImageOutput out(pa("-o"));
      out.send(&image);
    }else{
      show(image);
    }
  }
}
