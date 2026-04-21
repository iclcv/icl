// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/qt/Common2.h>
#include <icl/filter/UnaryOp.h>

HSplit gui;
int N = 0;

GenericGrabber g;

std::string get_filters(){
  return cat(UnaryOp::listFromStringOps(),",");
}

GUI gui_col(int i){
  std::string si = str(i);
  return ( VBox()
           << Display().handle("im"+si).minSize(8,6)
           << ( HBox().maxSize(100,3)
                << Combo(get_filters()).handle("cb"+si).maxSize(100,2).minSize(3,2).label("filter")
                << String("").label("params").handle("ps"+si).maxSize(100,2).minSize(4,2)
                << CheckBox("vis",true).maxSize(3,2).out("vis"+si).minSize(3,2)
                )
           << ( HBox().maxSize(100,3)
                << Label("ok").handle("err"+si).maxSize(100,2).label("error").minSize(1,2)
                << Label("9999ms").label("dt").handle("dt"+si).size(2,3)
                << Label("unknown").handle("syn"+si).maxSize(100,2).label("syntax").minSize(1,2)
                )
           );
}

void init(){
  N = pa("-n");

  gui << ( VBox()
           << Display().handle("input").minSize(8,6)
           << (HBox().label("desired params").maxSize(100,4)
               << Combo("1:1,QVGA,VGA,SVGA,XGA,WXGA,UXGA").handle("dsize").label("size")
               << Combo("!depth8u,depth16s,depth32s,depth32f,depth64f").handle("ddepth").label("size")
               << Combo("gray,!rgb,hls,lab,yuv").handle("dformat").label("format")
               )
           );

  for(int i=0;i<N;++i){
    gui << gui_col(i);
  }

  g.init(pa("-i"));
  gui << Show();
}

void run(){
  g.useDesired(parse<depth>(gui["ddepth"]));
  if(gui["dsize"].as<std::string>() == "1:1"){
    g.useDesired(Size::null);
  }else{
    g.useDesired(parse<Size>(gui["dsize"]));
  }
  g.useDesired(parse<format>(gui["dformat"]));

  Image image = g.grabImage();
  gui["input"] = image;
  std::vector<std::unique_ptr<UnaryOp>> ops;
  for(int i=0;i<N;++i){
    Time t = Time::now();
    std::string si = str(i);
    std::string opName = gui["cb"+si].as<std::string>();
    std::string params = gui["ps"+si].as<std::string>();

    gui["syn"+si] = UnaryOp::getFromStringSyntax(opName);
    try{
      ops.emplace_back(UnaryOp::fromString(params.size() ? (opName+"("+params+")") : opName));
      auto &op = ops.back();
      op->setClipToROI(false);
      gui["err"+si] = str("ok");

      if(image && gui["vis"+si].as<bool>()){
        image = op->apply(image);
      }
    }catch(const ICLException &ex){
      gui["err"+si] = str(ex.what());
    }
    gui["dt"+si] = str((Time::now()-t).toMilliSeconds())+"ms";
    gui["im"+si] = image;
  }
}

int main(int n, char **ppc){
  pa_explain("-input","image source definition like -input dc 0")
            ("-n","number of filter instances in a row (3 by default)");

  return ICLApplication(n,ppc,"[m]-input|-i(2) -num-filters|-n(int=3)",init,run).exec();
}
