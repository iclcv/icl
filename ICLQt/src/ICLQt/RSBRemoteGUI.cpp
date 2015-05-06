/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/RSBRemoteGUI.cpp                       **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/


#define BOOST_SIGNALS_NO_DEPRECATION_WARNING
#include <rsb/Factory.h>
#include <rsb/Handler.h>
#include <rsb/Listener.h>
#include <rsb/Informer.h>

#include <ICLQt/RSBRemoteGUI.h>
#include <ICLQt/DynamicGUI.h>

#include <ICLQt/GUI.h>
#include <ICLQt/ButtonHandle.h>
#include <ICLQt/MouseHandler.h>

#include <ICLIO/RSBGrabber.h>
#include <ICLUtils/Thread.h>

namespace icl{
  namespace qt{
    using namespace utils;
    
    struct RSBRemoteGUI::Data{

      struct StringHandler : public rsb::DataHandler<std::string>{
        std::string componentID;
        std::string componentType;
        GUI *gui;
        bool *verbose;
        StringHandler(const std::string &componentID, const std::string &componentType, GUI *gui, bool *verbose):
          componentID(componentID), componentType(componentType), gui(gui), verbose(verbose){}

        virtual void notify (boost::shared_ptr<std::string> data){
          if(data){
            if(componentType == "button"){
              ButtonHandle b = (*gui)[componentID];
              b.trigger(true);
              if(verbose){
                std::cout << "RSBRemoteGUI received external button click event for component " << componentID << std::endl;
              }
            }else{
              (*gui)[componentID] = *data;
              if(verbose){
                std::cout << "RSBRemoteGUI received string [" << *data << "] as value for component " << componentID << std::endl;
              }
            }
          }else{
            ERROR_LOG("received null string");
          }
        }
      };
    
      typedef DynamicGUI::NodePtr NodePtr;
    
      GUI *gui;

    
      struct Synchronizer : public Thread{
        SmartPtr<io::RSBGrabber> grabber;
        std::string handle;
        GUI *gui;
        bool on;
        Synchronizer(SmartPtr<io::RSBGrabber> grabber, const std::string &handle, GUI *gui):
          grabber(grabber),handle(handle),gui(gui),on(true){
          start();
        }
      
        virtual void run(){
          try{
            while(on){
              //DEBUG_LOG("waiting for new image for image component handle " << handle);
              (*gui)[handle] = grabber->grab();
              //DEBUG_LOG("received image image on input handle " << handle);
            }
          }catch(std::exception &e){
            DEBUG_LOG("ok, grab threw exception!! :" << e.what());
          }
      
        }
      };


      struct Instance{
        NodePtr node;
        rsb::Informer<std::string>::Ptr informer;
        rsb::ListenerPtr listener;
        rsb::HandlerPtr receiver;
        SmartPtr<io::RSBGrabber> grabber;
        SmartPtr<Synchronizer> sync;
      };

      std::map<std::string,Instance> handleLUT;
      bool verbose;
      
      Data():gui(0),verbose(false){}

      ~Data(){
        release();
      }
    
      static void find_child_nodes_with_handle_recursive(NodePtr n, std::vector<NodePtr> &target){
        if(n->isContainer()){
          for(size_t i=0;i<n->children.size();++i){
            find_child_nodes_with_handle_recursive(n->children[i],target);
          }
        }else if(n->hasHandle()){
          target.push_back(n);
        }
      }
    
      static std::string fix(std::string in){
        for(size_t i=0;i<in.length();++i){
          if(in[i] == ' ') in[i] = '_';
        }
        return in;
      }
    
      void send_string(const std::string &text, const std::string &fixedHandle){
        rsb::Informer<std::string>::DataPtr data(new std::string(text));
        if(handleLUT.find(fixedHandle) == handleLUT.end()){
          WARNING_LOG("strangely fixed handle " << fixedHandle << " was not found before");
        }else{
          handleLUT[fixedHandle].informer->publish(data);
        }
        if(verbose){
          std::cout << "RSBRemoteGUI sending string [" << text << "] to " << fixedHandle << std::endl;
        }
      }
    
      void cb_default(const std::string &handle){
        std::string fixed = fix(handle);
        send_string((*gui)[handle], fixed);      
      }

      void cb_toggle_button(const std::string &handle){
        std::string fixed = fix(handle);
        send_string( str((*gui)[handle].as<bool>()), fixed);      

      }

      void cb_button(const std::string &handle){
        std::string fixed = fix(handle);
        send_string("clicked",fixed);
      }

      struct ImageComponentMouseHandler : public MouseHandler{
        Data *data;
        std::string fixed;
        ImageComponentMouseHandler(Data *data, const std::string &fixedHandle):
          data(data),fixed(fixedHandle){
        }
      
