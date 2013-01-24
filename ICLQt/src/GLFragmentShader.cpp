/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/GLFragmentShader.cpp                         **
** Module : ICLGeom                                                **
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

#include <ICLQt/GLFragmentShader.h>

#ifdef ICL_SYSTEM_APPLE
#include <OpenGL/glew.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#include <ICLUtils/Macros.h>

#include <ICLQt/GLContext.h>
#include <map>

using namespace icl::utils;

namespace icl{
  namespace qt{
    static std::string shader_info(GLuint obj){
      int infologLength = 0;
      int charsWritten  = 0;
      
      glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
      
      if (infologLength > 1){
        std::string info(infologLength,'\0');
        glGetShaderInfoLog(obj, infologLength, &charsWritten, (GLchar*)info.c_str());
        return info;
      }else{
        return "";
      }
    }
    
    static std::string program_info(GLuint obj){
      int infologLength = 0;
      int charsWritten  = 0;
      
      glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);
      
      if (infologLength > 1){
        std::string info(infologLength,'\0');
        glGetProgramInfoLog(obj, infologLength, &charsWritten, (GLchar*)info.c_str());
        return info;
      }else{
        return "";
      }
    }
    
    struct GLFragmentShader::Data{
      std::string fragmentProgramString;
      std::string vertexProgramString;
  
      struct GLData{
        bool hasFragmentShader;
        bool hasVertexShader;
        GLuint fragmentShader;
        GLuint vertexShader;
        GLuint program;
        bool enabled;
      };
      std::map<GLContext,GLData> lut;
      };
    
    void GLFragmentShader::create(){
      GLContext ctx = GLContext::currentContext();
  
      if(ctx.isNull() || m_data->lut.find(ctx) != m_data->lut.end()) return;


      static bool first = true;
      if(first){
        first = false;
        glewInit();
      }
      
      Data::GLData & d = m_data->lut[ctx];
      
      const bool haveFragmentShader = m_data->fragmentProgramString.length();
      const bool haveVertexShader = m_data->vertexProgramString.length();


      if(haveFragmentShader){
        d.fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
        const char *pF = m_data->fragmentProgramString.c_str();
        int lenF = m_data->fragmentProgramString.size();
        glShaderSource(d.fragmentShader,1,&pF,&lenF);
        glCompileShader(d.fragmentShader);
        
        std::string info = shader_info(d.fragmentShader);
        if(info.length()){
          throw ICLException("unable to compile fragment shader:[ \n" + info + "\n]");
        }
      }
      
      if(haveVertexShader){
        d.vertexShader = glCreateShader(GL_VERTEX_SHADER);
        const char *pV = m_data->vertexProgramString.c_str();
        int lenV = m_data->vertexProgramString.size();
        glShaderSource(d.vertexShader,1,&pV,&lenV);
        glCompileShader(d.vertexShader);
        
        std::string info = shader_info(d.vertexShader);
        if(info.length()){
          throw ICLException("unable to compile vertex shader:[ \n" + info + "\n]");
        }
      }

      d.program = glCreateProgram();

      if(haveFragmentShader) {
        glAttachShader(d.program,d.fragmentShader);
      }

      if(haveVertexShader) {
        glAttachShader(d.program,d.vertexShader);
      }

      glLinkProgram(d.program);
  
      std::string info = program_info(d.program);
      if(info.length()){
        throw ICLException("unable to link shader program:[ \n" + info + "\n]");
      }
    }
    
    GLFragmentShader::GLFragmentShader(const std::string &vertexProgram, 
                                       const std::string &fragmentProgram,
                                       bool createOnFirstActivate) throw (ICLException):
      m_data(new Data){

        if(!vertexProgram.length() && !fragmentProgram.length()){
          throw ICLException("GLFragmentShader::GLFragmentShader: neither vertex- nor fragment-shader was given");
        }
      m_data->vertexProgramString = vertexProgram;
      m_data->fragmentProgramString = fragmentProgram;
      

    
      
      if(!createOnFirstActivate){
        create();
      }
    }
    
    GLFragmentShader::~GLFragmentShader(){
      for(std::map<GLContext,Data::GLData>::iterator it = m_data->lut.begin();
          it != m_data->lut.end(); ++it){
        it->first.makeCurrent();
        Data::GLData &d = it->second;
        if(m_data->vertexProgramString.length()){
          glDetachShader(d.program,d.vertexShader);
        }
        if(m_data->fragmentProgramString.length()){
          glDeleteShader(d.fragmentShader); 
        }
        glDeleteProgram(d.program);
      }
      delete m_data;
    }
    
    void GLFragmentShader::activate(){
      create();
      GLContext ctx = GLContext::currentContext();
      if(ctx.isNull()){
        throw ICLException("tried to activate shader program where no GL-Context was active");
      }
      Data::GLData &d = m_data->lut[ctx];
      d.enabled = true;
      glUseProgram(d.program);
    }
  
    void GLFragmentShader::deactivate(){
      GLContext ctx = GLContext::currentContext();
      if(ctx.isNull()){
        throw ICLException("tried to deactivate shader program where no GL-Context was active");
      }
      
      std::map<GLContext,Data::GLData>::iterator it = m_data->lut.find(ctx);
      if(it != m_data->lut.end()){
        it->second.enabled = false;
        glUseProgram(0);
      }
    }
  
    GLFragmentShader *GLFragmentShader::copy() const{
      return new GLFragmentShader(m_data->vertexProgramString,m_data->fragmentProgramString,true);
    }
  
  } // namespace qt
}
