// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

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
    std::cout << "  static std::shared_ptr<Img8u> image;" << std::endl;
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
      out.send(image);
    }else{
      show(image);
    }
  }
}
