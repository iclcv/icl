/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/PointCloudSegment.cpp              **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Andre Ueckermann                  **
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

#include <ICLGeom/PointCloudSegment.h>

namespace icl{
  namespace geom{

    PointCloudSegment::PointCloudSegment(int dim, bool withColors):
      PointCloudObject(dim,false,withColors),
      featuresComputed(false),
      numPoints(0)
    {
    }


    PointCloudSegment::PointCloudSegment(geom::PointCloudObject &obj):
      PointCloudObject(1,
                       obj.supports(PointCloudObjectBase::Normal),
                       obj.supports(PointCloudObjectBase::RGBA32f)), 
      featuresComputed(false){
      init(obj, std::vector<const std::vector<int>*>());
    }


    PointCloudSegment::PointCloudSegment(geom::PointCloudObject &obj, 
                                         const std::vector<int> &indices):
      PointCloudObject(1,
                       obj.supports(PointCloudObjectBase::Normal),
                       obj.supports(PointCloudObjectBase::RGBA32f)),
      featuresComputed(false){
      
      std::vector<const std::vector<int>*> v(1,&indices);
      init(obj, v);
    }

    
    PointCloudSegment::PointCloudSegment(geom::PointCloudObject &obj, 
                                         const std::vector<const std::vector<int>*> &indices):
      PointCloudObject(1,
                       obj.supports(PointCloudObjectBase::Normal),
                       obj.supports(PointCloudObjectBase::RGBA32f)),
      featuresComputed(false){
      init(obj,indices);
    }

    
    Mat PointCloudSegment::computeEigenVectorFrame() const throw (utils::ICLException){
      const math::Vec3 &x = eigenvectors[0];
      const math::Vec3 &y = eigenvectors[1];
      
      Vec z = math::cross(x.resize<1,4>(1), y.resize<1,4>(1));
      
      return Mat(x[0], y[0], z[0], mean[0],
                 x[1], y[1], z[1], mean[1],
                 x[2], y[2], z[2], mean[2],
                 0,0,0,1);
    }


    /// returns week pointer to the i-th child (already casted to PointCloudSegment type)
    utils::SmartPtr<PointCloudSegment>  PointCloudSegment::getSubSegment(int i) throw (utils::ICLException){
      if(i<0 || i >= getChildCount()){
        throw utils::ICLException("PointCloudSegment:getSubSegment: invalid index!");
      }
      utils::SmartPtr<SceneObject> obj = getChildPtr(i);
      if(!obj){
        throw utils::ICLException("PointCloudSegment:getSubSegment: selected sub-segment is null (which should not happen)");
      }
      PointCloudSegment *p = dynamic_cast<PointCloudSegment*>(obj.get());
      if(!p) throw utils::ICLException("PointCloudSegment::getSubSegment: "
                                       "given child index does not refer to an actual "
                                       "PointCloudSegment instance! (to avoid undefined behaviour "
                                       "in further processing steps, this exception is thrown)");
      return obj; // implicit smart-pointer cast should carry referrence conter
      //explicit cast//utils::SmartPtr<PointCloudSegment>(p,false);
    }
    

    static inline void copy_util(core::DataSegment<float,4> s, core::DataSegment<float,4> d, 
                                 const std::vector<const std::vector<int> *> &sidxs){
      if(!sidxs.empty()){
        int k = 0;
        for(size_t i=0;i<sidxs.size();++i){
          const std::vector<int> &v = *sidxs[i];
          for(size_t j=0;j<v.size();++j){
            d[k++] = s[v[j]];
          }
        }    
      }else{
        s.deepCopy(d);
      }
    }


