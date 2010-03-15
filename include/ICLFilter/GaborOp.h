/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
**                         University of Bielefeld                 **
**                Contact: nivision@techfak.uni-bielefeld.de       **
**                Website: www.iclcv.org                           **
**                                                                 **
** File   : include/ICLFilter/GaborOp.h                            **
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

#ifndef ICL_GABOR_OP
#define ICL_GABOR_OP

#include <ICLFilter/UnaryOp.h>
#include <ICLUtils/SimpleMatrix.h>
#include <ICLCore/Img.h>
#include <ICLUtils/Point.h>
#include <vector>
#include <ICLUtils/Uncopyable.h>

namespace icl{
  /** \cond */
  class ConvolutionOp;
  /** \endcond */

  
  /// Applies Gabor filter operation on images \ingroup UNARY
  /** \section GEN General Information about Gabor Filters

      A short introduction to Gabor filters can be found at Wikipedia: 
      A Gabor filter is a linear filter whose impulse response is defined by a 
      harmonic function multiplied by a Gaussian function. Because of the 
      multiplication-convolution property, the Fourier transform of a Gabor 
      filter's impulse response is the convolution of the Fourier transform of 
      the harmonic function and the Fourier transform of the Gaussian function.
      \f[
      g(x,y;\lambda,\theta,\psi,\sigma,\gamma)=\exp(-\frac{x'^2+\gamma^2y'^2}{2\sigma^2})\cos(2\pi\frac{x'}{\lambda}+\psi)
      \f]
      where \f[ x' = x \cos\theta + y \sin\theta\,  \f]
      and \f[ y' = -x \sin\theta + y \cos\theta\, \f]
      
      In this equation, \f$\lambda\f$ represents the wavelength of the cosine factor, \f$\theta\f$ represents the orientation
      of the normal to the parallel stripes of a Gabor function in degrees, \f$\psi\f$ is the phase offset in degrees, 
      and \f$\gamma\f$ is the spatial aspect ratio, and specifies the ellipticity of the support of the Gabor function.

      Gabor filters are directly related to Gabor wavelets, since they can be designed for number of dilations and rotations.
      However, in general, expansion is not applied for Gabor wavelets, since this requires computation of biorthogonal
      wavelets, which may be very time-consuming. Therefore, usually, a filter bank consisting of Gabor filters with various
      scales and rotations is created. The filters are convolved with the signal, resulting in a so-called Gabor space. 
      This process is closely related to processes in the primary visual cortex. The Gabor space is very useful in e.g., 
      image processing applications such as iris recognition. Relations between activations for a specific spatial 
      location are very distinctive between objects in an image. Furthermore, important activations can be extracted
      from the Gabor space in order to create a sparse object representation (cite http://en.wikipedia.org/wiki/Gabor_filter).
      
      \section THECLASS The GaborOp class
      
      The GaborOp class provides basic functionalities for applying Gabor
      filters on Images. To achieve optimal performance, it wraps the 
      ConvolutionOp class to realize the internal image convolution
      operations. Determined by a set op input parameters, it internally
      creates a filter bank that caches all gabor masks. In contrast to
      other filters, it knows two modes:
      -# <b>whole image mode</b> in this mode, the filter bank image applied on the
            whole input image, and an output image is created with one channel for
            each filter. This mode works essentially like all other UnaryOps.
      -# <b>specified center mode</b> here, the filters are applied not on all image
            locations, but on some well defined image locations. The result is
            not an image, but a matrix of filter responses, where the matrix's
            x index references the convolution center, and the y index then 
            defines the filter index on this location. (... some more detail here!)

   

      The GaborOp class provides functionalities for the creation of Gabor-Filter kernels, 
      as well as for applying gabor filter operation on images. As mentioned above, in contrast
      to other convolution operations, Gabor filters are often applied as so called 
      <em>Gabor-Jets</em> at some specified image locations only. A Gabor-Jet complies 
      a stack of gabor kernels that are created by some methodical variation of one, 
      some or all gabor mask parameters.\n
      Each GaborOp object provide function to create a gabor jet internally, whereas
      in the easiest case, there is only one value for each parameter, an consequently, 
      only a single gabor mask is created.\n
      In addition to the parameters mentioned in the formula above, the size of the 
      created gabor kernels must be set, and the parameter values must be adapted to
      to it. In the following, each parameter is explained again, but this time with 
      respect to its underlying effect for a kernel size of KWxKH.
      
      - \f$\lambda\f$ wave length of pixels (e.g. if set to KW, the wave will oscillate
        exactly once for direction (\f$\theta=0\f$)
      - \f$\theta\f$ orientation of the wave (0-> wave direction is 3 o'clock, 
        \f$\frac{\pi}{2}\f$ -> wave direction is 12 o'clock)
      - \f$\psi\f$ phase shift of the wave
      - \f$\sigma\f$ std.deviation of the Gaussian multiplied with the wave 
        (in pixels)
      - \f$\gamma\f$ aspect-ratio of the Gaussian
      
  **/
  class GaborOp : public UnaryOp, public Uncopyable{
    public:
    /// creates an empty GaborOp
    GaborOp();
    
