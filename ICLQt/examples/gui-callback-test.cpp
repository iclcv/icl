#include <iclCommon.h>

/// We use a global gui to be able to access it outside main 
GUI gui;
// This global LableHandle is obtained ones from the gui
// when it was created using gui.show(). Remember that
// gui.getValue performs a string lookup using a std::map
// with is kind of complex
LabelHandle h;

// Our working thread, calling it's run function 
// asynchronously to the GUI Thread
class DemoThread : public Thread{
  virtual void run(){
    while(true){
      h = Time::now().toString();
      sleep(2);
    }
  }
};

// a simple callback function of type "void (*callback)(void)"
void exit_callback(void){
  printf("exit callback was called \n");
  exit(0);
}

// another one (we can also access the GUI from here)
void click_callback(){
  h = "hello!";
  std::cout << "hello" << std::endl;
}

// a more complex callback implementing the GUI::Callback interface
// In contrast to simple functions, this callbacks are able to have 'own data'
struct MyCallback : public GUI::Callback{
  Time m_lastTime;
  virtual void exec(){
    Time now = Time::now();
    Time dt = m_lastTime-now;
    gui.getValue<LabelHandle>("time-diff-label") = dt.toString();
    m_lastTime = now;
  }
};

int main(int n, char**ppc){
  // create a QApplication object
  QApplication app(n,ppc);
  
  // create some nice components
  gui << "label(something)[@handle=current-time-label@label=current time]"
      << "label(something)[@handle=time-diff-label@label=time since last call]"
      << "button(Click me!)[@handle=click]"
      << "button(Click me too!)[@handle=click-2]"
      << "button(Exit!)[@handle=exit]";
  
  
  // create and show the GUI
  gui.show();
  h = gui.getValue<LabelHandle>("current-time-label");
  
  // register callbacks
  gui.getValue<ButtonHandle>("exit").registerCallback(new GUI::Callback(exit_callback));
  gui.getValue<ButtonHandle>("click").registerCallback(new GUI::Callback(click_callback));
  gui.getValue<ButtonHandle>("click-2").registerCallback(new MyCallback);
  
  // create the working loop thread
  DemoThread t;
  
  // start it
  t.start();
  
  // enter Qt's event loop
  app.exec();
  
  // stop the thread immediately when the window is closed
  t.stop();
}
