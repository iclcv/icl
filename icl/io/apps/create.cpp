// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Quick2.h>
#include <icl/utils/ProgArg.h>
#include <icl/utils/StringUtils.h>
#include <icl/utils/Size32f.h>

int main(int n, char **ppc){
  pa_explain
  ("-p","specifies image to create.\n"
   "Also possible: first unspecified argument.\n"
   "One of house,tree,parrot,windows, or flowers")
  ("-o","specifies output image name (filetype be extension)\n"
   "Also pssible: 2nd unspecified argmunet.\n")
  ("-d","defines output image depth")
  ("-f","defines output image format")
  ("-s","defines output image size (incompatible to -scale)")
  ("-sc","defines output scale factor (incompatibel to -size)");
  pa_init(n,ppc,"-o|-output(filename) -pattern|-p(pattern=parrot) "
          "-size|-s(Size) -depth|-d(depth=depth8u) -format|-f(format=rgb) "
          "-scale|-sc(scalefactor=1.0)",true);


  if( pa("-p") && pa_get_count()){
    pa_show_usage("if patterns are defined using -p or -pattern, dangling arguments are not allowed");
    exit(-1);
  }
  std::string patternName = pa_get_count() ? *pa(0) : *pa("-p");

  if(pa_get_count() > 2){
    pa_show_usage("only two dangling arguments are allowed");
  }
  if(!pa("-o") && pa_get_count() < 2){
    pa_show_usage("-output or two dangling arguments are mandatory");
    exit(-1);
  }
  if(pa("-o") && pa_get_count() == 2){
    pa_show_usage("if output filename is defined using -o, dangling arguments are not allowed");
    exit(-1);
  }
  if(pa("-s") && pa("-sc")){
    pa_show_usage("arguments -size and -scale cannot be combined");
    exit(-1);
  }
  std::string outFileName = pa_get_count()==2 ? *pa(1) : *pa("-o");

  Image image = create(patternName);

  Size size = pa_def("-s",image.getSize());
  if(pa("-sc")){
    float factor = pa("-sc");
    size = Size(round(size.width*factor), round(size.height*factor));
  }
  format fmt = pa_def("-f", image.getFormat());
  ImgParams p(size, getChannelsOfFormat(fmt), fmt);
  save(fixed_convert(image, p, pa_def("-d",image.getDepth())), outFileName);

}
