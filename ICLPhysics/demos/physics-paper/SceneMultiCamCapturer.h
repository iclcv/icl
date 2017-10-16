/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2014 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLPhysics/demos/physics-paper/SceneMultiCamCapturer.h **
** Module : ICLPhysics                                             **
** Author : Christof Elbrechter, Matthias Esau                     **
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
#ifndef ICL_SCENE_MULTICAM_CAPTURER_H
#define ICL_SCENE_MULTICAM_CAPTURER_H

#include <ICLGeom/Scene.h>
#include <ICLIO/GenericImageOutput.h>

namespace icl{
  class SceneMultiCamCapturer {
    std::vector<int> camIndices;
    geom::Scene *scene;
    std::vector<utils::SmartPtr<io::GenericImageOutput> > outputs;

    public:
    /// Dummy constructor
    SceneMultiCamCapturer();

    /// Simple constructor, that adds 3 default cameras to the scene, and creates SharedMemory image outputs sm 1,2,3
    SceneMultiCamCapturer(geom::Scene &scene, const std::vector<geom::Camera> &cams=std::vector<geom::Camera>());

    SceneMultiCamCapturer(geom::Scene &scene, int num, int* camIndices,
                          const std::string &progArgName="-o") throw (utils::ICLException);

    void init(geom::Scene &scene, int num, int* camIndices,
              const std::string &progArgName="-o") throw (utils::ICLException);

    void capture();
  };
}

#endif

