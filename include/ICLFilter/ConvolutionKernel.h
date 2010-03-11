/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLFilter module of ICL                       **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifndef ICL_CONVOLUTION_KERNEL_H
#define ICL_CONVOLUTION_KERNEL_H

#include <ICLUtils/Size.h>
#include <ICLUtils/Exception.h>


namespace icl{
  /// Utility struct for managing convolution kernel data
  /** This structure provides a shallow as well as deep wrapper for int- and 
      float-pointers into ConvolutionKernel-instances. Convolution Kernels
      can be float- or integer valued.


copies of shallow copied instances are shallow! */
  struct ConvolutionKernel{

    /// this enum contains several predefined convolution kernel types
    /** <h3>kernelSobleX</h3>
        The sobel x filter is a combined filter. It performs a symmetrical
        border filter operation in x-direction, followed by a smoothing
        operation in y-direction:
        <pre>

                        ---------     ---        
                       | 1  0 -1 |   | 1 |    ---------
        kernelSobelX = | 2  0 -2 | = | 2 | * | 1  0 -1 |
                       | 1  0 -1 |   | 1 |    ---------
                        ---------     ---
        </pre>           
    
        <h3>kernelSobelY</h3>
        The sobel y filter is essentially equal to the sobel y filter. The
        border detection will run in y-direction, and the smoothing x-direction.

        <pre>

                        ---------     ---        
                       | 1  2  1 |   | 1 |    ---------
        kernelSobelY = | 0  0  0 | = | 0 | * | 1  2  1 |
                       |-1 -2 -1 |   |-1 |    ---------
                        ---------     ---
        </pre>               
    
        <h3>kernelGauss3x3</h3>
        This is a 3x3-Pixel approximation of a 2D Gaussian. It is separable into
        smoothing filters in x- and y-direction.
        
        <pre>

                                 ---------     ---        
                                | 1  2  1 |   | 1 |    ---------
        kernelGauss3x3 = 1/16 * | 2  4  2 | = | 2 | * | 1  2  1 | * (1/4) *(1/4) 
                                | 1  2  1 |   | 1 |    ---------
                                 ---------     ---
        </pre>               

        <h3>kernelGauss5x5</h3>
        This is a 5x5-Pixel approximation of a 2D Gaussian. It is separable into
        smoothing filters in x- and y-direction.
        
        <pre>

                                  ----------------- 
                                 | 2   7  12  7  2 |
                                 | 7  31  52 31  7 |
        kernelGauss5x5 = 1/571 * |12  52 127 52 12 |
                                 | 7  31  52 31  7 |
                                 | 2   7  12  7  2 |
                                  ----------------- 

        </pre>               

        <h3>kernelLaplace</h3>
        The Laplacian kernel is a discrete approximation of the 2nd derivation
        of a 2D function.

        <pre>

                         --------- 
                        | 1  1  1 |
        kernelLaplace = | 1 -8  1 |
                        | 1  1  1 |
                         --------- 
        </pre>    
    */
    enum fixedType { 
      gauss3x3=0,   //!< 3x3 approximation of a Gaussian
      gauss5x5=10,  //!< 5x5 approximation of a Gaussian
      sobelX3x3=1,  //!< 3x3 sobel x filter
      sobelX5x5=11, //!< 5x5 sobel x filter
      sobelY3x3=2,  //!< 3x3 sobel y filter
      sobelY5x5=12, //!< 5x5 sobel y filter
      laplace3x3=3, //!< 3x3 approximation of the 2nd derivation
      laplace5x5=13,//!< 5x5 approximation of the 2nd derivation
      custom,
    };
    
    /// creates an empty null kernel
    ConvolutionKernel();
    
    /// crates a copied kernel
    /** shallow wrapped kernels are copied shallowly !*/
    ConvolutionKernel(const ConvolutionKernel &other);  
    
    /// create an integer based kernel
    /** factor can be used to rescale convolution results by given factor:
        <code>
        Result(x,y) = ( src *(x,y)* kernel ) / factor
        // where *(x,y)* is a convolution at location (x,y)
        </code>
    */
    ConvolutionKernel(int *data, const Size &size,int factor=1, bool deepCopy=true) throw (InvalidSizeException);
    
    /// create a float valued kernel
    ConvolutionKernel(float *data, const Size &size, bool deepCopy=true) throw (InvalidSizeException);
    
    /// create a fixed kernel (optionally as float kernel)
    ConvolutionKernel(fixedType t, bool useFloats=false);
    
    /// Destructor
    ~ConvolutionKernel();
    
    /// Assignment operator
    /** Shallow-wrapped kernels are copied shallowly*/
    ConvolutionKernel &operator=(const ConvolutionKernel &other);

    /// converts internal idata to float data regarding 'factor'
    void toFloat();
    
    /// converts fdata to idata if possible and/or if force flag is true
    /** 'possible' means that all float values do not show decimals */
    void toInt(bool force=false);
    
    /// returns kernels width * height
    inline int getDim() const { return size.getDim(); }
    
    /// returns the kernels size
    inline const Size &getSize() const { return size; }
    
    /// returns the kernels width
    inline int getWidth() const { return size.width; }
    
    /// returns the kernels height
    inline int getHeight() const { return size.height; }
    
    /// returns whether this kernel has float data
    inline bool isFloat() const { return !!fdata; }
    
    /// returns the kernels float data pointer (const) (which may be 0)
    inline const float *getFloatData() const { return fdata; }

    /// returns the kernels float data pointer (which may be 0)
    inline float *getFloatData() { return fdata; }

    /// returns the kernels int data pointer (const) (which may be 0)
    inline const int *getIntData() const { return idata; }

    /// returns the kernels int data pointer (which may be 0)
    inline int *getIntData() { return idata; }
    
    /// returns the kernels scaling factor (only used if its an integer kernel)
    inline int getFactor() const { return factor; }
    
    /// return whether idata or fdata is not null
    inline bool isNull() const { return isnull; }
    
    /// returns the kernels fixed type (necessary for IPP-optimization)
    inline fixedType getFixedType() const { return ft; }
    
    /// ensures shallow copied data is copied deeply
    void detach();
    
  private:
    Size size;    //!< associated size
    float *fdata; //!< float data pointer
    int *idata;   //!< int data pointer
    int factor;   //!< scaling factor for integer kernels
    bool isnull;  //!< is already initialized
    bool owned;   //!< is data owned
    fixedType ft; //!< fixed type set
  };
}

#endif
