// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter, Matthias Esau

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Uncopyable.h>
#include <ICLUtils/Exception.h>
#include <ICLMath/FixedVector.h>
#include <vector>

namespace icl::qt {
  /// Simple wrapper class for OpenGL 2.0 Fragment Shader Programs
  /** The GLFragmentShader class can be used to create simple fragment shader programs.

  */
  class ICLQt_API GLFragmentShader {
    struct Data;
    Data *m_data;

    void create();

    public:
    GLFragmentShader(const GLFragmentShader&) = delete;
    GLFragmentShader& operator=(const GLFragmentShader&) = delete;

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
  } // namespace icl::qt