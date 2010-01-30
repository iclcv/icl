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
      inFileName = pa(0);
    }
  }else{
    inFileName = pa("-i");
  }

  if(!pa("-o")){
    if(!pa(1)) { 
      pausage("please define output filename"); 
      exit(-1); 
    }else{
      outFileName = pa(1);
    }
  }else{
    outFileName = pa("-o");
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
