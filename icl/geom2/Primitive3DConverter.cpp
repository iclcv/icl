// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include <icl/geom2/Primitive3DConverter.h>
#include <icl/geom2/GeometryNode.h>
#include <icl/geom2/CuboidNode.h>
#include <icl/geom2/SphereNode.h>
#include <icl/geom2/CylinderNode.h>
#include <icl/geom/Material.h>

namespace icl::geom2 {

  NodePtr nodeFromPrimitive3D(const geom::Primitive3DFilter::Primitive3D &p,
                              uint32_t slices, const geom::GeomColor &color) {
    using P = geom::Primitive3DFilter;

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
    node->setMaterial(geom::Material::fromColor(color));
    node->setTransformation(mat);
    return node;
  }

} // namespace icl::geom2
