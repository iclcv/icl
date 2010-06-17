/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/examples/convert.cpp                             **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLIO/FileGrabber.h>
#include <ICLIO/FileWriter.h>
#include <ICLCC/FixedConverter.h>
#include <ICLQuick/Common.h>
#include <ICLUtils/Size32f.h>

int main(int n, char **ppc){
  paex("-i","specify input file (type by filename extension)\n\t[also as 1st unspecified arg]")
      ("-o","specify output file (type by filename extension)\n\t[also as 2nd unspecified arg]")
      ("-depth","define output file depth")
      ("-format","define output file format")
      ("-size","define output file size")
      ("-scale","define size scaling factor")
      ("-scalemode", "defines scalemode to use (one of NN, LIN, or RA)");
  painit(n,ppc,"-input|-i(filename) -output|-o(filename) -depth|-d(depth) -format|-f(format) -size|-s(Size) -scale(factor) -scalemode(scalemode)",true);
  
  std::string inFileName,outFileName;

  if(!pa("-i")){
    if(!pa(0)) { 
      pausage("please define input filename"); 
      exit(-1); 
    }else{
      inFileName = *pa(0);
    }
  }else{
    inFileName = *pa("-i");
  }

  if(!pa("-o")){
    if(!pa(1)) { 
      pausage("please define output filename"); 
      exit(-1); 
    }else{
      outFileName = *pa(1);
    }
  }else{
    outFileName = *pa("-o");
  }


  FileGrabber fg(inFileName);
  fg.setIgnoreDesiredParams(true);

  const ImgBase *image = 0;
  try{
    image = fg.grab();
  }catch(ICLException &ex){
    ERROR_LOG("unable to grab file:" << ex.what());
    exit(-1);
  }
  if(!image){
    ERROR_LOG("unable to grab file!");
    exit(-1);
  }

  //ImgParams(const Size &size, int channels, format fmt, const Rect& roi = Rect::null)
  
  format fmt = pa("-format") ? parse<format>(pa("-format")) : image->getFormat();
  int channels = image->getChannels();
  Size size = pa("-size") ? parse<Size>(pa("-size")) : image->getSize();
  if(pa("-scale")){
    Size32f s32(size.width,size.height);
    s32 = s32 * parse<float>(pa("-scale"));
    size.width = round(s32.width);
    size.height = round(s32.height);
    
  }
  ImgParams p(size,
              channels,
              fmt);
  depth d = pa("-depth") ? parse<depth>(pa("-depth")) : image->getDepth();
              
  FixedConverter conv(p,d);
  if(pa("-scalemode")){
    std::string sm = pa("-scalemode");
    if(sm == "NN"){
      conv.setScaleMode(interpolateNN);
    }else if(sm == "LIN"){
      conv.setScaleMode(interpolateLIN);
    }else if(sm == "RA"){
      conv.setScaleMode(interpolateRA);
    }else{
      ERROR_LOG("unknown scale mode (allowed modes are NN, LIN and RA)");
      return -1;
    }
  }
  
  ImgBase *dst = 0;
  conv.apply(image,&dst);
  
  FileWriter fw(outFileName);
  fw.write(dst);
}