    void PointCloudSegment::updateFeatures(){
      typedef math::FixedMatrix<float,3,3> Mat3;
      typedef math::FixedColVector<float,3> Vec3;      
      if(featuresComputed) return;

      math::Vec3 m(0,0,0);
      math::Mat3 C(0,0,0, 0,0,0, 0,0,0);
      AABB aabb = { Vec3(1.e38, 1.e38, 1.e38), Vec3(-1.e38, -1.e38, -1.e38) };
      size_t n = 0;

      if(isParent()){ 
        // update features of children first
        for(int i=0;i<getNumSubSegments();++i){
          getSubSegment(i)->updateFeatures();
        }
        for(int i=0;i<getNumSubSegments();++i){
          PointCloudSegmentPtr child = getSubSegment(i);
          const size_t nc = child->getDim();

          // update bounding box
          const AABB &a = child->aabb;
          for(int j=0;j<3;++j){
            if(a.min[j] < aabb.min[j]) aabb.min[j] = a.min[j];
            if(a.max[j] > aabb.max[j]) aabb.max[j] = a.max[j];
          }

          // compute moment matrix xx of child
          math::Mat3 xx (child->covariance);
          const math::Vec3& mc = child->mean;
          for(int y=0;y<3;++y){
            for(int x=0;x<3;++x){
              xx(x,y) += mc[x] * mc[y];
            }
          }
          // update moments + mean
          C += xx * float(nc);
          m += child->mean * float(nc);
          n += nc;
        }
      }else{
        // compute features!!
        n = getDim();
        
        core::DataSegment<float,3> xyz = selectXYZ();
        
        for(size_t i=0;i<n;++i){
          const math::Vec3 &v = xyz[i];
          // compute bounding box
          for(int d=0;d<3;++d){
            if(v[d] < aabb.min[d]) aabb.min[d] = v[d];
            if(v[d] > aabb.max[d]) aabb.max[d] = v[d];
          }
          // compute moments
          for(int y=0;y<3;++y){
            for(int x=0;x<3;++x){
              C(x,y) += v[x] * v[y];
            }
          }
          // compute mean
          m += v;
        }
      }
      if (n > 0) {
        m *= 1./n;
        C *= 1./n;
      }

      this->numPoints = n;
      this->mean = m;
      this->aabb = aabb;
      // covariance = <xx^t> - mm^t
      for(int y=0;y<3;++y){
        for(int x=0;x<3;++x){
          C(x,y) -= m[x] * m[y];
        }
      }
      this->covariance = C;
      
      // compute eigenvectors of covariance C
      try{
        Mat3 evec;
        Vec3 eval;
        C.eigen(evec,eval);
          
        this->eigenvectors[0] = evec.col(0);
        this->eigenvectors[1] = evec.col(1);
        this->eigenvectors[2] = evec.col(2);
          
        this->eigenvalues = eval;
      }catch(utils::ICLException &e){
        Vec3 vecZero(0,0,0);
        this->eigenvalues = Vec3(1,1,1);
          
        this->mean = (aabb.min + aabb.max)*0.5;
        for(int i=0;i<3;++i){
          this->eigenvectors[i] = vecZero;
          this->eigenvectors[i][i] = 1;
        }
      }
      featuresComputed = true;
    }

    
    void PointCloudSegment::init(geom::PointCloudObject &obj, 
                                 const std::vector<const std::vector<int> *> &indices){
      
      if(!indices.empty()){
        int n=0;
        for(size_t i=0;i<indices.size();++i){
          n+= indices[i]->size();
        }   
        setSize(utils::Size(n,1));
      }else{
        setSize(obj.getSize()); // we leave it organized!
      }

      
      copy_util(obj.selectXYZH(), selectXYZH(), indices);
      if(supports(Normal)){
        copy_util(obj.selectNormal(), selectNormal(), indices);
      }
      if(supports(RGBA32f)){
        copy_util(obj.selectRGBA32f(), selectRGBA32f(), indices);
      }
      
      updateFeatures();
    }
    
    PointCloudSegment *PointCloudSegment::copy() const{
      return new PointCloudSegment(*this);
    }


    static void count_points_recursive(PointCloudSegmentPtr p, int &n){
      if(p->isParent()){
        for(int i=0;i<p->getNumSubSegments();++i){
          count_points_recursive(p->getSubSegment(i),n);
        }
      }else{
        n += p->getDim();
      }
    }


    static void copy_points_recursive(PointCloudSegmentPtr p, PointCloudSegmentPtr dst, int &offs){
      if(p->isParent()){
        for(int i=0;i<p->getNumSubSegments();++i){
          copy_points_recursive(p->getSubSegment(i),dst,offs);
        }
      }else{
        const core::DataSegment<float,4> sxyz = p->selectXYZH();
        const core::DataSegment<float,4> srgb = p->selectRGBA32f();

        core::DataSegment<float,4> dxyz = dst->selectXYZH();
        core::DataSegment<float,4> drgb = dst->selectRGBA32f();

        for(int i=0;i<sxyz.getDim();++i,++offs){
          dxyz[offs] = sxyz[i];
          drgb[offs] = srgb[i];
        }
      }
    }
 
    
    static PointCloudSegmentPtr flatten_segment_hierarcy(PointCloudSegmentPtr p){
      int n = 0;
      count_points_recursive(p,n);
      PointCloudSegmentPtr c = new PointCloudSegment(n,true);
      int offs = 0;
      copy_points_recursive(p, c, offs);
      c->mean = p->mean;
      c->eigenvectors[0] = p->eigenvectors[0];
      c->eigenvectors[1] = p->eigenvectors[1];
      c->eigenvectors[2] = p->eigenvectors[2];
      c->eigenvalues = p->eigenvalues;
      c->numPoints = p->numPoints;
      c->aabb = p->aabb;
      return c;
    }

    utils::SmartPtr<PointCloudSegment> PointCloudSegment::flatten() const{
      PointCloudSegmentPtr p(const_cast<PointCloudSegment*>(this),false);
      return flatten_segment_hierarcy(p);
    }


    static void collect_children_recursive(std::vector<PointCloudSegmentPtr> &v, PointCloudSegmentPtr seg){
      v.push_back(seg);
      if(seg->isParent()){
        for(int i=0;i<seg->getNumSubSegments();++i){
          collect_children_recursive(v,seg->getSubSegment(i));
        }
      }
    }
    
    /// returns a vector containing this and all recursive children
    std::vector<PointCloudSegmentPtr> PointCloudSegment::extractThisAndChildren() const{
      std::vector<PointCloudSegmentPtr> v;
      collect_children_recursive(v,PointCloudSegmentPtr(const_cast<PointCloudSegment*>(this),false));
      return v;
    }

  }
}
