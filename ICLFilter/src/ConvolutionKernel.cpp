/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : ICLFilter/src/ConvolutionKernel.cpp                    **
** Module : ICLFilter                                              **
** Authors: Christof Elbrechter                                    **
**                                                                 **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
*********************************************************************/

#include <ICLFilter/ConvolutionKernel.h>
#include <ICLUtils/Macros.h>
#include <cmath>
#include <algorithm>

namespace icl{
  namespace{
    template<class T> inline T* deepcopy(const T*src, int dim){
      T *cpy = new T[dim];
      std::copy(src,src+dim,cpy);
      return cpy;
    }
    template<class T> inline T* save_deepcopy(const T*src, int dim){
      if(!src || !dim) return 0;
      T *cpy = new T[dim];
      std::copy(src,src+dim,cpy);
      return cpy;
    }
    bool is_convertable_to_int (float *data, int len){
      // tests if an element of the given float* has decimals
      // if it does: return 0, else 1
      for(int i=0;i<len;i++){
        if (data[i] != (float) rint (data[i])) return false;
      }
      return true;
    }

    int GAUSS_3x3[10] = { 16,
                          1, 2, 1,
                          2, 4, 2,
                          1, 2, 1 };
    
    int GAUSS_5x5[26] = { 571,
                          2,  7,  12,  7,  2,
                          7, 31,  52, 31,  7,
                          12, 52, 127, 52, 12,
                          7, 31,  52, 31,  7,
                          2,  7,  12,  7,  2 };
    
    int SOBEL_X_3x3[10] = { 1,
                            1,  0, -1,
                            2,  0, -2,
                            1,  0, -1 };
    
    int SOBEL_X_5x5[26] = { 1,
                            1,  2,  0,  -2,  -1, 
                            4,  8,  0,  -8,  -4,
                            6, 12,  0, -12,  -6,
                            4,  8,  0,  -8,  -4,
                            1,  2,  0,  -2,  -1 };
    
    int SOBEL_Y_3x3[10] = {  1, 
                             1,  2,  1,
                             0,  0,  0,
                             -1, -2, -1  };
    
    int SOBEL_Y_5x5[26] = {  1,
                             1,  4,   6,  4,  1,
                             2,  8,  12,  8,  2,
                             0,  0,   0,  0,  0,
                             -2, -8, -12, -8, -4,
                             -1, -4,  -6, -4, -1 };
    
    int LAPLACE_3x3[10] = { 1,
                            1, 1, 1,
                            1,-8, 1,
                            1, 1, 1} ;
    
    int LAPLACE_5x5[26] = { 1,
                            -1, -3, -4, -3, -1,
                            -3,  0,  6,  0, -3,
                            -4,  6, 20,  6, -4,
                            -3,  0,  6,  0, -3,
                            -1, -3, -4, -3, -1 };
    
  }
  ConvolutionKernel::ConvolutionKernel():fdata(0),idata(0),factor(0),isnull(true),owned(false),ft(custom){}
  
  ConvolutionKernel::ConvolutionKernel(const ConvolutionKernel &other):
    size(other.size),fdata(0),idata(0),factor(other.factor),isnull(other.isnull),owned(other.owned),ft(other.ft){
    if(owned){
      if(other.fdata)fdata = deepcopy(other.fdata,getDim());
      if(other.idata)idata = deepcopy(other.idata,getDim());
    }else{
      fdata = other.fdata;
      idata = other.idata;
    }
  }

  ConvolutionKernel::ConvolutionKernel(int *data, const Size &size,int factor, bool deepCopy) throw (InvalidSizeException):
    size(size),fdata(0),idata(0),factor(factor),isnull(false),owned(deepCopy),ft(custom){
    ICLASSERT_THROW(getDim() > 0,InvalidSizeException(__FUNCTION__));
    if(deepCopy){
      idata = deepcopy(data,getDim());
    }else{
      idata = data;
    }    
    isnull = !idata;
  }
  ConvolutionKernel::ConvolutionKernel(float *data, const Size &size, bool deepCopy) throw (InvalidSizeException):
    size(size),fdata(0),idata(0),factor(factor),isnull(false),owned(deepCopy),ft(custom){
    ICLASSERT_THROW(getDim() > 0,InvalidSizeException(__FUNCTION__));
    if(deepCopy){
      fdata = deepcopy(data,getDim());
    }else{
      fdata = data;
    }    
    isnull = !fdata;
  }
  
  
  ConvolutionKernel::ConvolutionKernel(fixedType t, bool useFloats):
    size(t>9 ? Size(5,5) : Size(3,3)),fdata(0),isnull(false),owned(false),ft(t){
    switch(t){
      case gauss3x3: idata = GAUSS_3x3; break;
      case gauss5x5: idata = GAUSS_5x5; break;
      case sobelX3x3: idata = SOBEL_X_3x3; break;
      case sobelX5x5: idata = SOBEL_X_5x5; break;
      case sobelY3x3: idata = SOBEL_Y_3x3; break;
      case sobelY5x5: idata = SOBEL_Y_5x5; break;
      case laplace3x3: idata = LAPLACE_3x3; break;
      case laplace5x5: idata = LAPLACE_5x5; break;
      default:
        throw ICLException("invalid fixed convolution kernel type");
    }
    factor = *idata++;
    if(useFloats){
      toFloat();
    }
  }
  
  ConvolutionKernel &ConvolutionKernel::operator=(const ConvolutionKernel &other){
    if(owned){
      ICL_DELETE(idata);
      ICL_DELETE(fdata);
    }
    size = other.size;
    fdata = 0;
    idata = 0;
    factor = other.factor;
    isnull = other.isnull;
    owned = other.owned;
    ft = other.ft;

    if(owned){
      if(other.fdata)fdata = deepcopy(other.fdata,getDim());
      if(other.idata)idata = deepcopy(other.idata,getDim());
    }else{
      fdata = other.fdata;
      idata = other.idata;
    }
    
    return *this;
  }
    
  ConvolutionKernel::~ConvolutionKernel() {
    if(owned){
      ICL_DELETE(idata);
      ICL_DELETE(fdata);
    }
  }

  void ConvolutionKernel::toFloat(){
    if(idata && !fdata){
      fdata = new float[getDim()];
      std::transform(idata,idata+getDim(),fdata,std::bind2nd(std::divides<float>(),factor));
      if(owned){
        ICL_DELETE(idata);
      }else{
        idata = 0;
      }
      owned = true;
      factor = 1.0;
    }
  }

  void ConvolutionKernel::toInt(bool force){
    if(fdata && !idata){
      if(force || is_convertable_to_int(fdata,getDim())){
        idata = new int[getDim()];
        std::copy(fdata,fdata+getDim(),idata);
        if(owned){
          ICL_DELETE(fdata);
        }else{
          fdata = 0;
        }
        owned = true;
      }
    }
  }

  
  void ConvolutionKernel::detach(){
    if(!owned){
      if(idata) idata = deepcopy(idata,getDim());
      if(fdata) fdata = deepcopy(fdata,getDim());
    }
  }

  
}
   

