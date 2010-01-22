#ifndef ICL_MEAN_SHIFT_TRACKER_H
#define ICL_MEAN_SHIFT_TRACKER_H

#include <ICLCore/Img.h>

namespace icl {

  /** MeanShiftTracker
      
      The MeanShiftTracker is a tracking component using the MeanShift algorithm.
      
      Given a, preferably formatGrey, so called weightImage, it can
      find a distribution maxima of the feature tracked in the workingImage. The workingImage
      must contain weights for each pixel, determing the likelihood of the pixel belonging to
      the feature being tracked. High values correspond to a high likelihood.
      
      The MeanShiftTracker uses the MeanShift algorithm for tracking blobs in realtime.
      It is an efficient technique for tracking 2D blobs through an image.
      Given a, preferably formatGrey, weightImage, it can find a distribution maxima of 
      the feature tracked. The weightImage must contain weights for each pixel, 
      determing the likelihood of the pixel belonging to the blob. High values 
      correspond to a high likelihood.
      The Meanshift algorithm is resistent to camera movements, partial covering, scaling
      and noise.
      The Meanshift algorithm is also useful in image segmentation.
      
      \section SEC_ALGORITM The MeanShift algorithm
      As stated above, the MeanShiftTracker uses the MeanShift algorithm to find the new
      center of the blob being tracked. This is the MeanShift algorithm:
      \f[P_n = \frac{\sum_{A_h}k(A_h-P_o)w(A_h)P_o}{\sum_{A_h} | k(A_h-P_o) w(A_h)|} \f], where
      \f$P_n \f$ is the new center of the blob, 
      \f$P_o \f$ is the previous center, where the algorithm starts, 
      \f$k() \f$ is the kernel function, 
      \f$w() \f$ is the weight function and
      \f$A_h \f$ are all pixels located in a squarish region of the size 2*bandwidth
      around \f$P_o \f$.
      
      \section SEC_BANDWIDTH The Bandwidth
      The Bandwidth is the only parameter for this algorithm. It is half the length of the
      squarish mask containing all pixels \f$A\f$. Choosing a correct size for this mask is
      essential. It should approximatly cover the blob being tracked.
      
      \section SEC_KERNEL The Kernel
      The Kernel determines how much each pixel \f$A\f$ contributes to the outcome of the formula.
      Using different kernels changes the speed and precision of the algorithm. So far, the 
      Epanechnikov kernel, which is used as the default kernel for this algorithm, due to 
      its good performance, and a Gauss kernel have been implemented.
      
      
      \section SEC_WEIGHTIMAGE The weightImage
      This implementation of the Meanshift algorithm does not work with the original image.
      Only a one-channel image, having the same size and using the same coordinate system as
      the original image, is needed. In this image, all pixels, that belong to the blob must
      have high values.
      
      \section TODO ToDo
      Add other kernels.
      Add functionality to open any image as kernel image.
      Performance tweaking.
      **/
  class MeanShiftTracker {
    
    public:
    
    /// An enumeration for the different kernel types
    enum kernelType {
      epanechnikov,
      gauss
    }; 
    private:
    
    
    /// The bandwidth of the kernel; equals diameter / 2
    int m_bandwidth;
    
    /// The kerneltype; displayed as int
    kernelType m_kernelType;

    /// An internal representation of the kernel used
    /** An image of the kernel used for the MeanShift. Stored is only the second 
        quadrant of the image, as the values are the same for absolute coordinates.
        */
    Img32f m_kernelImage;
    
    /// Applies a single step of the mean shift algorithm
    /** 
        @param pos start original position
        @return new center	
        */
    Point32f applyMeanShiftStep(const Img32f &image, const Point32f &pos);

    /// Generates an Epanechnikov Kernel
    /** 
        @param a kernel bandwidth
        */
    static Img32f generateEpanechnikov(int bandwidth);

    /// Generates a Gauss Kernel
    /** Generates the internal m_opKernelImage using the Gauss profile
        @param a i dont know
        @param stdDev profiles the standard deviation.
        */
    static Img32f generateGauss(int bandwidth,float stdDev);
    
    public:

    /// Constructor with only the most needed parameters
    /** The basic constructor with only the most needed parameters.
        @param bandwidth The desired kernel bandwidth
        @param kernelType The desired kernel type
        */
    MeanShiftTracker(kernelType type, int bandwidth, float stdDev=1);

    
    /// sets new kernel type (with params)
    /** @param type kernel type
        @param bandwidth new bandwidth
        */
    void setKernel(kernelType type, int bandwidth, float stdDev=1);

    /// returns current kernel type
    int getKernel() const { return m_kernelType;  }
    
    /// returns current kernel image
    const Img32f &getKernelImage() const { return m_kernelImage; }
    
    /// Returns current kernel bandwidth
    int getBandwidth() const { return m_bandwidth; }
    
    /// This function returns a new center after the MeanShift algorithm is applied
    /** @param weightImage gray level input image
        @param initialPoint starting point for meanshift loop
        @param maxCycles A treshold for the maximum iteration coutn if (-1, 10000 is used as upper limit)
        @param convergenceCriterion position difference (in pixels) of consecutive steps that is used as threshold
        for mean shift loop
        @param converged if a non-NULL pointer is given here, the reference boolean is used to notify
        the caller whether actually maxCycles iteration steps were used or the 
        convergence criterion was reached
        */
    const Point32f step(const Img32f &weigthImage, const Point32f &initialPoint,  int maxCycles=-1, float convergenceCriterion=1.0, bool *converged=0);

  };
}

#endif
