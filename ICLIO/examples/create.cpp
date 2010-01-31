#include <ICLQuick/Quick.h>
#include <ICLUtils/ProgArg.h>
#include <ICLUtils/StringUtils.h>
#include <ICLCC/FixedConverter.h>
#include <ICLIO/FileWriter.h>
#include <ICLUtils/Size32f.h>

int main(int n, char **ppc){
  paex
  ("-p","specifies image to create.\n"
   "Also possible: first unspecified argument.\n"
   "One of house,tree,parrot,windows, or flowers")
  ("-o","specifies output image name (filetype be extension)\n"
   "Also pssible: 2nd unspecified argmunet.\n")
  ("-d","defines output image depth")
  ("-f","defines output image format")
  ("-s","defines output image size (incompatible to -scale)")
  ("-sc","defines output scale factor (incompatibel to -size)");
  painit(n,ppc,"-o|-output(filename) -pattern|-p(pattern=parrot) "
         "-size|-s(Size) -depth|-d(depth=depth8u) -format|-f(format=rgb) "
         "-scale|-sc(scalefactor=1.0)",true);

  
  std::string patternName,outFileName;

  if(pa("-p") && pacount()){
    pausage("if patterns are defined using -p or -pattern, dangling arguments are not allowed");
    exit(-1);
  }
  patternName = pacount() ? pa(0) : pa("-p");

  if(pacount() > 2){
    pausage("only two dangling arguments are allowed");
  }  
  if(!pa("-o") && pacount() < 2){
    pausage("-output or two dangling arguments are mandatory");
    exit(-1);
  }
  if(pa("-o") && pacount() == 2){
    pausage("if output filename is defined using -o, dangling arguments are not allowed");
    exit(-1);
  }
  if(pa("-s") && pa("-sc")){
    pausage("arguments -size and -scale cannot be combined");
    exit(-1);
  }
  outFileName = pacount()<2 ? pa(1) : pa("-o");
  
  ImgBase *image = new ImgQ(create(patternName));
  
  format fmt = pa("-f");
  int channels = image->getChannels();
  Size size = padef("-s",image->getSize());
  if(pa("-sc")){
    Size32f s32(size.width,size.height);
    s32 = s32 * (float)pa("-sc");
    size.width = round(s32.width);
    size.height = round(s32.height);
  }
  ImgParams p(size,channels,fmt);
  depth d = padef("-d",image->getDepth());
              
  FixedConverter conv(p,d);
  
  ImgBase *dst = 0;
  conv.apply(image,&dst);
  
  FileWriter fw(outFileName);
  fw.write(dst);
  
}
