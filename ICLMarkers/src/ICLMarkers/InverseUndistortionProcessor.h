/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMarkers/src/ICLMarkers/                             **
**          InverseUndistortionProcessor.h                         **
** Module : ICLMarkers                                             **
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

#pragma once

#include <ICLUtils/Point32f.h>
#include <ICLUtils/Uncopyable.h>

#include <vector>

namespace icl{
  /// Utility class that performs gradient-descent based inverse undistortion mapping
  /** Internally, the class has two operation modes. An OpenCL-implementation and a C++-version.
      Unfortunately, the OpenCL version is not significantly faster for common numbers of points */
  class InverseUndistortionProcessor : public utils::Uncopyable{
    struct Data;   //!< internal data structure
    Data *m_data;  //!< internal data pointer
    
    public:

    /// constructor
    InverseUndistortionProcessor(bool preferOpenCL);

    /// destructor
    ~InverseUndistortionProcessor();

    /// sets whether to try to use open cl
    void setPreferOpenCL(bool preferOpenCL);

    /// performs the mapping with given distortion coefficients
    const std::vector<utils::Point32f> &run(const std::vector<utils::Point32f> &p, const float kf[9]);
  };
}
