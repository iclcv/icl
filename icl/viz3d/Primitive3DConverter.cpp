// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/viz3d/Primitive3DConverter.h>
#include <icl/viz3d/nodes/GeometryNode.h>
#include <icl/viz3d/nodes/CuboidNode.h>
#include <icl/viz3d/nodes/SphereNode.h>
#include <icl/viz3d/nodes/CylinderNode.h>
#include <icl/viz3d/render/Material.h>

namespace icl::viz3d {

  NodePtr nodeFromPrimitive3D(const cv3d::Primitive3D &p,
                              uint32_t slices, const cv3d::GeomColor &color) {
    using P = cv3d::Primitive3D;

    // orientation → rotation matrix, with the primitive position in column 3
    // (getTransformationMatrix is non-const → work on a local copy)
    P::Quaternion orientation = p.orientation;
    math::FixedMatrix<float,4,4> mat = orientation.getTransformationMatrix();
    mat(0,3) = p.position[0];
    mat(1,3) = p.position[1];
    mat(2,3) = p.position[2];

    std::shared_ptr<GeometryNode> node;
    switch (p.type) {
      case P::CUBE:
        node = CuboidNode::create(0,0,0, p.scale[0], p.scale[1], p.scale[2]);
        break;
      case P::SPHERE:
        node = std::make_shared<SphereNode>(0,0,0, p.scale[0]/2.f, p.scale[1]/2.f,
                                            p.scale[2]/2.f, (int)slices, (int)slices);
        break;
      case P::CYLINDER:
        node = CylinderNode::create(0,0,0, p.scale[0], p.scale[1], p.scale[2], (int)slices);
        break;
      default:
        return nullptr;
    }
    node->setMaterial(viz3d::Material::fromColor(color));
    node->setTransformation(mat);
    return node;
  }

} // namespace icl::viz3d