    /// creates a new gabor op with given kernel size and parameters
    /** The Gabor-Jet internally created consist of one convolution kernel
        for each possible combination of the parameter values. E.g. if
        the parameters are:
        - lambdas = {5}
        - thetas = {1,2}
        - psis = {0}
        - sigmas = {100,200}
        - and gammas = {1]
        
        the gabor jet consist of 4 convolution kernels with fixed params
        \f$\lambda=5\f$, \f$\psi=0\f$, \f$\gamma=1\f$
        and variable params 
        -# \f$\theta=1\f$ and \f$\sigma=100\f$
        -# \f$\theta=2\f$ and \f$\sigma=100\f$
        -# \f$\theta=1\f$ and \f$\sigma=200\f$
        -# \f$\theta=2\f$ and \f$\sigma=200\f$
    */
    GaborOp(const Size &kernelSize,
            std::vector<icl32f> lambdas,
            std::vector<icl32f> thetas,
            std::vector<icl32f> psis,
            std::vector<icl32f> sigmas,
            std::vector<icl32f> gammas );

    ~GaborOp();

    /// sets the current kernel size
    /** if the kernels have already been created, they are updated to this
        new size value */
    void setKernelSize(const Size &size);
    
    /// add a new lambda value
    void addLambda(float lambda);

    /// add a new theta value
    void addTheta(float theta);

    /// add a new psi value
    void addPsi(float psi);

    /// add a new sigma value
    void addSigma(float sigma);

    /// add a new gamma value
    void addGamma(float gamma);
    
    /// update the current kernels by currently possible value combinations
    void updateKernels();
    
    /// apply all filters to an image
    /** The output image gets as many channels as kernels could be created by
        combining given parameters. Channels c of ppoDst is complies the 
        convolution result of the c-th kernel. */
    virtual void apply(const ImgBase *poSrc, ImgBase **ppoDst);

    /// Import unaryOps apply function without destination image
    UnaryOp::apply;

    /// apply all filters to an image at a specific position
    /** The result vector contains the filter-response for all
        kernels */
    std::vector<icl32f> apply(const ImgBase *poSrc, const Point &p);
    
    /// static function to create a gabor kernel by given gabor parameters
    /** As reminder:
      \f[
      g(x,y;\lambda,\theta,\psi,\sigma,\gamma)=\exp(-\frac{x'^2+\gamma^2y'^2}{2\sigma^2})\cos(2\pi\frac{x'}{\lambda}+\psi)
      \f]
      where \f[ x' = x \cos\theta + y \sin\theta\,  \f]
      and \f[ y' = -x \sin\theta + y \cos\theta\, \f]
    */
    static Img32f *createKernel(const Size &size, float lambda, float theta, float psi, float sigma, float gamma);

    /// returns all currently created kernels
    const std::vector<Img32f> &getKernels() const { return m_vecKernels; }
   
    
    private:
    std::vector<icl32f> m_vecLambdas;
    std::vector<icl32f> m_vecThetas;
    std::vector<icl32f> m_vecPsis;
    std::vector<icl32f> m_vecSigmas;
    std::vector<icl32f> m_vecGammas;
    
    std::vector<Img32f> m_vecKernels;
    std::vector<ImgBase*> m_vecResults;
    Size m_oKernelSize;
  };
}



#endif
