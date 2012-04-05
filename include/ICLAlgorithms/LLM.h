/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLAlgorithms/LLM.h                            **
** Module : ICLAlgorithms                                          **
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
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#ifndef ICL_LLM_H
#define ICL_LLM_H

#include <vector>
#include <ICLUtils/Range.h>
#include <ICLUtils/Configurable.h>

namespace icl{
  /// Local Linear Map implementation (LLM)
  /** The LLM-network (Local Linear Maps) is a very powerful regression network that uses 
      a superposition of linearily weighted gaussians as regression model:
      
      <h3>Net Function y=f(x)</h3>
      \f[ 
      \vec{y}^{net}(x) = \sum\limits_{i=1}^N ( \vec{w}_i^{out}+A_i ( \vec{x}-\vec{w}_i^{in} ) ) \cdot g_i(\vec{x})
      \f]
      
      where each kernel i is defined by a kernel function \f$g_i(\vec{x})\f$ that is weighted linearily
      using \f$ ( \vec{w}_i^{out}+A_i ( \vec{x}-\vec{w}_i^{in} ) ) \f$. 
      \f$\vec{w}_i^{in}\f$ is the location in the input space, where kernel i is centered, 
      and \f$\vec{w}_i^{out}\f$ resp. \f$A_i\f$ are parameters of a linear weighting function.

      <h3>Kernel functions</h3>
      Here, normalized Gaussians are always used a kernel functions:
      \f[
      g_i(\vec{x}) = \frac{\exp(-\frac{\|\vec{x}-\vec{w}_i^{in}\|^2}{2\sigma_i^2})}
                          {\sum\limits_{j=1}^N \exp(-\frac{\|\vec{x}-\vec{w}_j^{in}\|^2}{2\sigma_j^2})}
      \f]
      
      <h3>Training of input weights and variances(unsuperwised)</h3>
      The kernel centers (also refered as data prototypes) \f$\vec{w}_i^{in}\f$ can be initialized randomly 
      e.g. using a uniform distribution in input data space or by a set of given prototypes. The prototypes 
      can be adapted while the network training process using means of vector quantisation (e.g. KMeans) 
      externally or by using the following internally implemented update rule:

      \f[
      \Delta \vec{w}_i^{in} = \epsilon_{in} \cdot ( \vec{x} - \vec{w}_i^{in} ) \cdot g_i(\vec{x})
      \f]

      and 
      \f[
      \Delta \sigma^2 = \epsilon_{\sigma} \cdot ( \vec{x} - \vec{w}_i^{in} )^2 - \sigma^2 \cdot g_i(\vec{x})
      \f]
      
      <b>Note:</b> For some initial tries, it turned out, that updating the kernel radii is rather unstable.
      
      <h3>Training of output weights and matrices (superwised)</h3>
      Training rules for output weights \f$\vec{w}_i^{out}\f$ and matrices \f$A_i\f$ are obtained by simple 
      gradient descent approach (given input tuple \f$(\vec{x}^{\alpha},\vec{y}^{\alpha})\f$):
      
      \f[
      \Delta \vec{w}_i^{out} = \epsilon_{out} \cdot g_i(\vec{x}^{\alpha}) \cdot 
                               (\vec{y}^{\alpha} - \vec{y}^{net}(\vec{x}^{\alpha}) ) + 
                               A_i \Delta \vec{w}_i^{in}
      \f]

      and       

      \f[
      \Delta A_i = \epsilon_A \cdot g_i(\vec{x}^{\alpha}) \cdot 
                    (\vec{y}^{\alpha} - \vec{y}^{net}(\vec{x}^{\alpha}) ) \cdot
                    \frac{(\vec{x}^{\alpha} - \vec{w}_i^{in} )^{\tau}}{\|\vec{x}^{\alpha} - \vec{w}_i^{in}\|^2}
      
      \f]

      <h2>Hacks</h2>
      - Delta A_i use sqrt of demonimator (???)
      
      <h2>TODO</h2>
      - check for initialized first or initialized later again
      - check if initialized at all (in train...)
      - enable soft-max on/off
      - provide an interface for Batch based VQ using (EM-Algorithm or something like that)
      - \f[ f_X(x_1, \dots, x_N) = \frac {1} {(2\pi)^{N/2}|\Sigma|^{1/2}} \exp \left( -\frac{1}{2} ( x - \mu)^\top \Sigma^{-1} (x - \mu) \right) \f]
  **/
  class LLM : public Configurable{
    public:
    public:
    /// Internally used Kernel structure
    struct Kernel{
      /// Empty base constructor
      Kernel();
      /// Default constructur with given input an output dimension
      Kernel(unsigned int inputDim, unsigned int outputDim);
      /// Copy constructor (deep copy)
      Kernel(const Kernel &k);
      /// Destructor
      ~Kernel();
      /// Assignment operator (deep copy)
      Kernel &operator=(const Kernel &k);

