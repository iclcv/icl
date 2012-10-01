#include <ICLQt/Common.h>
#include <ICLFilter/MorphologicalOp.h>

HSplit gui;
GUI input;
GenericGrabber grabber;
MorphologicalOp op;
int currFilter = 0;

Mutex resultMutex;
ImgBase *result = 0;

void saveImage(){
  try{
    const std::string &filename = saveFileDialog();
    Mutex::Locker lock(resultMutex);
    qt::save(*result, filename);
  }catch(...){}
}

void init(){
  grabber.init(pa("-i"));
  gui << Image().handle("result") 
      << ( VBox() 
           << Button("next filter").handle("next")
           << Button("save").handle("save")
           << Button("show src").handle("show")
           )
      << Show();

  input << Image().handle("image") << Create();

  gui["show"].registerCallback(function(input,
                      &GUI::switchVisibility));
  
  gui["save"].registerCallback(saveImage);
}

void run(){
  static ButtonHandle next = gui["next"];
  const ImgBase *image = grabber.grab();

  if(next.wasTriggered()){
    currFilter = (currFilter+1)%10;
    op.setOptype((MorphologicalOp::optype)currFilter);
  }
  
  resultMutex.lock();
  op.apply(image,&result);
  resultMutex.unlock();
  
  gui["result"] = result;

  if(input.isVisible()){
    input["image"] = image;
  } 
}
int main(int n, char **args){
   return ICLApp(n,args,"-input|-i(2)",init,run).exec();
}
