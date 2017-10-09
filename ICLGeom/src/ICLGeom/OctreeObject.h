/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/OctreeObject.h                     **
** Module : ICLGeom                                                **
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

#include <ICLUtils/CompatMacros.h>
#include <ICLMath/Octree.h>
#include <ICLGeom/SceneObject.h>
#include <ICLGeom/PointCloudObjectBase.h>
#include <ICLGeom/Camera.h>


#ifdef ICL_SYSTEM_APPLE
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#elif WIN32
#define NOMINMAX
#include <Windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif

namespace icl{
  namespace geom{

    /** \cond */
    ICLGeom_API void octree_object_render_box(float x0, float y0, float z0,
                                              float x1, float y1, float z1);

    template<class Scalar, int CAPACITY, int SF, class Pt, int ALLOC_CHUNK_SIZE>
    struct OctreePointRenderer{
      typedef typename math::Octree<Scalar,CAPACITY,SF,Pt,ALLOC_CHUNK_SIZE>::Node Node;
      static void render(const Node *node){
        glBegin(GL_POINTS);
        for(const Pt *p = node->points; p < node->next;++p){
          glVertex3f( (*p)[0],(*p)[1],(*p)[2]);
        }
        glEnd();
      }
    };

#if 0
    /// this optimization is not working: Why?
    template<int CAPACITY, int SF, int ALLOC_CHUNK_SIZE>
    struct OctreePointRenderer<float,CAPACITY,SF,math::FixedColVector<float,4>,ALLOC_CHUNK_SIZE>{
      typedef FixedColVector<float,4> Pt;
      typedef typename math::Octree<float,CAPACITY,SF,Pt,ALLOC_CHUNK_SIZE>::Node Node;
      static void render(const Node *node){
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(4,GL_FLOAT,0,node->points);
        glDrawArrays(GL_POINTS, 0, (int)(node->next - node->points));
        glDisableClientState(GL_VERTEX_ARRAY);
      }
    };

    template<int CAPACITY, int SF, int ALLOC_CHUNK_SIZE>
    struct OctreePointRenderer<icl32s,CAPACITY,SF,math::FixedColVector<icl32s,4>,ALLOC_CHUNK_SIZE>{
      typedef FixedColVector<icl32s,4> Pt;
      typedef typename math::Octree<icl32s,CAPACITY,SF,Pt,ALLOC_CHUNK_SIZE>::Node Node;
      static void render(const Node *node){
        glEnableClientState(GL_VERTEX_ARRAY);
        glVertexPointer(4,GL_INT,0,node->points);
        glDrawArrays(GL_POINTS, 0, (int)(node->next - node->points));
        glDisableClientState(GL_VERTEX_ARRAY);
      }
    };
#endif

    /** \endcond */



    /// The OctreeObjects provides a visualizable SceneObject interface for the Octree class
    /** \section Insertion Speed

        Please note that the insertion speed heavily depends on the data that is actually
        inserted. A very common problem is an incredible slow insertion of Kinect-Point clouds.
        The main reason for this is the fact that Kinect's depth image contains many holes
        with depth value 0.0, originated by -- for the device -- invisible parts of the scene.
        These holes in point-cloud points that are all exactly at the camera center. \n
        However, a large set of points at the same location leads to a very deep Octree node
        structure which affects the insertion performance extremely negatively.

        In order to avoid this, it is recommended to use a point filter, most commonly the
        internal class OctreeObject::PointFilter is sufficient here, when inserting points from
        another point cloud into an octree.

        Please note: another very common misconception is an issue with low speed of debug
        builds. As a matter of fact, in particular structures as the OctreeObject profit
        tremendously from the automatic compiler optimizations, so between "-O0 -g3" and
        "-O3 -g0 -march=native" a speedup factor of 10 is not very unlikely.

        As a general guideline, the insertion of a full QVGA (320x240) point cloud from a kinect
        device (employing the PointFilter to avoid insertion thousands of camera center points)
        takes about 5ms on a mobile Core i7 (Intel(R) Core(TM) i7-4700MQ CPU @ 2.40GHz)
     */

