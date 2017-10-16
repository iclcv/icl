/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLMath/src/ICLMath/LevenbergMarquardtFitter.cpp       **
** Module : ICLMath                                                **
** Authors: Christof Elbrechter, Sergius Gaulik                    **
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

#include <ICLMath/LevenbergMarquardtFitter.h>
#include <ICLMath/DynMatrixUtils.h>
#include <ICLUtils/Random.h>


using namespace icl::utils;

namespace icl{
  namespace math{

    template<class Scalar>
    LevenbergMarquardtFitter<Scalar>::LevenbergMarquardtFitter(){

    }

    template<class Scalar>
    LevenbergMarquardtFitter<Scalar>::LevenbergMarquardtFitter(Function f, int outputDim,
                     const std::vector<Jacobian> &js,
                     Scalar tau, int maxIterations,
                     Scalar minError, Scalar lambdaMultiplier,
                     Scalar eps1, Scalar eps2,
                     const std::string &linSolver){
      init(f,outputDim,js,tau,maxIterations,minError,lambdaMultiplier,eps1,eps2,linSolver);
    }

    template<class Scalar>
    LevenbergMarquardtFitter<Scalar>::LevenbergMarquardtFitter(FunctionMat f, int outputDim,
                     const std::vector<JacobianMat> &js,
                     Scalar tau, int maxIterations,
                     Scalar minError, Scalar lambdaMultiplier,
                     Scalar eps1, Scalar eps2,
                     const std::string &linSolver){
      init(f,outputDim,js,tau,maxIterations,minError,lambdaMultiplier,eps1,eps2,linSolver);
    }

    template<class Scalar>
    void LevenbergMarquardtFitter<Scalar>::init(Function f,
                           int outputDim,
                           const std::vector<Jacobian> &js,
                           Scalar tau, int maxIterations,
                           Scalar minError, Scalar lambdaMultiplier,
                           Scalar eps1, Scalar eps2,
                           const std::string &linSolver){
      this->useMultiThreading = false;
      this->f = f;
      if(js.size()){
        this->js = js;
      }else{
        this->js = create_numerical_jacobians(outputDim,f);
      }
      this->tau = tau;
      this->maxIterations = maxIterations;
      this->minError = minError;
      this->lambdaMultiplier = lambdaMultiplier;
      this->eps1 = eps1;
      this->eps2 = eps2;
      this->linSolver = linSolver;
      this->useMat = false;
    }

    template<class Scalar>
    void LevenbergMarquardtFitter<Scalar>::init(FunctionMat f,
                           int outputDim,
                           const std::vector<JacobianMat> &js,
                           Scalar tau, int maxIterations,
                           Scalar minError, Scalar lambdaMultiplier,
                           Scalar eps1, Scalar eps2,
                           const std::string &linSolver){
      this->useMultiThreading = false;
      this->fMat = f;
      if(js.size()){
        this->jsMat = js;
      }else{
        this->jsMat = create_numerical_jacobians(outputDim,f);
      }
      this->tau = tau;
      this->maxIterations = maxIterations;
      this->minError = minError;
      this->lambdaMultiplier = lambdaMultiplier;
      this->eps1 = eps1;
      this->eps2 = eps2;
      this->linSolver = linSolver;
      this->useMat = true;
    }

    template<class Scalar>
    inline Scalar sqr_dist(const Scalar &a, const Scalar &b){
      return sqr(a-b);
    }

    template<class Scalar>
    Scalar LevenbergMarquardtFitter<Scalar>::error(const Matrix &ys, const Matrix &y_est) const {
      return ys.sqrDistanceTo(y_est)/2.0;
    }

    template<class Scalar>
    void LevenbergMarquardtFitter<Scalar>::setUseMultiThreading(bool enable){
      useMultiThreading = enable;
    }