      /// input weight (prototype vector) of this kernel
      float *w_in;
      
      /// output weight of this kernel
      float *w_out;
      
      /// matrix for the linear map
      float *A;
      
      ///  buffer for last input weight update
      float *dw_in;
      
      /// variance-vector of the Gaussian kernel (trace(Cxx))
      float *var; 
      
      /// input dimension
      unsigned int inputDim;
      
      /// output dimension
      unsigned int outputDim;

      /// shows a kernel to std::out
      void show(unsigned int idx=0) const;

      /// updates the kernels internal values
      /** the given data is copied deeply into the kernel, i.e. the kernel's
          internal data pointers remain untouched. Only the given data is 
          is copied to where the data pointers point.
          @param w_in input weight vector (needs to be of size inputDim)
          @param w_out output weight vector (needs to be of size outputDim)
          @param A slope matrix (needs to be of size inputDim * outputDim)
                   the data layout is rowmajor
          */
      void set(const float *w_in, const float *w_out, const float *A);
    };
    

    static const int TRAIN_CENTERS = 1; /*!< training flag for updating input weights/prototype vectors */
    static const int TRAIN_SIGMAS = 2;  /*!< training flag for updating input sigmas */
    static const int TRAIN_OUTPUTS = 4; /*!< training flag for updating output weights */
    static const int TRAIN_MATRICES = 8;/*!< training flag for updating output matrices */
    static const int TRAIN_ALL = TRAIN_CENTERS | TRAIN_SIGMAS | TRAIN_OUTPUTS | TRAIN_MATRICES; /*!< training flag for updating all*/

    private:
    void init_private(unsigned int inputDim,unsigned int outputDim);

    public:

    /// Creates a new llm with given input and output dimensions
    LLM(unsigned int inputDim, unsigned int outputDim);

    LLM(unsigned int inputDim, unsigned int outputDim, unsigned int numCenters, 
        const std::vector<Range<icl32f> > &ranges, 
        const std::vector<float> &var=std::vector<float>(1,1));

    /// initializes the LLM prototypes with given kernel count
    /** Internally creates numCenters kernels for this LLM. Each kernel is initialized as follows:
        - input weight w_in: created with respect to ranges (w_in[i] becomes a value drawn from a uniform distibution of range ranges[i])
        - output weight w_out: initialized to 0-vector
        - Slope Matrix A: initialized to 0-matrix
        - variance sigma: initialized to given variance vector var 

        (internally this init function calls the other init function)

        @param numCenters new prototype count
        @param ranges ranges for uniform distributed prototypes (ranges.size must be equal to the input dimension of the LLM)
        @param var variance for the new created Gaussians (var.size must be equal to the input dimension of the LLM)
    **/
    void init(unsigned int numCenters, const std::vector<Range<icl32f> > &ranges, const std::vector<float> &var=std::vector<float>(1,1));
    
    /// initializes the LLM prototypes with given set of kernels
    /** Internall all centers.size() vectors are used to create that many prototypes. The given vectors are deeply copied and not released
        internally. Initialization is performend as described in the above function.
    **/
    void init(const std::vector<float*> &centers, const std::vector<float> &var=std::vector<float>(1,1));

    /// applies the LLM and returns current NET output vector 
    /** the ownership of the returnd float * is not passed to the caller. The output remains valid until
        apply is called with another x-value */
    const float *apply(const float *x);
    
