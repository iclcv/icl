/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/SimplexOptimizer.cpp               **
** Module : ICLMath                                                **
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

#include <ICLMath/SimplexOptimizer.h>
#include <vector>
#include <limits>
#include <algorithm>
#include <functional>
#include <iostream>
#include <ICLUtils/StringUtils.h>

using namespace icl::utils;

namespace icl{
  namespace math{
  
    
    template<class Vector>
    static inline Vector create_zero_vector(int dim){
      return Vector(dim,0.0);
    }
    template<> inline FixedColVector<float,3> create_zero_vector(int dim){
      return FixedColVector<float,3>(0.0f);
    }
  
    template<class Vector>
    static inline unsigned int vdim(const Vector &v){ return v.dim(); }
    template<> unsigned int vdim(const std::vector<float> &v){ return v.size(); }
    template<> unsigned int vdim(const std::vector<double> &v){ return v.size(); }
        
    template<class T, class Vector>
    inline void check_whether_check_dim_is_ok_throw() {
      throw ICLException("SimplexOptimizer::setDim(..) cannnot be called for fixed matrix/vector types");    
    }
    
    template<> void check_whether_check_dim_is_ok_throw<float,std::vector<float> >() {}
    template<> void check_whether_check_dim_is_ok_throw<float,DynMatrix<float> >() {}
    template<> void check_whether_check_dim_is_ok_throw<float,DynColVector<float> >() {}
    template<> void check_whether_check_dim_is_ok_throw<float,DynRowVector<float> >() {}
  
    template<> void check_whether_check_dim_is_ok_throw<double,std::vector<double> >() {}
    template<> void check_whether_check_dim_is_ok_throw<double,DynMatrix<double> >() {}
    template<> void check_whether_check_dim_is_ok_throw<double,DynColVector<double> >() {}
    template<> void check_whether_check_dim_is_ok_throw<double,DynRowVector<double> >() {}
  
  
    template<class T, class Vector>
    static T vector_distance(const Vector &a, const Vector &b){
      T r = 0;
      for(unsigned int i=0;i<vdim(a);++i) r += sqr(a[i]-b[i]);
      return sqrt(r);
    }
    
    template<class T, class X> 
    static std::vector<T> operator*(const std::vector<T> &v, X t){
      std::vector<T> r(v.size());
      for(unsigned int i=0;i<vdim(v);++i) r[i] = t*v[i];
      return r;
    }
  
    template<class T> 
    static std::vector<T> operator+=(std::vector<T> &a,const std::vector<T> &b){
      for(unsigned int i=0;i<vdim(a);++i) a[i]+=b[i];
      return a;
    }
    
    template<class T, class Vector>
    struct SimplexOptimizer<T,Vector>::Data{
      Data(error_function f, int dim, int iterations, T minError, T minDelta, T a, T b, T g, T h):
        dim(dim),num(dim+1),f(f),x(dim+1),
        iterations(iterations),minError(minError),minDelta(minDelta),
        center(create_zero_vector<Vector>(dim)),
        centerOld(create_zero_vector<Vector>(dim)),
        fx(num,0),
        a(a),b(b),g(g),h(h),
        xg(create_zero_vector<Vector>(dim)),
        xr(create_zero_vector<Vector>(dim)),
        xe(create_zero_vector<Vector>(dim)),
        xc(create_zero_vector<Vector>(dim))
      {
        
      }
      int dim; // data dimensionality
      int num; // number of vertices (dim+1)
      error_function f;   //!< error function that has to be minimized
      std::vector<Vector> x;  //!< list of simplex vertices (size: dim+1)
      
      int iterations;  // maximum iteration count
      T minError; // min error
      T minDelta; // min delta
      Vector center; // current centroid
      Vector centerOld; // last centroid
      std::vector<T> fx; // f(x) ( size num )
      T a; // relection factor
      T b; // expansion factor
      T g; // contraction factor
      T h; // full contraction factor
      
      Vector xg; // reflection center
      Vector xr; // reflection vector
      Vector xe; // expansion vector
      Vector xc; // contraction vector
      
      /// optional iteration callback structure
      SimplexOptimizer<T,Vector>::iteration_callback iteration_callback;
    };
  
    
  
    
    
    template<class T, class Vector>
    SimplexOptimizer<T,Vector>::SimplexOptimizer(error_function f, 
                                                 int dim, 
                                                 int iterations, 
                                                 T minError, 
                                                 T minDelta,
                                                 T a, T b, T g, T h):
      m_data(new Data(f,dim,iterations,minError,minDelta,a,b,g,h)){
      
     
    }
    
    template<class T, class Vector>
    int SimplexOptimizer<T,Vector>::getDim() const { return m_data->dim; }
  