    template<class Scalar>
    typename LevenbergMarquardtFitter<Scalar>::Result
    LevenbergMarquardtFitter<Scalar>::fitVec(const Matrix &xs, const Matrix &ys, Params params){
      const int O = ys.cols(); // output dim
      ICLASSERT_THROW(O == (int)js.size(), ICLException("LevenbergMarquardtFitter::fit: ys.cols() and outputDim differ"));
      const int I = xs.cols();
      const int D = xs.rows();
      const int P = params.dim();
      const int MAX_IT = maxIterations;
      const Scalar MIN_E = minError;
      const Scalar EPS1 = eps1;
      const Scalar EPS2 = eps2;

      dst.setDim(P);

      J.setBounds(P,D);
      H.setBounds(P,P);
      H.setBounds(P,P);

      params_new.setDim(P);
      y_est.setBounds(O,D);
      dy.setBounds(D);

      std::vector<Scalar> v(O,2.0);
      std::vector<Scalar> lambdas(O,0.0);

      std::vector<Vector> xs_rows(D);
      std::vector<Vector> ys_rows(D);
      std::vector<Vector> y_est_rows(D);

      for(int i=0;i<D;++i){
        xs_rows[i] = Vector(I, const_cast<Scalar*>(xs.row_begin(i)),false);
        ys_rows[i] = Vector(O, const_cast<Scalar*>(ys.row_begin(i)),false);
        y_est_rows[i] = Vector(O, y_est.row_begin(i), false);
        y_est_rows[i] = f(params,xs_rows[i]);
      }

      Scalar e = error(ys,y_est);
      Scalar eInit = e;
      if(e < minError){
        Result r = {-1, eInit, e, lambdas, params};
        if(dbg) dbg(r);
        return r;
      }

      int it = 0;

#ifdef USE_OPENMP
      bool mt = useMultiThreading;
#endif

      for(;it < MAX_IT; ++it){
        for(int o=0;o<O;++o){
          if (it == 0 || O > 1) {

#ifdef USE_OPENMP
#pragma omp parallel for if(mt)
#endif
            for(int i=0;i<D;++i){
              Vector Ji(P,J.row_begin(i),false);
              js[o](params,xs_rows[i],Ji);
              dy[i] = f(params, xs_rows[i])[o] - ys(o,i);
            }

            matrix_mult_t(J,J,H,SRC1_T);
            matrix_mult_t(J,dy,dst,SRC1_T);

            Scalar maxN = fabs(dst[0]);
            for (unsigned int i = 1; i < dst.rows(); ++i) {
              Scalar tmp = fabs(dst[i]);
              if (tmp > maxN) maxN = tmp;
            }
            if (maxN <= EPS1) {
              Result result = { it, eInit, e, lambdas, params};
              return result;
            }

            // first guess of lambda
            if (it == 0) {
              Scalar H_max = H(0,0);
              for (unsigned int i = 1; i < H.cols(); ++i) {
                if (H(i,i) > H_max) H_max = H(i,i);
              }
              lambdas[o] = tau * H_max;
            }
          }

          for(int i=0;i<P;++i){
            H(i,i) += lambdas[o];
          }

          // Creating Mx = b to solve
          Params pSolved = H.solve(dst,"svd");

          pSolved *= -1.0f;
          params_new = params + pSolved;

#ifdef USE_OPENMP
#pragma omp parallel for if(mt)
#endif
          for(int i=0;i<D;++i){
            y_est_rows[i] = f(params_new,xs_rows[i]);
          }
          Scalar e_new = error(ys, y_est);

          if(e_new < MIN_E){
            Result result = { it, eInit, e_new, lambdas, params_new};
            return result;
          }

          // stop if the change is small
          if (pSolved.norm() <= EPS2*(params.norm() + EPS2)) {
            Result result = { it, eInit, e_new, lambdas, params_new};
            return result;
          }

          // gain ratio
          Scalar delta = 2.0*(e - e_new) / (pSolved.transp() * (pSolved*lambdas[o] - dst))[0];

          if (delta > 0.0) {
            v[o] = 2.0;
            params = params_new;
            e = e_new;
            lambdas[o] *= iclMax(1.0/3.0, 1.0 - pow(2.0*delta - 1.0, 3));

            if (O == 1) {
#ifdef USE_OPENMP
#pragma omp parallel for if(mt)
#endif
              for(int i=0;i<D;++i){
                Vector Ji(P,J.row_begin(i),false);
                js[o](params,xs_rows[i],Ji);
                dy[i] = f(params, xs_rows[i])[o] - ys(o,i);
              }

              matrix_mult_t(J,J,H,SRC1_T);
              matrix_mult_t(J,dy,dst,SRC1_T);

              Scalar maxN = fabs(dst[0]);
              for (unsigned int i = 1; i < dst.rows(); ++i) {
                Scalar tmp = fabs(dst[i]);
                if (tmp > maxN) maxN = tmp;
              }
              if (maxN <= EPS1) {
                Result result = { it, eInit, e, lambdas, params};
                return result;
              }
            }
          } else {
            if (O == 1) {
              // change the hessian back to normal
              for(int i=0;i<P;++i){
                H(i,i) -= lambdas[o];
              }
            }

            lambdas[o] *= v[o];
            v[o] *= 2.0;

            if (v[o] > 1.e15) {
              Result r = { it, eInit, e, lambdas, params };
              if(dbg) dbg(r);
              return r;
            }
          }

          if(dbg){
            Result r = { it, eInit, e, lambdas, params};
            dbg(r);
          }
        }
      }
      Result result = { it, eInit, e, lambdas, params };
      return result;
    }

