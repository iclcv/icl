// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Matthias Esau

#include <icl/qt/GLFragmentShader.h>

#include <icl/utils/CompatMacros.h>
#include <QOpenGLContext>
#include <icl/utils/Macros.h>

#include <map>

#include <cstring>

using namespace icl::utils;

namespace icl::qt {
  static std::string shader_info(GLuint obj){
    int infologLength = 0;
    int charsWritten  = 0;

    glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 1){
      std::string info(infologLength,'\0');
      glGetShaderInfoLog(obj, infologLength, &charsWritten, const_cast<GLchar*>(info.c_str()));
      return info;
    }else{
      return "";
    }
  }

  static bool shader_compiled(GLuint obj){
    GLint compiled = 0;
    glGetShaderiv(obj, GL_COMPILE_STATUS, &compiled);
    return compiled;
  }

  static std::string program_info(GLuint obj){
    int infologLength = 0;
    int charsWritten  = 0;

    glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

    if (infologLength > 1){
      std::string info(infologLength,'\0');
      glGetProgramInfoLog(obj, infologLength, &charsWritten, const_cast<GLchar*>(info.c_str()));
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
      bool created;
    };
    GLData data;
    };

  void GLFragmentShader::create(){
    static bool first = true;
    if(first){
      first = false;
      glewInit();
    }

    if(m_data->data.created)return;

    Data::GLData & d = m_data->data;
    const bool haveFragmentShader = m_data->fragmentProgramString.length();
    const bool haveVertexShader = m_data->vertexProgramString.length();


    if(haveFragmentShader){
      d.fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
      const char *pF = m_data->fragmentProgramString.c_str();
      int lenF = m_data->fragmentProgramString.size();
      glShaderSource(d.fragmentShader,1,&pF,&lenF);
      glCompileShader(d.fragmentShader);

      if(!shader_compiled(d.fragmentShader)){
        std::string info = shader_info(d.fragmentShader);
        throw ICLException("unable to compile fragment shader:[ \n" + info + "\n]");
      }
    }

    if(haveVertexShader){
      d.vertexShader = glCreateShader(GL_VERTEX_SHADER);
      const char *pV = m_data->vertexProgramString.c_str();
      int lenV = m_data->vertexProgramString.size();
      glShaderSource(d.vertexShader,1,&pV,&lenV);
      glCompileShader(d.vertexShader);

      if (!shader_compiled(d.vertexShader)){
        std::string info = shader_info(d.vertexShader);
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
    m_data->data.created = true;
  }

  GLFragmentShader::GLFragmentShader(const std::string &vertexProgram,
                                     const std::string &fragmentProgram,
                                     bool createOnFirstActivate):
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
    Data::GLData &d = m_data->data;
    if(m_data->vertexProgramString.length()){
      glDetachShader(d.program,d.vertexShader);
    }
    if(m_data->fragmentProgramString.length()){
      glDeleteShader(d.fragmentShader);
    }
    glDeleteProgram(d.program);
    delete m_data;
  }


  void GLFragmentShader::setUniform(const std::string var, const float &val){
    const QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if(!ctx){
      throw ICLException("tried to deactivate shader program where no GL-Context was active");
    }
    GLint loc = glGetUniformLocation(m_data->data.program, var.c_str());
    if (loc != -1)
    {
      glUniform1f(loc, val);
    } else {
//          throw ICLException("Tried to set a non-existent uniform.");
    }
  }

  void GLFragmentShader::setUniform(const std::string var, const math::FixedMatrix<float,4,4> &val){
    const QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if(!ctx){
      throw ICLException("tried to deactivate shader program where no GL-Context was active");
    }
    GLint loc = glGetUniformLocation(m_data->data.program, var.c_str());
    if (loc != -1)
    {
      glUniformMatrix4fv(loc, 1, true, val.data());
    } else {
//          throw ICLException("Tried to set a non-existent uniform.");
    }
  }

  void GLFragmentShader::setUniform(const std::string var, const std::vector<math::FixedMatrix<float,4,4> > &val){
    const QOpenGLContext *ctx = QOpenGLContext::currentContext();
    if(!ctx){
      throw ICLException("tried to deactivate shader program where no GL-Context was active");
    }
    GLint loc = glGetUniformLocation(m_data->data.program, var.c_str());
    if (loc != -1)
    {
      float *matData = new float[16 * val.size()];
      for(unsigned int i = 0; i < val.size(); i++) {
        memcpy((matData + 16 * i), val[i].data(), 16 * sizeof(float));
      }
      glUniformMatrix4fv(loc, val.size(), true, matData);

      delete[] matData;
    } else {
//          throw ICLException("Tried to set a non-existent uniform.");
    }
  }

  void GLFragmentShader::setUniform(const std::string var, const math::FixedColVector<float,4> &val){
    const QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if(!ctx){
      throw ICLException("tried to deactivate shader program where no GL-Context was active");
    }
    GLint loc = glGetUniformLocation(m_data->data.program, var.c_str());
    if (loc != -1)
    {
      glUniform4f(loc, val[0], val[1], val[2], val[3]);
    } else {
//          throw ICLException("Tried to set a non-existent uniform.");
    }
  }

  void GLFragmentShader::setUniform(const std::string var, const int &val){
    const QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if(!ctx){
      throw ICLException("tried to deactivate shader program where no GL-Context was active");
    }
    GLint loc = glGetUniformLocation(m_data->data.program, var.c_str());
    if (loc != -1)
    {
      glUniform1i(loc, val);
    } else {
//          throw ICLException("Tried to set a non-existent uniform.");
    }
  }

  void GLFragmentShader::activate(){
    create();
    const QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if(!ctx){
      throw ICLException("tried to activate shader program where no GL-Context was active");
    }
    Data::GLData &d = m_data->data;
    d.enabled = true;
    glUseProgram(d.program);
  }

  void GLFragmentShader::deactivate(){
    const QOpenGLContext* ctx = QOpenGLContext::currentContext();
    if(!ctx){
      throw ICLException("tried to deactivate shader program where no GL-Context was active");
    }
    m_data->data.enabled = false;
    glUseProgram(0);
  }

  GLFragmentShader *GLFragmentShader::copy() const{
    return new GLFragmentShader(m_data->vertexProgramString,m_data->fragmentProgramString,true);
  }

  } // namespace icl::qt