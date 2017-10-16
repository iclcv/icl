#include <ICLQt/Common.h>
#include <ICLFilter/UnaryCompareOp.h>
#include <ICLUtils/FPSLimiter.h>

// again, we need some static data
GUI gui;                   // our static top-level GUI-instance
GenericGrabber grabber;    // an image source
UnaryCompareOp cmp(">");   // pixel comparison operator
FPSLimiter fps(25);        // utility to limit the run-loop


// again, we use the same application structure, init is called
// before Qt's event-loop is entered. Here the GUI is layouted
// and the grabber is initialized
void init(){
  // The stream operator '<<' can be cascaded to layout GUIs
  gui << Image().handle("image")
      << Slider(0,255,127).handle("thresh").label("threshold").maxSize(100,2)
      << Show();

  // again, the image source shall be selectable using
  // program arguments
  grabber.init(pa("-i"));
}

// the run loop is called in an extra thread after Qt's event
// loop was entered
void run(){
  // extract the current slider value from the gui and pass it
  // to the UnaryCompareOp instance. Even if the slider was moved
  // several times since this section is proccessed last time,
  // the value is only extracted once
  cmp.setValue(gui["thresh"]);

  // This is a combined expression: the next image is acquired
  // from the grabber and it is passed directly to the
  // apply method of the UnaryCompareOp instance. The resulting
  // image is again forwarded directly to the GUI-component
  // for visualization
  gui["image"] = cmp.apply(grabber.grab());

  // this suspends the current thread until at least 1/25 seconds
  // have passed since the last time this function was called
  fps.wait();
}

// again, the main function calls init, starts a thread for run
// and enters Qt's event loop
int main(int n, char **args){
  return ICLApplication(n,args,"-input|-i(device,device-info)",
                        init,run).exec();
}
