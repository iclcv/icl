/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLQt/GLFragmentShader.h                       **
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

#pragma once

#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/Exception.h>

namespace icl{
  namespace qt{
    /// Simple wrapper class for OpenGL 2.0 Fragment Shader Programs
    /** The GLFragmentShader class can be used to create simple fragment shader programs.
        
    */
    class GLFragmentShader : public utils::Uncopyable{
      struct Data;
      Data *m_data;
      
      void create();
      
      public:
      GLFragmentShader(const std::string &vertexProgram, 
                       const std::string &fragmentProgram,
                       bool createOnFirstActivate=true) throw (utils::ICLException);
      ~GLFragmentShader();
      
      void activate();
      
      /// deactivates the shader
      /** This function does not do anything, if the shader was not enabled before! */
      void deactivate();
      
      /// creates a deep copy of this shader
      /** The resulting copy does only use this instance's program string and is other than this
          independent. The copy is created in createOnFirstActivate mode */
      GLFragmentShader *copy() const;
    };
  } // namespace qt
}