    template<class Scalar, int CAPACITY=4, int SF=32, class Pt=math::FixedColVector<Scalar,4>, int ALLOC_CHUNK_SIZE=1024>
    class OctreeObject : public math::Octree<Scalar,CAPACITY,SF,Pt,ALLOC_CHUNK_SIZE>, public SceneObject{

      /// typedef to the parent class type
      typedef math::Octree<Scalar,CAPACITY,SF,Pt, ALLOC_CHUNK_SIZE> Parent;
      bool m_renderPoints;     //!< flag whether points are rendered as well
      bool m_renderBoxes;      //!< flag whether aabb boxes are rendered
      GeomColor m_pointColor;  //!< color used for the points (if rendered)
      GeomColor m_boxColor;    //!< color used for the aabb-boxes

      protected:
      void init(){
        m_renderPoints = false;
        m_renderBoxes = true;
        m_pointColor = GeomColor(0,0.5,1,1);
        m_boxColor = GeomColor(0,1,0,0.3);
        setLockingEnabled(true);
      }

      public:

      /// create OctreeObject from given axis-aligned bounding box
      OctreeObject(const Scalar &minX, const Scalar &minY, const Scalar &minZ,
                   const Scalar &width, const Scalar &height, const Scalar &depth):
      Parent(minX,minY,minZ,width,height,depth){
        init();
        SceneObject::setLockingEnabled(true);
      }

      /// create OctreeObject from given cubic axis-aligned bounding box
      OctreeObject(const Scalar &min, const Scalar &len):Parent(min,len){
        init();
      }

      /// Adds all points from the given point cloud object to the octree
      /** this only works for Pt == Vec, Internally SceneObject::lock/unlock
          is used to avoid issues during update */
      void fill(const PointCloudObjectBase &obj, bool clearBefore=true){
        SceneObject::lock();
        if(clearBefore) Parent::clear();
        if(obj.supports(PointCloudObjectBase::XYZH)){
          const core::DataSegment<float,4> xyzh = obj.selectXYZH();
          for(int i=0;i<xyzh.getDim();i+=1){
            Parent::insert(xyzh[i]);
          }
        }else if(obj.supports(PointCloudObjectBase::XYZ)){
          const core::DataSegment<float,3> xyz = obj.selectXYZ();
          for(int i=0;i<xyz.getDim();i+=1){
            Parent::insert(xyz[i].resize<1,4>(1));
          }
        }
        SceneObject::unlock();
      }

      /// Adds all points from the given point cloud object to the octree
      /** this only works for Pt == Vec, Internally SceneObject::lock/unlock
          is used to avoid issues during update. Supporting a filtering function that
          provides both bool operator()(const Vec&) const
          and bool operator()(const Vec3 &) const.

          A most common filter is provided as an internal class: PointFilter
          Which is able to
          */
      template<class Filter>
      void fill(const PointCloudObjectBase &obj, Filter f, bool clearBefore){
        SceneObject::lock();
        if(clearBefore) Parent::clear();
        if(obj.supports(PointCloudObjectBase::XYZH)){
          const core::DataSegment<float,4> xyzh = obj.selectXYZH();
          for(int i=0;i<xyzh.getDim();i+=1){
            if(f(xyzh[i])){
              Parent::insert(xyzh[i]);
            }
          }
        }else if(obj.supports(PointCloudObjectBase::XYZ)){
          const core::DataSegment<float,3> xyz = obj.selectXYZ();
          for(int i=0;i<xyz.getDim();i+=1){
            if(f(xyz[i])){
              Parent::insert(xyz[i].resize<1,4>(1));
            }
          }
        }
        SceneObject::unlock();
      }


      /// Internal most common fill-Filter to filter out single points
      /** The single point that is usually filtered out is the camera center.
          Therefore, two constructors are provided. One that can be
          arbitrarily defined and another one that uses a camera's position
          and it's focal length as distance threshold directly

          In other words, the filter filters our an epsilon sphere around
          the given center point, where epsilon is the given radius or
          the given camera's focal length */
      struct PointFilter {
        math::Vec3 pos;   //!< position to filter out
        float  sqrRadius; //!< squared distance threshold to the filter point