    template<class T, class Vector>
    void SimplexOptimizer<T,Vector>::setDim(int dim){
      check_whether_check_dim_is_ok_throw<T,Vector>();
      Data *old = m_data;
      m_data = new Data(old->f, dim, old->iterations, old->minError, old->minDelta,
                        old->a, old->b, old->g,old->h);
      delete old;
    }
    
    template<class T, class Vector>
    std::vector<Vector>  SimplexOptimizer<T,Vector>::createDefaultSimplex(const Vector &init){
      std::vector<Vector> r(vdim(init)+1);
      for(unsigned int i=0;i<r.size()-1;++i){
        r[i] = init;
        r[i][i] *= 1.05;
      }
      r.back() = init;
      return r;
    }
    
    template<class T, class Vector>
    SimplexOptimizationResult<T,Vector> SimplexOptimizer<T,Vector>::optimize(const Vector &init){
      for(int i=0; i<m_data->dim; ++i){
        m_data->x[i] = init;
        m_data->x[i][i] *= 1.05;
      }
      m_data->x.back() = init;
      m_data->centerOld = init * (m_data->num);
     
      return optimize(m_data->x);
    }
    template<class T, class Vector>
    SimplexOptimizationResult<T,Vector> SimplexOptimizer<T,Vector>::optimize(const std::vector<Vector> &init){
      if(&init != &m_data->x){
        ICLASSERT_THROW((int)init.size() == m_data->num, ICLException(str(__FUNCTION__)+": invalid count of initial vertices"));
        std::copy(init.begin(),init.end(),m_data->x.begin());
        m_data->centerOld = m_data->x[0];
        for(int i=1;i<m_data->num;++i){
          m_data->centerOld += m_data->x[i];
        }
      }
      int currentIteration=0; //iteration step number
      int idxBest=0, idxWorst=0, idx2ndWorst=0;
      for(int i=0;i<m_data->num;++i){
        m_data->fx[i] = m_data->f(m_data->x[i]);
      }
      
      //optimization begins
      for(currentIteration=0; currentIteration<m_data->iterations; ++currentIteration){
        // search for best, worst and 2ndWorst element
        idxBest = (int)(std::min_element(m_data->fx.begin(),m_data->fx.end())-m_data->fx.begin());
        T &fBest = m_data->fx[idxBest];
        if(fBest < m_data->minError) break;
        idxWorst = (int)(std::max_element(m_data->fx.begin(),m_data->fx.end())-m_data->fx.begin());
        T fxWorst = m_data->fx[idxWorst];
        m_data->fx[idxWorst] = std::numeric_limits<T>::min();
        idx2ndWorst = (int)(std::max_element(m_data->fx.begin(),m_data->fx.end())-m_data->fx.begin());
        m_data->fx[idxWorst] = fxWorst;
  
        Vector &xWorst = m_data->x[idxWorst];
        Vector &xBest = m_data->x[idxBest];
        T &fWorst = m_data->fx[idxWorst];
  
        
        // find reflection point m_data->xg and update the simplex-center
        std::fill(m_data->xg.begin(),m_data->xg.end(),0);
        for(unsigned int i=0; i<m_data->x.size(); ++i){
          if((int)i!=idxWorst) m_data->xg += m_data->x[i];
        }
        
        for(int i=0;i<m_data->dim;++i){
          m_data->center[i] = m_data->xg[i] + xWorst[i];
          m_data->xg[i] /= m_data->dim;
        }
  
        // test for minError and abort if center did not move enough
        if(currentIteration > 0 && vector_distance<T,Vector>(m_data->centerOld,m_data->center) < m_data->minDelta) break;
        else m_data->centerOld = m_data->center;
        
        // apply the reflection: result m_data->xr
        for( int i=0; i<m_data->dim; ++i){
          m_data->xr[i]=m_data->xg[i] + m_data->a*(m_data->xg[i]-xWorst[i]);
        }
        
        T fxr=m_data->f(m_data->xr);//record function at m_data->xr
        
        if(fBest<=fxr && fxr<=m_data->fx[idx2ndWorst]){ //------------> use reflection
          xWorst = m_data->xr;
          fWorst = fxr;
        }else if(fxr<fBest){ //---------------------------------> apply expansion
          for( int i=0; i<m_data->dim; ++i){
            m_data->xe[i]=m_data->xr[i]+m_data->b*(m_data->xr[i]-m_data->xg[i]);
          }
          T fxe = m_data->f(m_data->xe);
          xWorst = fxe < fxr ? m_data->xe : m_data->xr;
          fWorst = fxe < fxr ? fxe : fxr;
        } else if( fxr > m_data->fx[idx2ndWorst] ){ //----------------> apply contraction
          for( int i=0; i<m_data->dim; ++i){
            m_data->xc[i]=m_data->xg[i]+m_data->g*(xWorst[i]-m_data->xg[i]);
          }
          T fxc = m_data->f(m_data->xc);
          if( fxc < fWorst ){
            xWorst = m_data->xc;
            fWorst = fxc;
          }
          else{ //----------------------------------------------> multiple contraction
            for(unsigned int i=0; i<m_data->x.size(); ++i ){
              if( (int)i!=idxBest ){ 
                for(int j=0; j<m_data->dim; ++j){
                  m_data->x[i][j] = xBest[j] + m_data->h * ( m_data->x[i][j]-xBest[j] );
                }
                m_data->fx[i] = m_data->f(m_data->x[i]);
              }
            }	  
          }
        }
        if(m_data->iteration_callback){
          const Result result = { m_data->x[idxBest], m_data->fx[idxBest], currentIteration, m_data->x };
          m_data->iteration_callback(result);
        }
      } 
      Result result = { m_data->x[idxBest], m_data->fx[idxBest], currentIteration, m_data->x };
      return result;
    }
    
  
    template<class T, class Vector> void  SimplexOptimizer<T,Vector>::setA(T a){
      m_data->a = a;
    }
    template<class T, class Vector> void  SimplexOptimizer<T,Vector>::setB(T b){
      m_data->b = b;
    }
    template<class T, class Vector> void  SimplexOptimizer<T,Vector>::setG(T g){
      m_data->g = g;
    }
    template<class T, class Vector> void  SimplexOptimizer<T,Vector>::setH(T h){
      m_data->h = h;
    }
    template<class T, class Vector> void  SimplexOptimizer<T,Vector>::setIterations(int iterations){
      m_data->iterations = iterations;
    }
    template<class T, class Vector> void  SimplexOptimizer<T,Vector>::setMinError(T minError){
      m_data->minError = minError;
    }
    template<class T, class Vector> void  SimplexOptimizer<T,Vector>::setMinDelta(T minDelta){
      m_data->minDelta = minDelta;
    }
    template<class T, class Vector> 
    void  SimplexOptimizer<T,Vector>::setErrorFunction(SimplexOptimizer<T,Vector>::error_function f){
      m_data->f = f;
    }
    
