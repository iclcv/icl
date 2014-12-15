/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/Primitive3DFilter.h                **
** Module : ICLGeom                                                **
** Authors: Lukas Twardon                                          **
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

#include <ICLGeom/PointCloudObject.h>
#include <ICLGeom/SceneObject.h>

#ifdef ICL_HAVE_OPENCL
#include "ICLUtils/CLProgram.h"
#endif

namespace icl{
    namespace geom{

        /// Class for filtering point clouds according to a set of primitives and a filter configuration
        class Primitive3DFilter {

        public:

            /// the primitive type
            enum PrimitiveType {
                CUBE,
                SPHERE,
                CYLINDER
            };

            /// quaternion describing the orientation of the primitive
            struct Quaternion {

                /// Constructor
                /**
                    @param v vector part of the Quaternion
                    @param w scalar part of the quaternion
                    @param givenInAxisAngle if v and w are given in axis angle representation (flag is true), they are converted to a unit quaternion
                */
                Quaternion(const math::Vec3 &v, const float w, bool givenInAxisAngle = false) {
                    if(givenInAxisAngle) {
                        math::Vec3 vn = v.normalized();
                        float s = sin(w/2.0);
                        this->v[0] = vn[0] * s;
                        this->v[1] = vn[1] * s;
                        this->v[2] = vn[2] * s;
                        this->w = cos(w/2.0);
                        if(vn.length() <= 0.0001)
                            this->w = 1;
                    } else {
                        this->v = v;
                        this->w = w;
                    }
                }

                /// Hamilton product
                const Quaternion operator*(const Quaternion &q) {
                    Quaternion res(q);
                    res.w    = w*q.w    - v[0]*q.v[0] - v[1]*q.v[1] - v[2]*q.v[2];
                    res.v[0] = w*q.v[0] + v[0]*q.w    + v[1]*q.v[2] - v[2]*q.v[1];
                    res.v[1] = w*q.v[1] - v[0]*q.v[2] + v[1]*q.w    + v[2]*q.v[0];
                    res.v[2] = w*q.v[2] + v[0]*q.v[1] - v[1]*q.v[0] + v[2]*q.w;
                    return res;
                }

                /// complex conjugate
                Quaternion conj() {
                    Quaternion res(*this);
                    res.v *= -1;
                    return res;
                }

                /// convert to transformation matrix
                math::Mat4 getTransformationMatrix() {
                    float x = v[0];
                    float y = v[1];
                    float z = v[2];
                    float xx = x*x;
                    float yy = y*y;
                    float zz = z*z;
                    return math::Mat4 (1-2*yy-2*zz, 2*x*y-2*w*z, 2*x*z+2*w*y, 0,
                                       2*x*y+2*w*z, 1-2*xx-2*zz, 2*y*z-2*w*x, 0,
                                       2*x*z-2*w*y, 2*y*z+2*w*x, 1-2*xx-2*yy, 0,
                                       0,0,0,1);
                }

                /// rotate a given vector according to the quaternion
                /** Assumes a unit quaternion.
                    @param vIn input vector
                    @return rotated vector
                */
                math::Vec3 rotateVector(const math::Vec3 &vIn) {
                    Quaternion qIn(vIn, 0);
                    Quaternion qOut = qIn * this->conj();
                    qOut = (*this) * qOut;
                    return qOut.v;
                }

                /// vector part
                math::Vec3 v;

                /// scalar part
                float w;

            };

            /// a primitive
            struct Primitive3D {

                /// Constructor
                Primitive3D(const PrimitiveType &type, const Vec &position, const Quaternion &orientation,
                            const Vec &scale, unsigned long timestamp, const std::string &description = "") :
                    type(type), position(position), orientation(orientation), scale(scale), timestamp(timestamp), description(description) {

                }

                /// renders this 3D primitive into the scene object (by adding a child object to SceneObject *object)
                void toSceneObject(SceneObject *object, uint slices = 15, GeomColor const &color = geom_white(100));

                /// the type of primitive
                PrimitiveType type;

                /// position of the primitive center
                Vec position;

                /// orientation of the primitive
                Quaternion orientation;

                /// scale (e.g., [2r 2r 2r] for a sphere with radius r)
                Vec scale;

                /// timestamp of creation
                unsigned long timestamp;

                /// string describing the object (e.g., a robot link)
                std::string description;

                /// internal group id (see PrimitiveGroup)
                unsigned char groupBit;

            };

            /// a general filter action
            struct FilterAction {

                /// Default constructor
                FilterAction() {}

                /// Constructor
                FilterAction(std::vector<unsigned char> formula) : formula(formula) {}

