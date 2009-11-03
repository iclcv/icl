#include <iclCommon.h>

#include <iclUnaryOp.h>
#include <iclConvolutionOp.h>
#include <iclMedianOp.h>
#include <iclMorphologicalOp.h>
#include <map>

GUI gui("hsplit");
int N = 0;

struct PassOp:public UnaryOp{
  void apply(const ImgBase *src, ImgBase **dst){
    ICLASSERT(dst);
    const_cast<ImgBase*>(src)->deepCopy(dst);
    (*dst)->setFullROI();
  }
  UnaryOp::apply;
};

std::map<std::string,UnaryOp*> filters; 

struct FiltersInitializer{
  FiltersInitializer(){

    filters["pass"] = new PassOp;

#define CO(x) filters[#x] = new ConvolutionOp(ConvolutionKernel::x)
    CO(gauss3x3);CO(gauss5x5);CO(sobelX3x3);CO(sobelX5x5);
    CO(sobelY3x3);CO(sobelY5x5);CO(laplace3x3);CO(laplace5x5);
#undef CO    
    
    filters["median3x3"] = new MedianOp(Size(3,3));
    filters["median5x5"] = new MedianOp(Size(5,5));
    filters["median11x11"] = new MedianOp(Size(11,11));
    
#define MO(x) filters[#x] = new MorphologicalOp(MorphologicalOp::x)
    MO(dilate);MO(erode);MO(openBorder);MO(closeBorder);
    MO(tophatBorder);MO(blackhatBorder);MO(gradientBorder);
#undef MO
    
    for(std::map<std::string,UnaryOp*>::const_iterator it=filters.begin();it != filters.end();++it){
      it->second->setClipToROI(false);
    }
  }
} filterInit;

std::string get_filters(){
  std::ostringstream str;
  for(std::map<std::string,UnaryOp*>::const_iterator it=filters.begin();it != filters.end();){
    str << it->first;
    ++it;
    if(it != filters.end()) str << ",";
  }
  return str.str();
}

GUI gui_col(int i){
  return ( GUI() 
           << ("image[@handle=im"+str(i)+"@minsize=8x6]")
           << ( GUI("hbox[@maxsize=100x3]")
                << ("combo("+get_filters()+")[@out=cb"+str(i)+"@maxsize=100x2@label=filter]") 
                << ("label(9999ms)[@label=dt@handle=dt"+str(i)+"@maxsize=100x2]")));
  
}

void init(){
  N = pa_subarg<int>("-n",0,3);
  
  gui << ( GUI("vbox") 
           << "image[@handle=input@minsize=8x6]"
           << "label(input image)[@maxsize=100x2@minsize=8x2]" );
  for(int i=0;i<N;++i){
    gui << gui_col(i);
  }

  gui.show();
}

void run(){
  static GenericGrabber g(FROM_PROGARG("-input"));
  g.setDesiredDepth(depth32f);
  g.setDesiredSize(parse<Size>(pa_subarg<std::string>("-size",0,"QVGA")));
  const ImgBase *image = g.grab();
  gui["input"] = image;
  gui["input"].update();
  for(int i=0;i<N;++i){
    Time t = Time::now();
    image = filters[gui["cb"+str(i)].as<std::string>()]->apply(image);
    gui["dt"+str(i)] = str((Time::now()-t).toMilliSeconds())+"ms";
    gui["im"+str(i)] = image;
    gui["im"+str(i)].update();
  }
}

int main(int n, char **ppc){
  pa_explain("-input","image source definition like -input dc 0");
  pa_explain("-n","number of filter instances in a row (3 by default)");
  
  return ICLApplication(n,ppc,"-input(2) -n(1) -size(1)",init,run).exec();
}

