#include <ICLQt/Common.h>

// top-level component is a "horizontal split"
HSplit gui;
GenericGrabber grabber;

void init(){

  grabber.init(pa("-i"));

  gui << Image().handle("image")
      << ( VBox().minSize(10,26).maxSize(14,100)
           << ( VBox().label("thresholds")
                << Slider(0,255,0).label("red").handle("r")
                << Slider(0,255,0).label("green").handle("g")
                << Slider(0,255,0).label("blue").handle("b")
                )
           << ( HBox()
                << Button("save")
                << Button("load")
              )
           << Combo("input,output").label("visualization")
           << ( VBox().label("options")
                << CheckBox("preprocessing")
                << CheckBox("open mp")
                << CheckBox("fast approx.")
                << Spinner(1,5,2).label("num threads")
                )
           << CamCfg()
           )
      << Show();

}

void run(){
  gui["image"] = grabber.grab();
  
  // image processing stuff ...
  
}

int main(int n, char **args){
  return ICLApp(n, args, "-input|-i(2)", init,run).exec();
}
