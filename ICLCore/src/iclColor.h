#ifndef ICL_COLOR_H
#define ICL_COLOR_H

#include <iclCore.h>
#include <iostream>
#include <string>
#include <algorithm>
//#include <bits/stl_function.h>

namespace icl{

  /** \cond */
  template<class T,int N> class GeneralColor;
  /** \endcond */

  /// By default, colors are byte valued and have 3 channels
  typedef GeneralColor<icl8u,3> Color;
  
#define ICL_INSTANTIATE_DEPTH(D)               \
  typedef GeneralColor<icl##D,4> Color##D##_4; \
  typedef GeneralColor<icl##D,3> Color##D##_3; \
  typedef GeneralColor<icl##D,2> Color##D##_2; \
  typedef GeneralColor<icl##D,1> Color##D##_1; \
  typedef GeneralColor<icl##D,1> Color##D##_1; \
  typedef GeneralColor<icl##D,3> Color##D;   
  ICL_INSTANTIATE_ALL_DEPTHS;
#undef ICL_INSTANTIATE_DEPTH

  // Create a color by given name (see GeneralColor Constructor)
  const Color &iclCreateColor(std::string name);
  
  
  /// Color class template handle colors of different channel counts and different value types
  /** In most cases you will not need the GeneralColor class itselft, but you can work with one of
      the several Color typedefs.
      
      The most common typedef is Color with is GeneralColor<icl8u,3> further typedefs for
      3 channel Color types are Color##D = GeneralColor<icl##D,3>. Furthermore, Colors with 1,2,3 or 4 channels
      can be accessed by typedefs Color##D_C = GeneralColor<icl##D,C>.
      
      Color objects can be added, multiplied, copyied and so on.
  */
  template<class T=icl8u, int N=3>
  class GeneralColor{
    T c[N];
    public:
    
    GeneralColor(T c0=0, T c1=0, T c2=0, T c3=0, T c4=0, T c5=0){
      if(N>0) c[0]=c0;
      if(N>1) c[1]=c1;
      if(N>2) c[2]=c2;
      if(N>3) c[3]=c3;
      if(N>4) c[4]=c4;
      if(N>5) c[5]=c5;
    }

    
    /// create color by name: red,green,blue,...
    /** Currently allowed:
        - red
        - green
        - blue
        - cyan
        - magenta
        - yellow
        - black
        - white
        - gray200
        - gray150
        - gray100
        - gray50
        
        Match works case-insensitive! 
        
        Although using a hashmap for colors internally, this function is
        sort of slow in comparison to the creation of a Color objects using
        the default constructor Color(T,T,T,...)
    */
    GeneralColor(const std::string &name){
      *this = iclCreateColor(name);
    }
    
    template<class otherT, int otherN>
    GeneralColor(const GeneralColor<otherT,otherN> &other){
      *this = other;
    }
    
    template<class otherT>
    GeneralColor(T *data){
      for(int i=0;i<N;c[i]=data[i],++i);
    }
    
    template<class otherT>
    GeneralColor<otherT,N> cvt() const {
      return GeneralColor<otherT,N>(*this);
    }

    template<int otherN>
    GeneralColor<T,otherN> cvt() const {
      return GeneralColor<T,otherN>(*this);
    }
    
    template<class otherT, int otherN>
    GeneralColor<otherT,otherN> cvt() const {
      return GeneralColor<otherT,otherN>(*this);
    }
    
    T & operator[](unsigned int idx){
      return c[idx];
    }

    const T & operator[](unsigned int idx) const{
      return c[idx];
    }
    
    T *data() { return c; }

    const T* data() const { return c; }
    
    template<class otherT,int otherN>
    GeneralColor<T,N> &operator=(const GeneralColor<otherT,otherN> &other){
      for(int i=0;(i<N)&&(i<otherN);++i){
        c[i]=Cast<otherT,T>(other.c[i]);
      }
      for(int i=otherN;i<N;++i){
        c[i] = 0;
      }
    }
    
    template<class otherT, int otherN>
    GeneralColor<T,N> operator+(const GeneralColor<otherT,otherN> &other) const{
      GeneralColor<T,N> c(*this);
      return c+=other;
    }
    template<class otherT, int otherN>
    GeneralColor<T,N> operator-(const GeneralColor<otherT,otherN> &other) const{
      GeneralColor<T,N> c(*this);
      return c+=other;
    }
    GeneralColor<T,N> operator*(double d) const {
      GeneralColor<T,N> c(*this);
      return c*=d;;
    }
    GeneralColor<T,N> operator/(double d) const{
      GeneralColor<T,N> c(*this);
      return c/=d;
    }
    
    template<class otherT, int otherN>
    GeneralColor<T,N> &operator+=(const GeneralColor<otherT,otherN> &other){
      std::transform(c,c+iclMin(otherN,N),other.c,c,std::plus<T>());
      return *this;
    }

    template<class otherT, int otherN>
    GeneralColor<T,N> &operator-=(const GeneralColor<otherT,otherN> &other){
      std::transform(c,c+iclMin(otherN,N),other.c,c,std::minus<T>());
      return *this;
    }

    template<class otherT, int otherN>
    GeneralColor<T,N> &operator*=(double d){
      std::transform(c,c+iclMin(otherN,N),c,std::bind2nd(std::multiplies<T>(),d));
      return *this;
    }

    template<class otherT, int otherN>
    GeneralColor<T,N> &operator/=(double d){
      return (*this)*(1.0/d);
    }
    
    GeneralColor<T,N> darker(double factor=0.8){
      return (*this)*factor;
    }

    GeneralColor<T,N> lighter(double factor=0.8){
      return (*this)/factor;
    }
    

  };

  /** \cond */
  template<class T>
  inline std::ostream &iclToStream(std::ostream &str,const T &t){
    return str << t;
  }

  template<> 
  inline std::ostream &iclToStream(std::ostream &str,const icl8u &t){
    return str << (int)t;
  }
  /** \endcond */
  
  template<class T,int N>
  inline std::ostream &operator<<(std::ostream &str, const GeneralColor<T,N> &c){
    str << '(';
    for(int i=0;i<N-1; iclToStream(str,c[i++]) << ',');
    iclToStream(str,c[N-1]) << ')';
    return str;
  }
  

  
}
#endif
