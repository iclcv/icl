#ifndef ICL_VEC4_H
#define ICL_VEC4_H

#include <iclTypes.h>
#include <iclCore.h>
#include <math.h>
#include <string>

namespace icl{
  /// 4D Vector class implementation
  template<class T=icl32f>
  class Vec4D{
    public:
    /// internal data representation
    T data[4];
    
    /// creates a new Vec4D object
    inline Vec4D(T x=0,T y=0, T z=0, T h=0){
      data[0] = x;
      data[1] = y;
      data[2] = z;
      data[3] = h;
    }
    
    /// copy constructor 
    template<class OtherT>
    inline Vec4D(const Vec4D<OtherT> &v){
      data[0] = Cast<OtherT,T>::cast(v.x());
      data[1] = Cast<OtherT,T>::cast(v.y());
      data[2] = Cast<OtherT,T>::cast(v.z());
      data[3] = Cast<OtherT,T>::cast(v.h());
    }
    
    /// returns (*this)[0]
    inline T& x() { return data[0]; } 

    /// returns (*this)[1]
    inline T& y() { return data[1]; } 

    /// returns (*this)[2]
    inline T& z() { return data[2]; }     

    /// returns (*this)[3]
    inline T& h() { return data[3]; } 

    /// returns (*this)[0] (const)
    inline const T& x() const { return data[0]; } 

    /// returns (*this)[1] (const)
    inline const T& y() const { return data[1]; } 

    /// returns (*this)[2] (const)
    inline const T& z() const { return data[2]; }     

    /// returns (*this)[3] (const)
    inline const T& h() const { return data[3]; } 

    
    /// Assign operator (other vec)
    template<class OtherT>
    inline Vec4D &operator=(const Vec4D<OtherT> &v) { 
      x()=Cast<OtherT,T>::cast(v.x());
      y()=Cast<OtherT,T>::cast(v.y());
      z()=Cast<OtherT,T>::cast(v.z());
      h()=Cast<OtherT,T>::cast(v.h());
      return *this;
    }
    
    /// Assign all elements to a given value
    inline Vec4D &operator=(T f) { x()=y()=z()=h()=f; return *this;}
    
    /// array access operator (0=x,1=y,2=z,3=h)
    inline T &operator[](int i){ 
      return data[i]; 
    }
    
    /// array access operator (0=x,1=y,2=z,3=h) (const version)
    inline const T &operator[](int i) const{ 
      return data[i]; 
    }
    
    /// add two vectors
    inline Vec4D operator+(const Vec4D &v) const{ return Vec4D(x()+v.x(),y()+v.y(),z()+v.z(),h()+v.h()); }
    
    /// subtract two vectors
    inline Vec4D operator-(const Vec4D &v) const{ return Vec4D(x()-v.x(),y()-v.y(),z()-v.z(),h()-v.h()); }
    
    /// scalar product of two vectors
    inline T operator*(const Vec4D &v) const{ return x()*v.x() + y()*v.y() + z()*v.z() +h()*v.h(); }
    
    /// add a constant to all elements 
    inline Vec4D operator+(T f) const{ return Vec4D(x()+f,y()+f,z()+f,h()+f); }
    
    /// subtract a constant from all elements 
    inline Vec4D operator-(T f) const{ return Vec4D(x()-f,y()-f,z()-f,h()-f); }
    
    /// mutliplicate a constant to all elements 
    inline Vec4D operator*(T f) const{ return Vec4D(x()*f,y()*f,z()*f,h()*f); }
    
    /// devide all elements by a constant 
    inline Vec4D operator/(T f) const{ return Vec4D(x()/f,y()/f,z()/f,h()/f); }
    
    /// inplace add another vector
    inline Vec4D &operator+=(const Vec4D &v) { x()+=v.x(); y()+=v.y(); z()+=v.z(); h()+=v.h(); return *this;}
    
    /// inplace subtract another vector
    inline Vec4D &operator-=(const Vec4D &v) { x()-=v.x(); y()-=v.y(); z()-=v.z(); h()-=v.h(); return *this;} 
    
    /// inplace add a scalar to each parameter 
    inline Vec4D &operator+=(T f) { x()+=f; y()+=f; z()+=f; h()+=f; return *this;}
    
    /// inplace subtract a scalar to each parameter 
    inline Vec4D &operator-=(T f) { x()-=f; y()-=f; z()-=f; h()-=f; return *this;}
    
    /// inplace mulitplicate each parameter by() a scalar 
    inline Vec4D &operator*=(T f) { x()*=f; y()*=f; z()*=f; h()*=f; return *this;}
    
    /// inplace divide each parameter by() a scalar 
    inline Vec4D &operator/=(T f) { x()/=f; y()/=f; z()/=f; h()/=f; return *this;}
    
    /// returns the length of the vector (default:: euclidian length)
    inline icl64f length(T norm=2) const{ return ::pow( ::pow(x(),norm)+::pow(y(),norm)+::pow(z(),norm) +::pow(h(),norm), 1.0/norm); }

    /// returns (-x,-y,-z,-h)
    inline Vec4D operator-() const { return Vec4D(-x(),-y(),-z(),-h()); }

    /// normalize the vector to length 1 (inplace) [ problems with int-values ] 
    inline Vec4D &normalize() { 
      icl64f l=length(); 
      x() = (T)(x()/l);
      y() = (T)(y()/l);
      z() = (T)(z()/l);
      h() = (T)(h()/l);
      return *this;
    }
    
    /// normalized version of the vector to length 1 
    inline Vec4D normalized() const { return Vec4D(*this).normalize(); }
    
    /// homogenize the vector (inplace) --> h==1
    inline Vec4D &homogenize() { 
      if(h()){
        x()/=h();y()/=h();z()/=h();h()=1; 
      }
      return *this; 
    }
    
    /// homogenized version of the vector --> h==1
    inline Vec4D homogenized() const { ICLASSERT_RETURN_VAL(h(),*this); return (*this)/h();}
     
    /// projects the vector into the plane perpendicular to (0,0,z) */
    inline Vec4D &project(T z){
      T zz = z*this->z();
      x()/=zz;
      y()/=zz;
      this->z()=0;
      h()=1; // ??
      return *this;
    }

    /// homogeneous 3D cross-product
    inline Vec4D cross(const Vec4D &v2) const{
      const Vec4D &v1 = *this;
      return Vec4D(v1[1]*v2[2]-v1[2]*v2[1],
                   v1[2]*v2[0]-v1[0]*v2[2],
                   v1[0]*v2[1]-v1[1]*v2[0],
                   1 );
    }
    /// create a projected version 
    /** projects the vector into the plane perpendicular to (0,0,z) */
    inline Vec4D projected(T z)const{
      return Vec4D(*this).project(z);
    }

    /// show the vector to std::out
    void show(const std::string &title="vec") const;
  };
  
  /// Vector definition for float precision elements
  typedef Vec4D<icl32f> Vec4D32f;
  
  /// Vector definition for double precision elements
  typedef Vec4D<icl64f> Vec4D64f;
  
  /// Vector definitin for signed integer elements (T2=float e.g. for length() or for operator*) 
  typedef Vec4D<icl32s> Vec4D32s;
}

#endif
