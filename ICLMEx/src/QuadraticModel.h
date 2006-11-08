#ifndef QUADRATIC_MODEL_H
#define QUADRATIC_MODEL_H

#include <GeneralModel.h>

namespace icl{
 
 template<class T>
 class QuadraticModel : public GeneralModel<T>{
    public:
    virtual ~QuadraticModel(){}
    virtual Array<T> x(T y, T *params) const;
    virtual Array<T> y(T x, T *params) const;

    virtual T px(T y, T* params) const = 0; 
    virtual T qx(T y, T* params) const = 0; 
    virtual T py(T x, T* params) const = 0;     
    virtual T qy(T x, T* params) const = 0; 


    protected:
    QuadraticModel(int dim);
 };

}
#endif

/*
class EllipseModel : public Model{
// {{{ open

  public:
    EllipseModel() : Model(6){}
    virtual ~EllipseModel(){}
    virtual void features(real x,real y, real *dst)const{
      // {{{ open

      dst[0] = x*x;
      dst[1] = x*y;
      dst[2] = y*y;
      dst[3] = x;
      dst[4] = y;
      dst[5] = 1;
    }

    // }}}
    virtual real* x(int &n,real y, real *params){
      // {{{ open

      real &a=params[0], &b=params[1], &c=params[2], &d=params[3],  &e=params[4], &f=params[5];
      real p = (b*y+d)/a;
      real q = (c*y*y+e*y+f)/a;
      real rootTerm = p*p/4-q;
      if(rootTerm >= 0){ // real solution
        m_arXY[0]=(-p*0.5 + sqrt(rootTerm));
        m_arXY[1]=(-p*0.5 - sqrt(rootTerm));
        n = 2;
      }else{
        n=0;
      }     
      return m_arXY;
    }

    // }}}
    virtual real* y(int &n,real x, real *params){
      // {{{ open

      real &a=params[0], &b=params[1], &c=params[2], &d=params[3],  &e=params[4], &f=params[5];
      real p = (b*x+e)/c;
      real q = (a*x*x+d*x+f)/c;
      real rootTerm = p*p/4-q;
      if(rootTerm >= 0){ // real solution
        m_arXY[0]=(-p*0.5 + sqrt(rootTerm));
        m_arXY[1]=(-p*0.5 - sqrt(rootTerm));
        n = 2;
      }else{
        n=0;
      }
      return m_arXY;
    }

    // }}}
  private:
    real m_arXY[2];
      
  };

  // }}}
  
class CircleModel : public Model{
// {{{ open

  public:
    CircleModel() : Model(5){}
    virtual ~CircleModel(){}
    virtual void features(real x,real y, real *dst)const{
      dst[0] = x*x;
      dst[1] = y*y;
      dst[2] = x;
      dst[3] = y;
      dst[4] = 1;
    }
    virtual real* x(int &n,real y, real *params){
      // {{{ open
      
      real &a=params[0], &b=params[1], &c=params[2], &d=params[3],  &e=params[4];
      real p = c/a;
      real q = (b*y*y+d*y+e)/a;
      real rootTerm = p*p/4-q;
      if(rootTerm >= 0){ // real solution
        m_arXY[0]=(-p*0.5 + sqrt(rootTerm));
        m_arXY[1]=(-p*0.5 - sqrt(rootTerm));
        n = 2;
      }else{
        n=0;
      }     
      return m_arXY;
    }

    // }}}
    virtual real* y(int &n,real x, real *params){
      // {{{ open
      // using equation of the ellipse with b=a
      real &a=params[0], &b=params[1], &c=params[2], &d=params[3],  &e=params[4];
      real p = d/b;
      real q = (a*x*x+c*x+e)/b;
      real rootTerm = p*p/4-q;
      if(rootTerm >= 0){ // real solution
        m_arXY[0]=(-p*0.5 + sqrt(rootTerm));
        m_arXY[1]=(-p*0.5 - sqrt(rootTerm));
        n = 2;
      }else{
        n=0;
      }
      return m_arXY;
    }

    // }}}
  private:
    real m_arXY[2];
  };

  // }}}

class CircleModel2 : public Model{
// {{{ open

  public:
    CircleModel2() : Model(4){}
    virtual ~CircleModel2(){}
    virtual void features(real x,real y, real *dst)const{
      dst[0] = x*x+y*y;
      dst[1] = x;
      dst[2] = y;
      dst[3] = 1;
    }
    virtual real* x(int &n,real y, real *params){
      // {{{ open
      
      real &a=params[0], &b=params[1], &c=params[2], &d=params[3];
      real p = b/a;
      real q = (a*y*y+c*y+d)/a;
      real rootTerm = p*p/4-q;
      if(rootTerm >= 0){ // real solution
        m_arXY[0]=(-p*0.5 + sqrt(rootTerm));
        m_arXY[1]=(-p*0.5 - sqrt(rootTerm));
        n = 2;
      }else{
        n=0;
      }     
      return m_arXY;
    }

    // }}}
    virtual real* y(int &n,real x, real *params){
      // {{{ open
      // using equation of the ellipse with b=a
      real &a=params[0], &b=params[1], &c=params[2], &d=params[3];
      real p = c/a;
      real q = (a*x*x+b*x+d)/a;
      real rootTerm = p*p/4-q;
      if(rootTerm >= 0){ // real solution
        m_arXY[0]=(-p*0.5 + sqrt(rootTerm));
        m_arXY[1]=(-p*0.5 - sqrt(rootTerm));
        n = 2;
      }else{
        n=0;
      }
      return m_arXY;
    }

    // }}}
  private:
    real m_arXY[2];
  };

  // }}}
*/