    /// applies an update step to the net with given net input x and destination output y
    /** The trainingflags parameter can be used to specify special parameter updates, a list of 
        training targets can be creates using bit-or: e.g.: TRAIN_CENTERS|TRAIN_SIGMAS */
    void train(const float *x,const float *y, int trainflags = TRAIN_ALL);

    /// trains the input weights w_in
    void trainCenters(const float *x);

    /// trains the input variances sigma
    void trainSigmas(const float *x);

    /// trains the output weights w_out
    void trainOutputs(const float *x,const float *y);
    
    /// trains the output slope matrices A
    void trainMatrices(const float *x,const float *y);

    private:
    /// possible usefull utility function (internally used)
    /** updates an internal variable vector containing provisional results for all g_i[x]. This internally speeds up
        performance (please dont call this function!, propably it can be made private) **/
    const float *updateGs(const float *x);
    
    public:
    /// returns the current error vector with respect to a given destination value y
    const float *getErrorVec(const float *x, const float *y);
    

    /// sets up learning rate for input weights to a new value (about 0..1)
    void setEpsilonIn(float val) { setPropertyValue("epsilon In",val); }

    /// sets up learning rate for output weights to a new value (about 0..1)
    void setEpsilonOut(float val) { setPropertyValue("epsilon Out",val); }

    /// sets up learning rate for slope matrices to a new value (about 0..1)
    void setEpsilonA(float val) { setPropertyValue("epsilon A",val); }
    
    /// sets up learning rate for sigmas to a new value (about 0..1)
    /** <b>Note</b> Update of the sigmas does not run very good! */
    void setEpsilonSigma(float val) { setPropertyValue("epsilon Sigma",val); }
    
    /// Shows all current kernels to std::out
    void showKernels() const;
    
    /// returns the current internal kernel count
    unsigned int numKernels() const { return m_kernels.size(); }
    
    /// returns a specifice kernel at given index (const)
    const Kernel &operator[](unsigned int i) const { return m_kernels[i]; }

    /// returns a specifice kernel (unconst version)
    /** <b>please note:</b> it is not safe to change the obtained Kernel's
        value by hand unless you really know what you are doing. For setting
        the kernel weights manually, it is strongly recommended to use the 
        Kernel::set-method, which does only allow to change the weight values */
    Kernel &operator[](unsigned int i) { return m_kernels[i]; }
    
    /// returns whether the softmax function for calculation for g_i[x] is used
    bool isSoftMaxUsed() const { return const_cast<Configurable*>(static_cast<const Configurable*>(this))->getPropertyValue("soft max enabled").as<bool>(); }
    
    /// sets whether the softmax function for calculation for g_i[x] is used
    void setSoftMaxEnabled(bool enabled) { setPropertyValue("soft max enabled",enabled); }

    private:

    /// internal training function for the input weights/prototypes
    void trainCentersIntern(const float *x,const float *g);
    
    /// interal training function for the sigmas
    void trainSigmasIntern(const float *x,const float *g);

    /// internal training function for the output weights
    void trainOutputsIntern(const float *x,const float *y,const float *g, const float *dy, bool useDeltaWin);
    
    /// internal training function for the slope matrices A
    void trainMatricesIntern(const float *x,const float *y,const float *g, const float *dy);
    
    /// internal apply function
    const float *applyIntern(const float *x,const  float *g);

    /// internal function to return the current error vector
    const float *getErrorVecIntern(const float *y, const float *ynet);

    /// input dimension
    unsigned int m_inputDim;

    /// output dimension
    unsigned int m_outputDim;

#if 0    
    /// learning rate for the input weigts
    float m_epsilonIn;
    
    /// learning rate for the output weigts
    float m_epsilonOut;
    
    /// learning reate tor the slope matrices
    float m_epsilonA;
    
    /// learning rate for the sigmas
    float m_epsilonSigma;
#endif
    /// internal storage for the kernels
    std::vector<Kernel> m_kernels;

    /// internal output value buffer
    std::vector<float> m_outBuf;
    
    /// internal buffer for the g_i[x]
    std::vector<float> m_gBuf;

    /// internal buffer for the current error vector
    std::vector<float> m_errorBuf;

#if 0
    /// internal flag whether soft max is used or not
    bool m_bUseSoftMax;
#endif
  };

}
#endif
