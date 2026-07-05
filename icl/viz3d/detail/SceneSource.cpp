// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

// "scene" image-source backend: a synthetic depth / RGBD / color camera that
// renders a built-in viz3d scene. It lets the depth-camera / point-cloud apps
// be exercised without real hardware. Headless: it uses the CPU BVH raytracer
// (no GL context needed), so it works in any app's grab thread.
//
//   -i scene                      (depth Img32f, mm, camera in image metadata)
//   -i scene@format=rgbd          (4-ch float: R,G,B,depth)
//   -i scene@format=color         (Img8u RGB)
//   -i scene@depth mode=DistToCamCenter
//   -i scene@animate=off
//
// The rendered camera is serialized into the image's metadata (operator<< /
// operator>> round-trip), so a consumer can unproject the depth into a cloud
// (see PointCloudCreator) without separately being told the intrinsics.

#include <icl/io/detail/SourceBackend.h>
#include <icl/io/source/SourceBackendRegistry.h>
#include <icl/io/source/DeviceDescription.h>
#include <icl/viz3d/Scene2.h>
#include <icl/viz3d/SceneCapture.h>
#include <icl/viz3d/GroupNode.h>
#include <icl/viz3d/MeshNode.h>
#include <icl/viz3d/SphereNode.h>
#include <icl/viz3d/CuboidNode.h>
#include <icl/viz3d/ConeNode.h>
#include <icl/viz3d/LightNode.h>
#include <icl/cv3d/Camera.h>
#include <icl/viz3d/Material.h>
#include <icl/core/Image.h>
#include <icl/utils/prop/Constraints.h>
#include <sstream>

namespace icl::viz3d {

  using namespace icl::geom;
  using icl::utils::Size;

  class SceneSource : public io::SourceBackend {
  public:
    explicit SceneSource(const std::string &/*param*/) {
      addCamera();
      buildScene();
      m_capture.setCaching(true);   // rebuilt only when the scene is animated

      addProperty("format", utils::prop::Menu{"depth", "rgbd", "color"}, "depth");
      addProperty("depth mode", utils::prop::Menu{"DistToCamPlane", "DistToCamCenter"},
                  "DistToCamPlane");
      addProperty("animate", utils::prop::Flag{}, true);
    }

    core::Image acquireImage() override {
      if ((bool)prop("animate").value && m_spinner) {
        m_spinner->rotate(0, 0, 0.03f);
        m_capture.invalidate();    // geometry changed → rebuild the BVH
      }

      const BVH::DepthMode mode =
          (std::string)prop("depth mode").value == "DistToCamCenter"
              ? BVH::DistToCamCenter : BVH::DistToCamPlane;
      const BVH::ImageResult r = m_capture.capture(m_scene, 0, mode);

      const std::string fmt = prop("format").value;
      core::Image out;
      if (fmt == "color")      out = core::Image(r.image);
      else if (fmt == "rgbd")  out = core::Image(packRGBD(r.image, r.depth));
      else                     out = core::Image(r.depth);

      // Attach the depth camera so the consumer can unproject without intrinsics.
      std::ostringstream os;
      os << m_scene.getCamera(0);
      out.ptr()->setMetaData(os.str());
      return out;
    }

  private:
    void addCamera() {
      // A fixed VGA camera looking at the content; intrinsics travel in metadata.
      m_scene.addCamera(Camera::lookAt(Vec(0, -600, 320, 1), Vec(0, 0, 40, 1),
                                       Vec(0, 0, 1, 1), Size::VGA, 45.0f));
      m_scene.setBounds(500);
    }

    void buildScene() {
      m_spinner = std::make_shared<GroupNode>();
      auto sph = SphereNode::create(0, 0, 70, 50, 32, 32);
      sph->setMaterial(Material::fromColor(GeomColor(220, 70, 70, 255)));
      m_spinner->addChild(sph);
      auto cube = CuboidNode::create(130, 0, 45, 60, 60, 90);
      cube->setMaterial(Material::fromColor(GeomColor(70, 90, 230, 255)));
      m_spinner->addChild(cube);
      auto cone = ConeNode::create(-130, 0, 30, 50, 50, 100, 28);
      cone->setMaterial(Material::fromColor(GeomColor(240, 200, 60, 255)));
      m_spinner->addChild(cone);
      m_scene.addNode(m_spinner);

      auto ground = std::make_shared<MeshNode>();
      const float gs = 340;
      ground->addVertex(Vec(-gs, -gs, 0, 1));
      ground->addVertex(Vec(gs, -gs, 0, 1));
      ground->addVertex(Vec(gs, gs, 0, 1));
      ground->addVertex(Vec(-gs, gs, 0, 1));
      ground->addQuad(0, 1, 2, 3);
      ground->createAutoNormals(false);
      ground->setMaterial(Material::fromColor(GeomColor(165, 165, 170, 255)));
      m_scene.addNode(ground);

      auto light = std::make_shared<LightNode>(LightNode::Point);
      light->setIntensity(1.6f);
      light->translate(250, -150, 400);
      m_scene.addLight(light);
    }

    static core::Img32f packRGBD(const core::Img8u &color, const core::Img32f &depth) {
      core::Img32f rgbd(depth.getSize(), 4);   // ch0-2 = RGB [0,255], ch3 = depth (mm)
      const int dim = depth.getDim();
      const bool haveColor = color.getDim() == dim && color.getChannels() >= 3;
      const icl8u *R = haveColor ? color.getData(0) : nullptr;
      const icl8u *G = haveColor ? color.getData(1) : nullptr;
      const icl8u *B = haveColor ? color.getData(2) : nullptr;
      const float *d = depth.getData(0);
      float *o0 = rgbd.getData(0), *o1 = rgbd.getData(1), *o2 = rgbd.getData(2), *o3 = rgbd.getData(3);
      for (int i = 0; i < dim; ++i) {
        o0[i] = R ? R[i] : 0; o1[i] = G ? G[i] : 0; o2[i] = B ? B[i] : 0;
        o3[i] = d[i];
      }
      return rgbd;
    }

    Scene2 m_scene;
    BVHSceneCapture m_capture;
    std::shared_ptr<GroupNode> m_spinner;
  };

  static io::SourceBackend *createScene(const std::string &param) {
    return new SceneSource(param);
  }

  static const std::vector<io::DeviceDescription> &getSceneDeviceList(std::string, bool) {
    static std::vector<io::DeviceDescription> list;
    if (list.empty())
      list.push_back(io::DeviceDescription(
          "scene", "default", "synthetic scene depth/RGBD/color source"));
    return list;
  }

  REGISTER_SOURCE_BACKEND(scene, createScene, getSceneDeviceList,
      "scene preset (optional)~synthetic depth/RGBD/color camera over a built-in viz3d scene")

} // namespace icl::viz3d