        virtual void process(const MouseEvent &e){
          std::ostringstream text;
          // todo: stringify e
          text << "mouse " 
               << (e.isPressEvent() ?  "press" : 
                   e.isReleaseEvent() ? "release" :
                   e.isDragEvent() ? "drag" :
                   "move")
               << " event at (" << e.getPos().x << "," << e.getPos().y << ")  "
               << "color:[" << cat(e.getColor(),",") << "]"
               << " buttonmask: [ "
               << (e.isLeft() ? "1 " : "0 ") 
               << (e.isMiddle() ? "1 " : "0 ") 
               << (e.isRight() ? "1" : "0 ") << "]";
                 
          data->send_string(text.str(), fixed);
        }
      };

      void init(GUI *gui, const std::string &baseScope, bool createSetterGUI){
        if(!gui){
          release();
          return;
        }
        this->gui = gui;

        DynamicGUI dyn;
        dyn.initialize(gui->createXMLDescription());
        DynamicGUI::ParseTreePtr pt = dyn.getParseTree();
        rsb::Factory& factory = rsb::getFactory();
      
        std::vector<NodePtr> all;
        find_child_nodes_with_handle_recursive(pt,all);

        for(size_t i=0;i<all.size();++i){
          std::string handle = all[i]->getHandleName();
          std::string fixed = fix(handle);

          std::string type = all[i]->name;
          static std::vector<std::string> d = tok("slider,fslider,checkbox,combo,string,int,float",",");
          if(std::find(d.begin(),d.end(), type) != d.end()){
            gui->registerCallback(function(this, &Data::cb_default), handle);
          }else if(type == "button"){
            gui->registerCallback(function(this, &Data::cb_button), handle);
          }else if(type == "togglebutton"){
            gui->registerCallback(function(this, &Data::cb_toggle_button), handle);
          }else if(type == "image" || type == "draw" || type == "draw3d" || type == "draw3D"){
            (*gui)[handle].install(new ImageComponentMouseHandler(this,fixed));
          }
          if(createSetterGUI){
            if(type == "image" || type == "draw" || type == "draw3d" || type == "draw3D"){
              /// these are not synchronized!
            }else{
              rsb::Informer<std::string>::Ptr informer = factory.createInformer<std::string> (baseScope + "/set/" + fixed);
              Instance inst = { all[i], informer, rsb::ListenerPtr(), rsb::HandlerPtr() };
              handleLUT[fixed] = inst;
            }
          }else{

            rsb::Informer<std::string>::Ptr informer = factory.createInformer<std::string> (baseScope + "/" + fixed);
            if(verbose){
              std::cout << "RSBRemoteGUI created informer for scope " << (baseScope + "/" + fixed) << std::endl;
            }

            rsb::ListenerPtr listener;
            rsb::HandlerPtr receiver;
            SmartPtr<io::RSBGrabber> grabber;
            SmartPtr<Synchronizer> sync;
          
            if(type == "image" || type == "draw" || type == "draw3d" || type == "draw3D"){
              grabber = new io::RSBGrabber(baseScope+"/set/"+fixed);
              //DEBUG_LOG("registered grabber for receiving image on " 
              //          << baseScope << "/set/" << fixed);
              sync = new Synchronizer(grabber, handle, gui);
              if(verbose){
                std::cout << "RSBRemoteGUI created RSBGrabber for scope " << (baseScope+"/set/"+fixed) << std::endl;
              }
            }else{
              listener = factory.createListener(baseScope + "/set/" + fixed);
              receiver = rsb::HandlerPtr(new Data::StringHandler(handle, type, gui, &verbose));
              listener->addHandler(receiver);
              if(verbose){
                std::cout << "RSBRemoteGUI created string-listener for scope " << (baseScope+"/set/"+fixed) << std::endl;
              }
            }
            Instance inst = { all[i], informer, listener, receiver, grabber, sync };
            handleLUT[fixed] = inst;
          }
        }
      }
    
      void release(){
        if(gui){
      
          // release code
        }
        gui = 0;
      }
    };
  
    RSBRemoteGUI::RSBRemoteGUI(GUI *gui, const std::string &rsbBaseScope, 
                               bool createSetterGUI, bool verbose) : m_data(new Data){
      m_data->verbose = verbose;
      m_data->init(gui,rsbBaseScope, createSetterGUI);
    }
    
    void RSBRemoteGUI::init(GUI *gui, const std::string &rsbBaseScope, bool createSetterGUI){
      m_data->init(gui,rsbBaseScope, createSetterGUI);
    }
    
    RSBRemoteGUI::~RSBRemoteGUI(){
      try{
        ICL_DELETE(m_data);
      }catch(...){}
    }

    void RSBRemoteGUI::setVerboseMode(bool on){
      m_data->verbose = on;
    }
    
  }

}