    template<class Scalar>
    typename LevenbergMarquardtFitter<Scalar>::Result
    LevenbergMarquardtFitter<Scalar>::fitMat(const Matrix &xs, const Matrix &ys, Params params){
      const int O = ys.cols(); // output dim
      ICLASSERT_THROW(O == (int)jsMat.size(), ICLException("LevenbergMarquardtFitter::fit: ys.cols() and outputDim differ"));
      const int D = xs.cols();
      const int P = params.dim();
      const int MAX_IT = maxIterations;
      const Scalar MIN_E = minError;
      const Scalar EPS1 = eps1;
      const Scalar EPS2 = eps2;

      dst.setDim(P);

      J.setBounds(D,P);
      H.setBounds(P,P);
      H.setBounds(P,P);

      params_new.setDim(P);
      y_est.setBounds(O,D);
      dy.setBounds(D);

      std::vector<Scalar> v(O,2.0);
      std::vector<Scalar> lambdas(O,0.0);

      y_est = fMat(params, xs);

      Scalar e = error(ys,y_est);
      Scalar eInit = e;

      if(e < minError){
        Result r = {-1, eInit, e, lambdas, params};
        if(dbg) dbg(r);
        return r;
      }

      int it = 0;

      for(; it < MAX_IT; ++it){
        for(int o=0;o<O;++o){
          if (it == 0 || O > 1) {
            if (o > 0) y_est = fMat(params, xs);
            jsMat[o](params, xs, J);
            dy = y_est.row(o).transp() - ys.transp().row(o).transp();

            matrix_mult_t(J.transp(),J.transp(),H,SRC1_T);
            matrix_mult_t(J.transp(),dy,dst,SRC1_T);

            Scalar maxN = fabs(dst[0]);
            for (unsigned int i = 1; i < dst.rows(); ++i) {
              Scalar tmp = fabs(dst[i]);
              if (tmp > maxN) maxN = tmp;
            }
            if (maxN <= EPS1) {
              Result result = { it, eInit, e, lambdas, params};
              return result;
            }

            // first guess of lambda
            if (it == 0) {
              Scalar H_max = H(0,0);
              for (unsigned int i = 1; i < H.cols(); ++i) {
                if (H(i,i) > H_max) H_max = H(i,i);
              }
              lambdas[o] = tau * H_max;
            }
          }

          for(int i=0;i<P;++i){
            H(i,i) += lambdas[o];
          }

          // Creating Mx = b to solve
          Params pSolved = H.solve(dst,"svd");

          pSolved *= -1.0f;
          params_new = params + pSolved;

          y_est = fMat(params_new, xs);
          Scalar e_new = error(ys, y_est);

          if(e_new < MIN_E){
            Result result = { it, eInit, e_new, lambdas, params_new};
            return result;
          }

          // stop if the change is small
          if (pSolved.norm() <= EPS2*(params.norm() + EPS2)) {
            Result result = { it, eInit, e_new, lambdas, params_new};
            return result;
          }

          // gain ratio
          Scalar delta = 2.0*(e - e_new) / (pSolved.transp() * (pSolved*lambdas[o] - dst))[0];

          if (delta > 0.0) {
            v[o] = 2.0;
            params = params_new;
            e = e_new;
            lambdas[o] *= iclMax(1.0/3.0, 1.0 - pow(2.0*delta - 1.0, 3));

            if (O == 1) {
              jsMat[o](params, xs, J);
              dy = y_est.row(o).transp() - ys.transp().row(o).transp();

              matrix_mult_t(J.transp(),J.transp(),H,SRC1_T);
              matrix_mult_t(J.transp(),dy,dst,SRC1_T);

              Scalar maxN = fabs(dst[0]);
              for (unsigned int i = 1; i < dst.rows(); ++i) {
                Scalar tmp = fabs(dst[i]);
                if (tmp > maxN) maxN = tmp;
              }
              if (maxN <= EPS1) {
                Result result = { it, eInit, e, lambdas, params};
                return result;
              }
            }
          } else {
            if (O == 1) {
              // change the hessian back to normal
              for(int i=0;i<P;++i){
                H(i,i) -= lambdas[o];
              }
            }

            lambdas[o] *= v[o];
            v[o] *= 2.0;

            if (v[o] > 1.e15) {
              Result r = { it, eInit, e, lambdas, params };
              if(dbg) dbg(r);
              return r;
            }
          }

          if(dbg){
            Result r = { it, eInit, e, lambdas, params};
            dbg(r);
          }
        }
      }
      Result result = { it, eInit, e, lambdas, params };
      return result;
    }

