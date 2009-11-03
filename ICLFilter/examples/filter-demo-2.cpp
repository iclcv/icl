#include <iclCommon.h>

#include <iclUnaryOp.h>
#include <iclConvolutionOp.h>
#include <iclMedianOp.h>
#include <iclMorphologicalOp.h>
#include <map>

GUI gui("hsplit");
int N = 0;



std::string get_filters(){
  return cat(UnaryOp::listFromStringOps(),",");
}

GUI gui_col(int i){
  return ( GUI() 
           << ("image[@handle=im"+str(i)+"@minsize=8x6]")
           << ( GUI("hbox[@maxsize=100x3]")
                << ("combo("+get_filters()+")[@out=cb"+str(i)+"@maxsize=100x2@label=filter]") 
                << ("label(9999ms)[@label=dt@handle=dt"+str(i)+"@maxsize=100x2]")
                << ("string()[@label=params@handle=ps"+str(i)+"@maxsize=100x2@out=_"+str(i)+"]"))
           << ("label(ok)[@handle=err"+str(i)+"@maxsize=100x2@label=error]") 
           << ("label(unknown)[@handle=syn"+str(i)+"@maxsize=100x2@label=syntax]") );
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
  std::vector<UnaryOp*> ops;
  for(int i=0;i<N;++i){
    Time t = Time::now();
    std::string si = str(i);
    std::string opName = gui["cb"+si].as<std::string>();
    std::string params = gui["ps"+si].as<std::string>();
    
    UnaryOp *op = 0;
    gui["syn"+si] = UnaryOp::getFromStringSyntax(opName);
    try{
      op = UnaryOp::fromString(params.size() ? (opName+"("+params+")") : opName);
      op->setClipToROI(false);
      ops.push_back(op);
      gui["err"+si] = str("ok"); 
    }catch(const ICLException &ex){
      gui["err"+si] = str(ex.what());
    }
    if(op && image){
      image = op->apply(image);
      gui["dt"+si] = str((Time::now()-t).toMilliSeconds())+"ms";
      gui["im"+si] = image;
      gui["im"+si].update();
    }
  }
  for(unsigned int i=0;i<ops.size();++i){
    delete ops[i];
  }
}

int main(int n, char **ppc){
  pa_explain("-input","image source definition like -input dc 0");
  pa_explain("-n","number of filter instances in a row (3 by default)");
  
  return ICLApplication(n,ppc,"-input(2) -n(1) -size(1)",init,run).exec();
}

