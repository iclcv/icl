#include <ICLQuick/Quick.h>
#include <ICLUtils/ProgArg.h>
#include <ICLUtils/StringUtils.h>
#include <ICLCC/FixedConverter.h>
#include <ICLIO/FileWriter.h>
#include <ICLUtils/Size32f.h>

int main(int n, char **ppc){
  pa_init(n,ppc,"-o(1) -size(1) -depth(1) -format(1)",true);
  
  pa_explain("-p","specify image pattern to create\n"
             "[also as 1st unspecified argument]\n\tone of:\n"
             "\t- house\n"
             "\t- tree\n"
             "\t- parrot\n"
             "\t- windows\n"
             "\t- flowers");
  pa_explain("-o","specify output file (type by filename extension)\n\t[also as 2nd unspecified arg]");
  pa_explain("-depth","define output file depth");
  pa_explain("-format","define output file format");
  pa_explain("-size","define output file size");
  pa_explain("-scale","define scale factor");
  pa_init(n,ppc,"-i(1) -o(1) -depth(1) -format(1) -size(1) -scale(1)",true);
  
  std::string patternName,outFileName;
  std::vector<std::string> dargs = pa_dangling_args();

  if(!pa_defined("-p")){
    if(dargs.size()){
      patternName = dargs.front();
      dargs.erase(dargs.begin());
    }else{
      pa_usage("please define input pattern!");
      exit(-1);
    }
  }else{
    patternName = pa_subarg<std::string>("-p",0,"");
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

  ImgBase *image = new ImgQ(create(patternName));
  
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
  
  ImgBase *dst = 0;
  conv.apply(image,&dst);
  
  FileWriter fw(outFileName);
  fw.write(dst);
  
}
