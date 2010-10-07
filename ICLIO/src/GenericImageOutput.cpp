/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/GenericImageOutput.cpp                       **
** Module : ICLIO                                                  **
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

#include <ICLIO/GenericImageOutput.h>

#include <ICLIO/ImageOutput.h>

#ifdef HAVE_XCF
#include <ICLIO/XCFPublisher.h>
#endif

#ifdef HAVE_QT
#include <ICLIO/SharedMemoryPublisher.h>
#endif

#ifdef HAVE_OPENCV2
#include <ICLIO/OpenCVVideoWriter.h>
#endif

#include <ICLIO/FileWriter.h>

#include <ICLUtils/StringUtils.h>

namespace icl{
  
  
  GenericImageOutput::GenericImageOutput(const std::string &type, const std::string &description){
    init(type,description);
  }

  void GenericImageOutput::init(const std::string &type, const std::string &description){
    impl = SmartPtr<ImageOutput>();
          
    this->type = type;
    this->description = description;
    
    ImageOutput *o = 0;

    std::string d = description;
    if(d.substr(0,type.length()+1) == type+"=") d = d.substr(type.length()+1);

    
#ifdef HAVE_XCF
    if(type == "xcfp"){
       o = new XCFPublisher(d);
    }
#endif
    
#ifdef HAVE_OPENCV2
    if(type == "video"){
      try{
        std::vector<std::string> t = tok(d,",");
        if(!t.size()) throw ICLException("unable to create OpenCVVideoWriter with empty destination filename");
        std::string fourcc = t.size() > 1 ? t[1] : str("DIV3");
        Size size = t.size() > 2 ? parse<Size>(t[2]) : Size::VGA;
        double fps = t.size() > 3 ? parse<double>(t[3]) : 24;
        o = new OpenCVVideoWriter(t[0],fourcc,fps,size);
      }catch(const std::exception &ex){
        ERROR_LOG("Unable to create OpenCVVideoWriter with this parameters: " << d);
      }
      
    }
#endif
    

#ifdef HAVE_QT
    if(type == "sm"){
      o = new SharedMemoryPublisher(d);
    }
#endif
    
    
    if(type == "file"){
      o = new FileWriter(d);
    }
    
    if(!o){
      ERROR_LOG("unable to instantiate GenericImageOutput with type \"" << type << "\" and params \"" << d << "\"");
    }else{
      impl = SmartPtr<ImageOutput>(o);
    }
  }
}