    template<class Scalar>
    typename LevenbergMarquardtFitter<Scalar>::Result
    LevenbergMarquardtFitter<Scalar>::fit(const Matrix &xs, const Matrix &ys, Params params){
      if (useMat) return fitMat(xs, ys, params);
      else return fitVec(xs, ys, params);
    }

    namespace{
      template<class Scalar>
      struct NumericJacobian : public FunctionImpl<void,const DynColVector<Scalar>&,
      const DynColVector<Scalar>&,
      DynColVector<Scalar>&>{
        typedef typename LevenbergMarquardtFitter<Scalar>::Function Function;
        typedef typename LevenbergMarquardtFitter<Scalar>::Params Params;
        typedef typename LevenbergMarquardtFitter<Scalar>::Vector Vector;

        int o;
        Function f;
        Scalar delta;

        NumericJacobian(int o, Function f, Scalar delta):
          o(o),f(f),delta(delta){}

        virtual void operator()(const Params &params,
                                const Vector &x,
                                Vector &target) const{
          Vector p = params;
          for(unsigned int i=0;i<params.dim();++i){
            p[i] = params[i] + delta/2;
            Scalar f1 = f(p,x)[o];
            p[i] = params[i] - delta/2;
            Scalar f2 = f(p,x)[o];
            p[i] = params[i];
            target[i] = ( f1 - f2 ) / delta;
          }
        }
      };
    }

