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

namespace icl{
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
    std::string programString;
    GLuint shader;
    GLuint program;
    bool enabled;
    bool created;
  };
  
  void GLFragmentShader::create(){
    if(m_data->created) return;
    m_data->shader = glCreateShader(GL_FRAGMENT_SHADER);
    
    const char *p = m_data->programString.c_str();
    int len = m_data->programString.size();
    
    glShaderSource(m_data->shader,1,&p,&len);
    glCompileShader(m_data->shader);
    
    std::string info = shader_info(m_data->shader);
    if(info.length()){
      throw ICLException("unable to compile shader:[ \n" + info + "\n]");
    }
    m_data->program = glCreateProgram();
    
    glAttachShader(m_data->program,m_data->shader);
    glLinkProgram(m_data->program);

    info = program_info(m_data->program);
    if(info.length()){
      throw ICLException("unable to link shader program:[ \n" + info + "\n]");
    }
    m_data->created = true;
  }
  
  GLFragmentShader::GLFragmentShader(const std::string &program, bool createOnFirstActivate) throw (ICLException):
    m_data(new Data){
    m_data->programString = program;
    m_data->enabled = false;
    m_data->created = false;
    
    static bool first = true;
    if(first){
      first = false;
      glewInit();
    }
    
    if(!createOnFirstActivate){
      create();
    }
  }
  
  GLFragmentShader::~GLFragmentShader(){
    glDetachShader(m_data->program,m_data->shader);
    glDeleteShader(m_data->shader); 
    glDeleteProgram(m_data->program);
    delete m_data;
  }
  
  void GLFragmentShader::activate(){
    create();
    m_data->enabled = true;
    glUseProgram(m_data->program);
  }

  void GLFragmentShader::deactivate(){
    if(m_data->enabled){
      m_data->enabled = false;
      glUseProgram(0);
    }
  }
}
