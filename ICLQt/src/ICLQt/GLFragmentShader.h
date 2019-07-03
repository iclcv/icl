/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLQt/src/ICLQt/GLFragmentShader.h                     **
** Module : ICLQt                                                  **
** Authors: Christof Elbrechter, Matthias Esau                     **
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

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/Exception.h>
#include <ICLMath/FixedVector.h>
#include <vector>

namespace icl{
  namespace qt{
    /// Simple wrapper class for OpenGL 2.0 Fragment Shader Programs
    /** The GLFragmentShader class can be used to create simple fragment shader programs.

    */
    class ICLQt_API GLFragmentShader : public utils::Uncopyable{
      struct Data;
      Data *m_data;

      void create();

      public:
      GLFragmentShader(const std::string &vertexProgram,
                       const std::string &fragmentProgram,
                       bool createOnFirstActivate=true);
      ~GLFragmentShader();

      void setUniform(const std::string var, const float &val);
      void setUniform(const std::string var, const int &val);
      void setUniform(const std::string var, const math::FixedMatrix<float,4,4> &val);
      void setUniform(const std::string var, const std::vector<math::FixedMatrix<float,4,4> > &val);
      void setUniform(const std::string var, const math::FixedColVector<float,4> &val);

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

