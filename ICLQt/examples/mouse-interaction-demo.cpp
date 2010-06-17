/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/examples/mouse-interaction-demo.cpp              **
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
#include <iterator>

GenericGrabber *grabber;
ICLWidget *widget;


class Mouse : public MouseHandler{
public:
  virtual void process(const MouseEvent &event){
    std::cout << "image location: " << event.getPos() << std::endl;    
    std::cout << "widget location: " << event.getWidgetPos() << std::endl;
    
    std::string eventNames[] = {"MoveEvent","DragEvent","PressEvent","ReleaseEvent", "EnterEvent","LeaveEvent"};
    std::cout << "type: " << eventNames[event.getType()] << std::endl;
    
    if(event.isPressEvent()){
      if(event.getColor().size()){
        std::cout << "color:";
        std::copy(event.getColor().begin(),event.getColor().end(),std::ostream_iterator<icl64f>(std::cout,","));
        std::cout << endl;
      }else{
        std::cout << "no color here!" << std::endl;
      }
    }
  } 
} mouse;

void init(){
  widget = new ICLWidget(0);
  widget->setGeometry(200,200,640,480);
  widget->show();
  grabber = new GenericGrabber;
  widget->install(&mouse);
}  
void run(){
  widget->setImage(grabber->grab());
  widget->update();
}

int main(int n, char **ppc){
  return ICLApplication(n,ppc,"",init,run).exec();
}
