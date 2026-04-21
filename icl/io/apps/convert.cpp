// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/io/FileGrabber.h>
#include <icl/io/FileWriter.h>
#include <icl/core/FixedConverter.h>
#include <icl/qt/Common2.h>
#include <icl/utils/Size.h>
#include <icl/filter/RotateOp.h>

using namespace icl;
using namespace icl::utils;
using namespace icl::core;
using namespace icl::io;
using namespace icl::filter;

int main(int n, char **ppc){
  pa_explain("-i","specify input file (type by filename extension)\n\t[also as 1st unspecified arg]")
      ("-o","specify output file (type by filename extension)\n\t[also as 2nd unspecified arg]\t %D in filename will be replaced by the image time HHMMSSZZZ")
      ("-depth","define output file depth")
      ("-format","define output file format")
      ("-size","define output file size")
      ("-rotate","rotate angle by given angle (clock-wise in deg)")
      ("-scale","define size scaling factor")
      ("-crop","define crop-rect x y w h")
      ("-flip","flips the image (allowed values are horz, vert, or both")
      ("-scalemode", "defines scalemode to use (one of NN, LIN, or RA)");
  pa_init(n,ppc,"-input|-i(filename) -output|-o(filename) -depth|-d(depth) "
          "-format|-f(format) -c|-clip|-crop(x,y,width,height) -size|-s(Size) -scale(factor) -scalemode(scalemode) -rotate(angle) -flip(axis)",true);

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

  Image image;
  try{
    image = fg.grabImage();
  }catch(ICLException &ex){
    ERROR_LOG("unable to grab file:" << ex.what());
    exit(-1);
  }
  if(!image){
    ERROR_LOG("unable to grab file!");
    exit(-1);
  }

	std::string time_string = image.getTime().toStringFormated("%H%M%S%#",32,true);
	std::string::size_type pos = outFileName.find("%D");
	if (pos != std::string::npos) {
		outFileName.replace(pos,2,time_string);
	}

  //ImgParams(const Size &size, int channels, format fmt, const Rect& roi = Rect::null)

  format fmt = pa("-format") ? pa("-format").as<format>() : image.getFormat();
  int channels = pa("-format") ? getChannelsOfFormat(fmt) : image.getChannels();
  Size size = pa("-size") ? pa("-size").as<Size>() : image.getSize();
  if(pa("-scale")){
    Size32f s32(size.width,size.height);
    s32 = s32 * pa("-scale").as<float>();
    size.width = round(s32.width);
    size.height = round(s32.height);

  }
  ImgParams p(size,
              channels,
              fmt);
  depth d = pa("-depth") ? pa("-depth").as<depth>() : image.getDepth();

  FixedConverter conv(p,d);
  scalemode sm = interpolateLIN;

  if(pa("-scalemode")){
    const std::string s = pa("-scalemode").as<std::string>();
    if(s == "NN"){
      sm = interpolateNN;
    }else if(s == "LIN"){
      sm = interpolateLIN;
    }else if(s == "RA"){
      sm = interpolateRA;
    }else{
      ERROR_LOG("unknown scale mode (allowed modes are NN, LIN and RA)");
      return -1;
    }
  }
  conv.setScaleMode(sm);

  // One-shot tool — no need for static buffers. Image() owns each
  // intermediate ImgBase via shared_ptr and releases on reassignment.
  ImgBase *converted = nullptr;
  conv.apply(image.ptr(), &converted);
  Image dst(converted);

  if(pa("-flip")){
    const std::string axis = pa("-flip").as<std::string>();
    core::axis a;
    if(axis == "horz")      a = axisHorz;
    else if(axis == "vert") a = axisVert;
    else if(axis == "both") a = axisBoth;
    else {
      ERROR_LOG("invalid flip axis (allowed is 'horz', 'vert' or 'both', but got " << axis << ")");
      return -1;
    }
    dst.ptr()->mirror(a);
  }

  if(pa("-rotate")){
    RotateOp rot(pa("-rotate").as<float>(), sm);
    ImgBase *rotated = nullptr;
    rot.apply(dst.ptr(), &rotated);
    dst = Image(rotated);
  }

  if(pa("-c")){
    const Rect r(pa("-c",0), pa("-c",1), pa("-c",2), pa("-c",3));
    if((dst.getImageRect() | r) != dst.getImageRect()){
      ERROR_LOG("wrong option -crop|-clip: given rectangle was "
                << r << " but the image rect is only " << dst.getImageRect());
      return -1;
    }
    dst.ptr()->setROI(r);
    ImgBase *cropped = nullptr;
    dst.ptr()->deepCopyROI(&cropped);
    dst = Image(cropped);
  }

  FileWriter(outFileName).write(dst.ptr());
  return 0;
}
