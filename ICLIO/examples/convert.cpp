#include <ICLIO/FileGrabber.h>
#include <ICLIO/FileWriter.h>
#include <ICLCC/FixedConverter.h>
#include <ICLQuick/Common.h>
#include <ICLUtils/Size32f.h>

int main(int n, char **ppc){
  pa_explain("-i","specify input file (type by filename extension)\n\t[also as 1st unspecified arg]");
  pa_explain("-o","specify output file (type by filename extension)\n\t[also as 2nd unspecified arg]");
  pa_explain("-depth","define output file depth");
  pa_explain("-format","define output file format");
  pa_explain("-size","define output file size");
  pa_explain("-scale","define size scaling factor");
  pa_explain("-scalemode", "defines scalemode to use (one of NN, LIN, or RA)");
  pa_init(n,ppc,"-i(1) -o(1) -depth(1) -format(1) -size(1) -scale(1) -scalemode(1)",true);
  
  std::string inFileName,outFileName;
  std::vector<std::string> dargs = pa_dangling_args();

  if(!pa_defined("-i")){
    if(dargs.size()){
      inFileName = dargs.front();
      dargs.erase(dargs.begin());
    }else{
      pa_usage("please define input format!");
      exit(-1);
    }
  }else{
    inFileName = pa_subarg<std::string>("-i",0,"");
  }
  if(!pa_defined("-o")){
    if(dargs.size()){
      outFileName = dargs.front();
      dargs.erase(dargs.begin());
    }else{
      pa_usage("please define input format!");
      exit(-1);
    }
  }else{
    outFileName = pa_subarg<std::string>("-o",0,"");
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
  
  format fmt = pa_defined("-format") ? parse<format>(pa_subarg<std::string>("-format",0,"")) : image->getFormat();
  int channels = image->getChannels();
  Size size = pa_defined("-size") ? parse<Size>(pa_subarg<std::string>("-size",0,"")) : image->getSize();
  if(pa_defined("-scale")){
    Size32f s32(size.width,size.height);
    s32 = s32 * pa_subarg<float>("-scale",0,1);
    size.width = round(s32.width);
    size.height = round(s32.height);
    
  }
  ImgParams p(size,
              channels,
              fmt);
  depth d = pa_defined("-depth") ? parse<depth>(pa_subarg<std::string>("-depth",0,"")) : image->getDepth();
              
  FixedConverter conv(p,d);
  if(pa_defined("-scalemode")){
    std::string sm = pa_subarg<std::string>("-scalemode",0,"NN");
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