                /// perform the actual filter action
                /**
                    @param pcObj the point cloud to be filtered
                    @param actionMap the action map specifying which points to filter out (may change in case points are actually removed)
                    @param groupMap may change in case points are actually removed
                    @param depthImage pointer to the depth image or 0 if no depth image
                */
                virtual void performAction(PointCloudObjectBase &pcObj, std::vector<unsigned char> &actionMap,
                                           std::vector<unsigned char> &groupMap, core::Img32f *depthImage) = 0;

                /// the filter action formula in disjunctive normal form (three bytes per atomic primitive group)
                /// first byte: 1 if this is the beginning of a new intersection, 0 otherwise
                /// second byte: 1 if the inner part of the primitive group should be filtered out, 0 for the inner part (negation)
                /// third byte: the group bit (internal group id which is set according to the id in filter config)
                std::vector<unsigned char> formula;

            };

            /// remove action
            struct RemoveAction : FilterAction {

                /// Constructor
                RemoveAction(std::vector<unsigned char> formula) : FilterAction(formula) {}

                void performAction(PointCloudObjectBase &pcObj, std::vector<unsigned char> &actionMap,
                                   std::vector<unsigned char> &groupMap, core::Img32f *depthImage);

            };

            /// setpos action
            struct SetposAction : FilterAction {

                /// Constructor
                SetposAction(std::vector<unsigned char> formula, float x, float y, float z) : FilterAction(formula), x(x), y(y), z(z) {}

                void performAction(PointCloudObjectBase &pcObj, std::vector<unsigned char> &actionMap,
                                   std::vector<unsigned char> &groupMap, core::Img32f *depthImage);

                /// parameters
                float x;
                float y;
                float z;

            };

            /// color action
            struct ColorAction : FilterAction {

                /// Constructor
                ColorAction(std::vector<unsigned char> formula, float r, float g, float b, float a) : FilterAction(formula), r(r), g(g), b(b), a(a) {}

                void performAction(PointCloudObjectBase &pcObj, std::vector<unsigned char> &actionMap,
                                   std::vector<unsigned char> &groupMap, core::Img32f *depthImage);

                /// parameters
                float r;
                float g;
                float b;
                float a;

            };

            /// label action
            struct LabelAction : FilterAction {

                /// Constructor
                LabelAction(std::vector<unsigned char> formula, icl32s value) : FilterAction(formula), value(value) {}

                void performAction(PointCloudObjectBase &pcObj, std::vector<unsigned char> &actionMap,
                                   std::vector<unsigned char> &groupMap, core::Img32f *depthImage);

                /// value
                icl32s value;

            };

            /// intensity action
            struct IntensityAction : FilterAction {

                /// Constructor
                IntensityAction(std::vector<unsigned char> formula, float value) : FilterAction(formula), value(value) {}

                void performAction(PointCloudObjectBase &pcObj, std::vector<unsigned char> &actionMap,
                                   std::vector<unsigned char> &groupMap, core::Img32f *depthImage);

                /// value
                float value;

            };

            /// filterdepthimg action
            struct FilterDepthImgAction : FilterAction {

                /// Constructor
                FilterDepthImgAction(std::vector<unsigned char> formula, float value) : FilterAction(formula), value(value) {}

                void performAction(PointCloudObjectBase &pcObj, std::vector<unsigned char> &actionMap,
                                   std::vector<unsigned char> &groupMap, core::Img32f *depthImage);

                /// value
                float value;

            };

            /// filter config
            struct FilterConfig {

                /// Constructor
                /** Parses the XML config.
                    @param filename the XML config file
                */
                FilterConfig(const std::string &filename) throw(utils::ParseException);

                /// map primitive group id to internal groupBit
                std::map<std::string, unsigned char> mapGroupIdToBit;

                /// map regular expression (matching the primitive description) to internal groupBit
                std::map<std::string, unsigned char> mapRegexToBit;

                /// all filter actions to be performed
                std::vector<utils::SmartPtr<FilterAction> > filterActions;

            };

            /// Constructor
            Primitive3DFilter(const FilterConfig &config);

            /// Destructor
            virtual ~Primitive3DFilter() {}

            /// set filter config
            void setConfig(const FilterConfig &config) {
                this->config = config;
            }

            /// applies the filter operation to the given point cloud
            /**
               @param primitives a vector of geometric primitives
               @param pcObj the point cloud to be filtered
               @param depthImage the optional depth image (only needed if there is a filterdepthimg tag in the config)
            */
            void apply(const std::vector<Primitive3D> &primitives, PointCloudObjectBase &pcObj, core::Img32f *depthImage = 0);

        private:

            #ifdef ICL_HAVE_OPENCL
            utils::CLProgram program;
            utils::CLBuffer pcbuffer, groupmapbuffer, actionmapbuffer, formulabuffer;
            utils::CLKernel kernelCreateGroupMap;
            utils::CLKernel kernelCreateActionMap;
            #endif

            static const char *KERNEL_CODE;
            FilterConfig config;

        };

    } // namespace geom
}
