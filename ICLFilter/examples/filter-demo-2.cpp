/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLFilter/examples/filter-demo-2.cpp                   **
** Module : ICLFilter                                              **
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

#include <ICLQuick/Common.h>
#include <ICLFilter/UnaryOp.h>
#include <ICLFilter/ConvolutionOp.h>
#include <ICLFilter/MedianOp.h>
#include <ICLFilter/MorphologicalOp.h>

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
                << ("combo("+get_filters()+")[@out=cb"+str(i)+"@maxsize=100x2@minsize=3x2@label=filter]") 
                << ("string()[@label=params@handle=ps"+str(i)+"@maxsize=100x2@out=_"+str(i)+"@minsize=4x2]")
                << ("checkbox(vis,checked)[@maxsize=3x2@out=vis"+str(i)+"@minsize=3x2]"))
           << ( GUI("hbox[@maxsize=100x3]")
                << ("label(ok)[@handle=err"+str(i)+"@maxsize=100x2@label=error@minsize=1x2]") 
                << ("label(9999ms)[@label=dt@handle=dt"+str(i)+"@size=2x3]"))
           << ("label(unknown)[@handle=syn"+str(i)+"@maxsize=100x2@label=syntax@minsize=1x2]") );
}

void init(){
  N = pa("-n");
  
  gui << ( GUI("vbox") 
           << "image[@handle=input@minsize=8x6]"
           << (GUI("hbox[@label=desired params@maxsize=100x4]") 
               << "combo(QVGA,!VGA,SVGA,XGA,WXGA,UXGA)[@out=_dsize@handle=dsize@label=size]"
               << "combo(!depth8u,depth16s,depth32s,depth32f,depth64f)[@out=_ddepeth@handle=ddepth@label=size]"
               << "combo(gray,!rgb,hls,lab,yuv)[@out=_dformat@handle=dformat@label=format]"));
  for(int i=0;i<N;++i){
    gui << gui_col(i);
  }

  gui.show();
}

void run(){
  static GenericGrabber g(FROM_PROGARG("-input"));
  g.setDesiredDepth(parse<depth>(gui["ddepth"].as<std::string>()));
  g.setDesiredSize(parse<Size>(gui["dsize"].as<std::string>()));
  g.setDesiredFormat(parse<format>(gui["dformat"].as<std::string>()));
  
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
    if(op && image && gui["vis"+si].as<bool>()){
      try{
        image = op->apply(image);
      }catch(const ICLException &ex){
        gui["err"+si] = str(ex.what());
      }
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
  paex
  ("-input","image source definition like -input dc 0")
  ("-n","number of filter instances in a row");
  return ICLApplication(n,ppc,"-input|-i(device,device-params) "
                        "-n-filters|-n(int=3)",init,run).exec();
}

