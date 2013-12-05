/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/apps/create/create.cpp                           **
** Module : ICLIO                                                  **
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

#include <ICLQt/Quick.h>
#include <ICLUtils/ProgArg.h>
#include <ICLUtils/StringUtils.h>
#include <ICLCore/FixedConverter.h>
#include <ICLIO/FileWriter.h>
#include <ICLUtils/Size32f.h>

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

  std::string patternName,outFileName;

  
  if( pa("-p") && pa_get_count()){
    pa_show_usage("if patterns are defined using -p or -pattern, dangling arguments are not allowed");
    exit(-1);
  }
  patternName = pa_get_count() ? *pa(0) : *pa("-p");

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
  outFileName = pa_get_count()==2 ? *pa(1) : *pa("-o");
  
  ImgBase *image = new ImgQ(create(patternName));
  
  format fmt = pa("-f");
  int channels = image->getChannels();
  Size size = pa_def("-s",image->getSize());
  if(pa("-sc")){
    Size32f s32(size.width,size.height);
    s32 = s32 * (float)pa("-sc");
    size.width = round(s32.width);
    size.height = round(s32.height);
  }
  ImgParams p(size,channels,fmt);
  depth d = pa_def("-d",image->getDepth());
              
  FixedConverter conv(p,d);
  
  ImgBase *dst = 0;
  conv.apply(image,&dst);
  
  FileWriter fw(outFileName);
  fw.write(dst);
  
}
