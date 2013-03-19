/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/apps/convert/convert.cpp                         **
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

#include <ICLIO/FileGrabber.h>
#include <ICLIO/FileWriter.h>
#include <ICLCore/FixedConverter.h>
#include <ICLQt/Common.h>
#include <ICLUtils/Size32f.h>
#include <ICLFilter/RotateOp.h>

int main(int n, char **ppc){
  pa_explain("-i","specify input file (type by filename extension)\n\t[also as 1st unspecified arg]")
      ("-o","specify output file (type by filename extension)\n\t[also as 2nd unspecified arg]")
      ("-depth","define output file depth")
      ("-format","define output file format")
      ("-size","define output file size")
      ("-rotate","rotate angle by given angle (clock-wise in deg)")
      ("-scale","define size scaling factor")
      ("-flip","flips the image (allowed values are horz, vert, or both")
      ("-scalemode", "defines scalemode to use (one of NN, LIN, or RA)");
  pa_init(n,ppc,"-input|-i(filename) -output|-o(filename) -depth|-d(depth) -format|-f(format) -size|-s(Size) -scale(factor) -scalemode(scalemode) -rotate(angle) -flip(axis)",true);
  
  std::string inFileName,outFileName;

  if(!pa("-i")){
    if(!pa(0)) { 
      pa_show_usage("please define input filename"); 
      exit(-1); 
    }else{
      inFileName = *pa(0);
    }
  }else{
    inFileName = *pa("-i");
  }

  if(!pa("-o")){
    if(!pa(1)) { 
      pa_show_usage("please define output filename"); 
      exit(-1); 
    }else{
      outFileName = *pa(1);
    }
  }else{
    outFileName = *pa("-o");
  }


  FileGrabber fg(inFileName);

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
  int channels = pa("-format") ? getChannelsOfFormat(fmt) : image->getChannels();
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
  scalemode sm = interpolateLIN;
  
  if(pa("-scalemode")){
    std::string sm = pa("-scalemode");
    if(sm == "NN"){
      sm = interpolateNN;
    }else if(sm == "LIN"){
      sm = interpolateLIN;
    }else if(sm == "RA"){
      sm = interpolateRA;
    }else{
      ERROR_LOG("unknown scale mode (allowed modes are NN, LIN and RA)");
      return -1;
    }
  }
  conv.setScaleMode(sm);
  
  ImgBase *dst = 0;
  conv.apply(image,&dst);
  
  if(pa("-flip")){
    std::string axis = pa("-flip");
    core::axis a = axisHorz;
    if(axis == "horz"){}
    else if(axis == "vert") a = axisVert;
    else if(axis == "both") a = axisBoth;
    else {
      ERROR_LOG("invalied flip axis (allowed is 'horz', 'vert' or 'both', but got " << axis << ")" );
      return -1;
    }
    dst->mirror(a);
  }       
  if(pa("-rotate")){
    float angle = pa("-rotate");
    static RotateOp rot(angle,sm);
    static ImgBase *dst2 = 0;
    rot.apply(dst,&dst2);
    dst = dst2;
  }
  
  FileWriter fw(outFileName);
  fw.write(dst);
}