        /// Create point filter with given center and radius
        PointFilter(const Vec &p, float radius=10){
          pos = p.resize<1,3>();
          sqrRadius = radius*radius;
        }

        /// Create point filter with given camera
        /** Internally uses the Camera's position and its focal length as
            radius*/
        PointFilter(const geom::Camera &cam){
          pos = cam.getPosition().resize<1,3>();
          this->sqrRadius = utils::sqr(cam.getFocalLength());
        }

        /// actual filter function
        /** realized as an inline template */
        template<int N>
        inline bool operator()(const math::FixedColVector<float,N> &v) const{
          return ( ( sqr(v[0]-pos[0]) +
                     sqr(v[1]-pos[1]) +
                     sqr(v[2]-pos[2]) ) > sqrRadius);
        }
      };


      /// sets whether points are rendered as well
      /** Please not, that the point rendering of the OctreeObject is less efficient
          the the point-rendering used in SceneObject or in the PointCouldObjectBase
          sub-classes. Furthermore, all points are rendered with the same color */
      void setRenderPoints(bool enabled) {
        m_renderPoints = enabled;
      }

      /// return whether points are rendered as well
      bool getRenderPoints() const {
        return m_renderPoints;
      }

      /// sets whether aabbs are to be rendered (default: true)
      void setRenderBoxes(bool enabled) {
        m_renderBoxes= enabled;
      }

      /// return whether aabbs are rendered
      bool getRenderBoxes() const {
        return m_renderBoxes;
      }

      /// sets the color used for boxes (default is semi-transparent green)
      void setBoxColor(const GeomColor &color){
        m_boxColor = color/255;
      }

      /// returns the box color
      GeomColor getBoxColor() const{
        return m_boxColor*255;
      }

      /// sets the color used for rendering ppoints (if point rendering is activated)
      /** The default  point rendering color is (0,128,255,255) */
      void setPointColor(const GeomColor &color){
        m_pointColor = color/255;
      }

      /// returns the point rendering color
      GeomColor getPointColor() const{
        return m_pointColor*255;
      }

      /// adapted customRenderMethod
      virtual void customRender(){
        if(!m_renderPoints && !m_renderBoxes) return;
        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();

        glScalef(1./SF,1./SF,1./SF);

        GLboolean lightWasOn = true;
        glGetBooleanv(GL_LIGHTING,&lightWasOn);
        glDisable(GL_LIGHTING);

        glPointSize(m_pointSize);
        if(m_renderPoints){
          renderNodeWithPoints(Parent::root);
        }else{
          glColor4fv(m_boxColor.data());
          renderNode(Parent::root);
        }
        if(lightWasOn){
          glEnable(GL_LIGHTING);
        }

        glPopMatrix();
      }

      protected:

      /// utility function to render AABB-boxes
      void box(const typename Parent::AABB &bb) const {
        const Pt &c = bb.center, s = bb.halfSize;
        octree_object_render_box(c[0] - s[0],c[1] - s[1],c[2] - s[2],
                                 c[0] + s[0],c[1] + s[1],c[2] + s[2]);

      }

      /// recursive render function rendering a node's AABB and its points
      void renderNodeWithPoints(const typename Parent::Node *node) const  {
        if(m_renderBoxes){
          glColor4fv(m_boxColor.data());
          box(node->boundary);
        }

        glColor4fv(m_pointColor.data());
        OctreePointRenderer<Scalar,CAPACITY,SF,Pt,ALLOC_CHUNK_SIZE>::render(node);

        if(node->children){
          for(int i=0;i<8;++i){
            renderNodeWithPoints(node->children+i);
          }
        }
      }

      /// recursive render function rendering a node's AABB only
      void renderNode(const typename Parent::Node *node) const{
        box(node->boundary);
        if(node->children){
          for(int i=0;i<8;++i){
            renderNode(node->children+i);
          }
        }
      }
    };

  }

}
