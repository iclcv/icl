// SPDX-License-Identifier: LGPL-3.0-or-later
// Copyright (C) 2006-2026 Christof Elbrechter

#ifndef ICL_SCENE_MULTICAM_CAPTURER_H
#define ICL_SCENE_MULTICAM_CAPTURER_H

#include <ICLGeom/Scene.h>
#include <ICLIO/GenericImageOutput.h>

namespace icl{
  class SceneMultiCamCapturer {
    std::vector<int> camIndices;
    geom::Scene *scene;
    std::vector<std::shared_ptr<io::GenericImageOutput> > outputs;

    public:
    /// Dummy constructor
    SceneMultiCamCapturer();

    /// Simple constructor, that adds 3 default cameras to the scene, and creates SharedMemory image outputs sm 1,2,3
    SceneMultiCamCapturer(geom::Scene &scene, const std::vector<geom::Camera> &cams=std::vector<geom::Camera>());

    SceneMultiCamCapturer(geom::Scene &scene, int num, int* camIndices,
                          const std::string &progArgName="-o");

    void init(geom::Scene &scene, int num, int* camIndices,
              const std::string &progArgName="-o");

    void capture();
  };
}

#endif