    namespace{
      template<class Scalar>
      struct NumericJacobianMat : public FunctionImpl<void,const DynColVector<Scalar>&,
      const DynMatrix<Scalar>&,
      DynMatrix<Scalar>&>{
        typedef typename LevenbergMarquardtFitter<Scalar>::FunctionMat FunctionMat;
        typedef typename LevenbergMarquardtFitter<Scalar>::Params Params;
        typedef typename LevenbergMarquardtFitter<Scalar>::Vector Vector;
        typedef typename LevenbergMarquardtFitter<Scalar>::Matrix Matrix;

        int o;
        FunctionMat f;
        Scalar delta;

        NumericJacobianMat(int o, FunctionMat f, Scalar delta):
          o(o),f(f),delta(delta){}

        virtual void operator()(const Params &params,
                                const Matrix &x,
                                Matrix &target) const{
          Vector p = params;
          for(unsigned int i=0;i<params.dim();++i){
            p[i] = params[i] + delta/2;
            Matrix f1(x.cols(), 1, f(p,x).row_begin(o));
            p[i] = params[i] - delta/2;
            Matrix f2(x.cols(), 1, f(p,x).row_begin(o));
            p[i] = params[i];
            target.row(i) = (( f1 - f2 ) / delta);
          }
        }
      };
    }


    template<class Scalar>
    typename LevenbergMarquardtFitter<Scalar>::Jacobian
    LevenbergMarquardtFitter<Scalar>::create_numerical_jacobian(int o, Function f, float delta){
      return Jacobian(new NumericJacobian<Scalar>(o,f,delta));
    }

    template<class Scalar>
    typename LevenbergMarquardtFitter<Scalar>::JacobianMat
    LevenbergMarquardtFitter<Scalar>::create_numerical_jacobian(int o, FunctionMat f, float delta){
      return JacobianMat(new NumericJacobianMat<Scalar>(o,f,delta));
    }

    template<class Scalar>
    std::vector<typename LevenbergMarquardtFitter<Scalar>::Jacobian>
    LevenbergMarquardtFitter<Scalar>::create_numerical_jacobians(int n, Function f, float delta){
      std::vector<typename LevenbergMarquardtFitter<Scalar>::Jacobian> js(n);
      for(int i=0;i<n;++i){
        js[i] = create_numerical_jacobian(i,f);
      }
      return js;
    }

    template<class Scalar>
    std::vector<typename LevenbergMarquardtFitter<Scalar>::JacobianMat>
    LevenbergMarquardtFitter<Scalar>::create_numerical_jacobians(int n, FunctionMat f, float delta){
      std::vector<typename LevenbergMarquardtFitter<Scalar>::JacobianMat> js(n);
      for(int i=0;i<n;++i){
        js[i] = create_numerical_jacobian(i,f);
      }
      return js;
    }


    template<class Scalar>
    void LevenbergMarquardtFitter<Scalar>::setDebugCallback(DebugCallback dbg){
      this->dbg = dbg;
    }

    template<class Scalar>
    typename LevenbergMarquardtFitter<Scalar>::Data
    LevenbergMarquardtFitter<Scalar>::create_data(const Params &p, Function f, int xDim, int yDim, int num, Scalar minX, Scalar maxX){
      URand r(minX,maxX);
      Data data = { Matrix(xDim,num), Matrix(yDim, num) };
      for(int i=0;i<num;++i){
        std::fill(data.x.row_begin(i), data.x.row_end(i), r);
        Vector yi(yDim, data.y.row_begin(i), false);
        yi = f(p,Vector(xDim,data.x.row_begin(i),false));
      }
      return data;
    }

    template<class Scalar>
    void LevenbergMarquardtFitter<Scalar>::default_debug_callback(const Result &r){
      std::cout << r << std::endl;
    }


    template class ICLMath_API LevenbergMarquardtFitter<icl32f>;
    template class ICLMath_API LevenbergMarquardtFitter<icl64f>;
  } // namespace math
}

