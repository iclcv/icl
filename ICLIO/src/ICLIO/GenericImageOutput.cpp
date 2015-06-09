/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/ICLIO/GenericImageOutput.cpp                 **
** Module : ICLIO                                                  **
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

#include <ICLIO/GenericImageOutput.h>

#include <ICLIO/ImageOutput.h>

#ifdef ICL_HAVE_QT
#include <ICLIO/SharedMemoryPublisher.h>
#endif

#ifdef ICL_HAVE_ZMQ
#include <ICLIO/ZmqImageOutput.h>
#endif

#ifdef ICL_HAVE_OPENCV
#include <ICLIO/OpenCVVideoWriter.h>
#endif

#ifdef ICL_HAVE_LIBAV
#include <ICLIO/LibAVVideoWriter.h>
#endif

#if defined(ICL_HAVE_RSB) && defined(ICL_HAVE_PROTOBUF)
#include <ICLIO/RSBImageOutput.h>
#endif

#include <ICLIO/FileWriter.h>

#include <ICLUtils/StringUtils.h>
#include <ICLUtils/TextTable.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace io{
    
    
    GenericImageOutput::GenericImageOutput(const std::string &type, const std::string &description){
      init(type,description);
    }
  
    GenericImageOutput::GenericImageOutput(const ProgArg &pa){
      init(pa);
    }
    
    void GenericImageOutput::init(const ProgArg &pa){
      init(*pa,utils::pa(pa.getID(),1));
    }
  
    void GenericImageOutput::init(const std::string &type, const std::string &description){
      impl = SmartPtr<ImageOutput>();
            
      this->type = type;
      this->description = description;
      
      ImageOutput *o = 0;
  
      std::string d = description;
      if(d.substr(0,type.length()+1) == type+"=") d = d.substr(type.length()+1);
  
      if(type == "null"){
        struct NullOutput : public ImageOutput{
          virtual void send(const ImgBase *) {}
        };
        o = new NullOutput;
      }
      std::vector<std::string> plugins;
      
  #ifdef ICL_HAVE_LIBAV
      plugins.push_back("video~Video File~libav based video file writer");
      if(type == "video"){
        try{
          std::vector<std::string> t = tok(d,",");
          if(!t.size()) throw ICLException("unable to create LibAVVideoWriter with empty destination filename");
          std::string fourcc = t.size() > 1 ? t[1] : str("DIV3");
          Size size = t.size() > 2 ? parse<Size>(t[2]) : Size::VGA;
          double fps = t.size() > 3 ? parse<double>(t[3]) : 24;
          std::cout<<"t.size():"<<size<<std::endl;
          o = new LibAVVideoWriter(t[0], fourcc, fps, size);
        }catch(const std::exception &e){
          ERROR_LOG("Unable to create LibAVVideoWriter with this parameters: " << d << "(error: " << e.what() << ")");
        }

      }
  #else
  #ifdef ICL_HAVE_OPENCV
      plugins.push_back("video~Video File~OpenCV based video file writer");
      if(type == "video"){
        try{
          std::vector<std::string> t = tok(d,",");
          if(!t.size()) throw ICLException("unable to create OpenCVVideoWriter with empty destination filename");
          std::string fourcc = t.size() > 1 ? t[1] : str("DIV3");
          Size size = t.size() > 2 ? parse<Size>(t[2]) : Size::VGA;
          double fps = t.size() > 3 ? parse<double>(t[3]) : 24;
          o = new OpenCVVideoWriter(t[0], fourcc, fps, size);
        }catch(const std::exception &e){
          ERROR_LOG("Unable to create OpenCVVideoWriter with this parameters: " << d << "(error: " << e.what() << ")");
        }
        
      }
  #endif
  #endif

  #ifdef ICL_HAVE_ZMQ
      plugins.push_back("zmq~port~ZMQ-based network transfer");
      if(type == "zmq"){
        o = new ZmqImageOutput(parse<int>(d));
      }

  #endif
  
  #ifdef ICL_HAVE_QT
      plugins.push_back("sm~Shared Memory Segment ID~Qt based shared memory writer");

      if(type == "sm"){
        o = new SharedMemoryPublisher(d);
      }
  #endif
      
  #if defined(ICL_HAVE_RSB) && defined(ICL_HAVE_PROTOBUF)
      plugins.push_back("rsb~[transport:]/scope~Network output stream");
      if(type == "rsb"){
        try{
          std::vector<std::string> ts = tok(d,":");
          if(!ts.size()) throw ICLException("unable to create RSBImageOutput without scope-definition");
          if(ts.size() == 1){
      o = new RSBImageOutput(ts[0],"");
          }else if(ts.size() == 2){
            o = new RSBImageOutput(ts[1],ts[0]);
          }else{
            throw ICLException("invalid definition string (exptected: [transport-list:]scope");
          }
  
        }catch(std::exception &e){
          ERROR_LOG("Unable to create RSBImageOutput with this parameters: " << d << "(error: "  <<e.what() << ")");
        }
      }
  #endif
      plugins.push_back("file~File Pattern~File Writer");
      
      if(type == "file"){
        o = new FileWriter(d);
      }
      
      if(type == "list"){
        int numPlugins = plugins.size();
        TextTable t(4,numPlugins+1,50);
        t(0,0) = "nr";
        t(1,0) = "id";
        t(2,0) = "parameter";
        t(3,0) = "explanation";
        for(size_t i=0;i<plugins.size();++i){
          t(0,i+1) = str(i);
          std::vector<std::string> ts = tok(plugins[i],"~");
          t(1,i+1) = ts[0];
          t(2,i+1) = ts[1];
          t(3,i+1) = ts[2];
        }     
        
        std::cout << "Supported Image Output Devices: "<< std::endl
                  << std::endl << t << std::endl;
        
        std::terminate();
      }
      
      if(!o){
        ERROR_LOG("unable to instantiate GenericImageOutput with type \"" << type << "\" and params \"" << d << "\"");
      }
      else{
        impl = SmartPtr<ImageOutput>(o);
      }
    }
  } // namespace io
}


