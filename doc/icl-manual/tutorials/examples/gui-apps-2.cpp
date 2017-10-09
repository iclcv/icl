// first, we include the 'care-free'-header ICLQt/Common.h which
// includes all most common ICL-headers. Please note, this
// header also includes all icl sub-namespaces icl::utils,
// icl::math, etc. as well as the std namespace
#include <ICLQt/Common.h>

// we need a global GUI instance that can be seen from your
// init- and from your run-function. As you application
// normally has only one static GUI instances, the use of a
// global variable is justifiable here
GUI gui;

// the same is true for the image source of your application.
// Here, we use an instance of type GenericGrabber,
// which can be configured simply using program arguments
GenericGrabber grabber;

// we use the init function for initialization of the
// grabber and the GUI instance
void init(){
   // the grabber can be initialized directly with the given
   // program argument, which can be accessed by the utils::pa
   // function. utils::pa gets one of the program argument's
   // name aliases and optionally also the sub-argument index.
   grabber.init(pa("-i"));

   // now we have to create a GUI component for image
   // visualization. To this ends, we use ICL's powerful
   // Qt-based GUI creation framework. Here, GUI components
   // can conveniently streamed into each other to create
   // complex GUIs, but also for simple GUI as necessary
   // in this example, it is convenient to use.
   gui << Image().handle("image") << Show();
}

// the run function contains our processing loop which is
// acutually very simple in this case. It just grabs a new
// image and visualizes it with our image visualization GUI
// component.
void run(){
   // pass the next grabbed image from the grabber instance
   // directly to the visualization component
   gui["image"] = grabber.grab();
}

// the main function creates an ICLApp instance and
// returns the result of its exec()-function. It also
// specifies the list of allowed program arguments. In this
// case, only a single argument is allowed, that gets two
// sub-arguments. The "[m]"-prefix makes the argument
// mandatory
int main(int n, char **args){
   return ICLApp(n,args,"[m]-input|-i(2)",init,run).exec();
}
