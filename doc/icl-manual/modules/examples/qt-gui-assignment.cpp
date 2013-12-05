#include <ICLQt/Common.h>

GUI gui;

void init(){
  gui << Slider(0,100,50).handle("mySlider").out("mySliderVal") 
      << Label("something").handle("myLabel")
      << Combo("entry1,entry2,entry3").handle("myCombo")
      << Show();
}

void run(){
  // extract slider value from handle
  int i = gui["mySlider"];
  
  // same works for the value
  int j = gui["mySliderVal"];
  
  // set slider value (actually moves the slider)
  gui["mySlider"] = 30;

  // no error, but the slider is not moved
  gui["mySliderVal"] = 30; 

  // set slider range
  gui["mySlider"] = Range32s(100,200);

  // set label text (must be a string)
  gui["myLabel"] = str("a new text");
  
  // get label text (usually not used)
  std::string t = gui["myLabel"];
  
  // get selected combobox entry
  std::string c = gui["myCombo"];

  // get selected combobox index
  int idx = gui["myCombo"];
  
  // set selected combobox entry (must be string)
  gui["myCombo"] = str("entry1");

  // set selected combobox index
  gui["myCombo"] = 2;
  
  Thread::msleep(1000);
}

int main(int n, char **args){
  return ICLApp(n,args,"",init,run).exec();
}
