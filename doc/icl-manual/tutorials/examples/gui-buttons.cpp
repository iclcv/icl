#include <ICLQt/Common.h>
#include <ICLFilter/MorphologicalOp.h>

HSplit gui;
GUI input;
GenericGrabber grabber;
MorphologicalOp op;
int currFilter = 0;

void init(){
  grabber.init(pa("-i"));
  gui << Display().handle("result")
      << ( VBox()
           << Button("next filter").handle("next")
           << Button("save").handle("save")
           << Button("show src").handle("show")
           )
      << Show();

  input << Display().handle("image") << Create();

  gui["show"].registerCallback(function(input,
                      &GUI::switchVisibility));
}

void run(){
  static ButtonHandle next = gui["next"];
  static ButtonHandle save = gui["save"];

  const ImgBase *image = grabber.grab();

  if(next.wasTriggered()){
    currFilter = (currFilter+1)%10;
    op.setOptype((MorphologicalOp::optype)currFilter);
  }

  static ImgBase *resultBuf = 0;
  op.apply(image, &resultBuf);

  gui["result"] = resultBuf;

  if(input.isVisible()){
    input["image"] = image;
  }

  if(save.wasTriggered()){
    qt::save(*resultBuf,"current-image.png");
  }
}
int main(int n, char **args){
   return ICLApp(n,args,"-input|-i(2)",init,run).exec();
}
