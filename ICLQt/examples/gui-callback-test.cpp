/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/examples/gui-callback-test.cpp                   **
** Module : ICLQt                                                  **
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

/// global gui instance
GUI gui;

// Our working thread, calling it's run function 
// asynchronously to the GUI Thread
void run(){
  // shortcut to extract currentTimeLabel from the gui
  gui_LabelHandle(currentTimeLabel);
  currentTimeLabel = Time::now().toString();
  Thread::sleep(1);
}

// a simple callback function of type "void (*callback)(void)"
void exit_callback(void){
  printf("exit callback was called \n");
  exit(0);
}

// another one (we can also access the GUI from here)
void click_callback(){
  gui_LabelHandle(currentTimeLabel);
  currentTimeLabel = "hello!";
}

// a more complex callback implementing the GUI::Callback interface
// In contrast to simple functions, this callbacks are able to have 'own data'
struct MyCallback : public GUI::Callback{
  Time m_lastTime;
  virtual void exec(){
    Time now = Time::now();
    Time dt = now-m_lastTime;

    // here we could use the macro gui_LabelHandle(timeDiffLabel) as well
    gui.getValue<LabelHandle>("timeDiffLabel") = str(dt.toSecondsDouble())+" sec";
    m_lastTime = now;
  }
};

void init(){
  // create some nice components
  gui << "label(something)[@handle=currentTimeLabel@label=current time]"
      << "label(something)[@handle=timeDiffLabel@label=time since last call]"
      << "button(Click me!)[@handle=click]"
      << "button(Click me too!)[@handle=click-2]"
      << "button(Exit!)[@handle=exit]";
  
  
  // create and show the GUI
  gui.show();
  
  /// sometimes, this works as well !
  gui["currentTimeLabel"] = Time::now().toString();
  
  // register callbacks (on the specific handles)
  gui.getValue<ButtonHandle>("exit").registerCallback(new GUI::Callback(exit_callback));
  gui.getValue<ButtonHandle>("click").registerCallback(new GUI::Callback(click_callback));

  // or let gui find the corresponding components internally
  gui.registerCallback(new MyCallback,"click-2");
  
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}