    template<class T, class Vector> T  SimplexOptimizer<T,Vector>::getA() const {
      return m_data->a; 
    }
    template<class T, class Vector>T  SimplexOptimizer<T,Vector>::getB() const{
      return m_data->b;
    }
    template<class T, class Vector>T  SimplexOptimizer<T,Vector>::getG() const{
      return m_data->g;
    }
    template<class T, class Vector>T  SimplexOptimizer<T,Vector>::getH() const{
      return m_data->h;
    }
    template<class T, class Vector>int  SimplexOptimizer<T,Vector>::getIterations() const{
      return m_data->iterations;
    }
    template<class T, class Vector>T  SimplexOptimizer<T,Vector>::getMinError() const{
      return m_data->minError;
    }
    template<class T, class Vector>T  SimplexOptimizer<T,Vector>::getMinDelta() const{
      return m_data->minDelta;
    }
  
    template<class T, class Vector>  
    Function<T,const Vector&> SimplexOptimizer<T,Vector>::getErrorFunction() const{
      return m_data->f;
    }
    
    template<class T, class Vector>
    void SimplexOptimizer<T,Vector>::setIterationCallback(const SimplexOptimizer<T,Vector>::iteration_callback &cb){
      m_data->iteration_callback = cb;
    }
  
    
    template class SimplexOptimizer<float,DynColVector<float> >;
    template class SimplexOptimizer<double,DynColVector<double> >;
  
    template class SimplexOptimizer<float,DynRowVector<float> >;
    template class SimplexOptimizer<double,DynRowVector<double> >;
  
    template class SimplexOptimizer<float,DynMatrix<float> >;
    template class SimplexOptimizer<double,DynMatrix<double> >;
  
    template class SimplexOptimizer<float,std::vector<float> >;
    template class SimplexOptimizer<double,std::vector<double> >;
  
  
  #define INST(D)                                                       \
    template class SimplexOptimizer<float,FixedColVector<float,D> >;    \
    template class SimplexOptimizer<float,FixedRowVector<float,D> >;    \
    template class SimplexOptimizer<float,FixedMatrix<float,1,D> >;     \
    template class SimplexOptimizer<float,FixedMatrix<float,D,1> >;     \
    template class SimplexOptimizer<double,FixedColVector<double,D> >;  \
    template class SimplexOptimizer<double,FixedRowVector<double,D> >;  \
    template class SimplexOptimizer<double,FixedMatrix<double,1,D> >;   \
    template class SimplexOptimizer<double,FixedMatrix<double,D,1> >
  
    INST(2);
    INST(3); 
    INST(4);
    INST(5); 
    INST(6);
    //INST(7); 
    //INST(8);
    //INST(9);
    //INST(10);
  
  #undef INST
    
  } // namespace math
}
