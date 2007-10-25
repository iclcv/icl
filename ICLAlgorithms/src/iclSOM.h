#ifndef ICL_SOM_H 
#define ICL_SOM_H

#include <iclMathematics.h>
#include <iclRange.h>
#include <vector>

namespace icl{

  /// Generic implementation of D to K dim Self Organizing Map (SOM)
  /** Input dimension as well as output dimension is freely configurable, i.e. SOM
      neurons can be aligned in a K dimensional grid with arbitrary cell count for 
      each dimension. For more convenience
      an inline wrapper class called SOM2D is also available in "iclSOM2D.h"
      
      <h3>Nomenclature</h3>

      - list of neurons: \f$W=\{n_1,...,n_N\}\f$
      - best matching neuron index: \f$s\f$
      - prototype vector of neuron \f$n_i\f$: \f$n_i.p\f$ of dim \f$D\f$
      - grid position vector of neuron \f$n_i\f$: \f$n_i.g\f$ of dim \f$K\f$
      - learning rate \f$\epsilon\f$

      <h3>Prototype Update</h3>

      - new input vector: \f$x\f$

      \f[
      \forall r \in \{1,...,n\} \\
      \Delta n_r.p = \epsilon \cdot h(n_r.g,h_s.g) \cdot (x-n_r.p)
      \f]
      
      <h3>Grid distance function (\f$h\f$) </h3>
      \f[
      h(a,b) = e^{-\frac{(a-b)^2}{s\sigma^2}}
      \f]
      
  **/
  class SOM{
    public:
    /// SOM internal Neuron struct
    /** The SOM Neuron struct is defined by grid-location vector and prototype vector. In 
        addition, each neuron has a void* meta, that can be used to associate some 
        meta-data with a certain neuron (Note,the pointer meta is not released automatically).*/
    struct Neuron{
      /// create an empty neuron with 0 pointers and dimensions
      Neuron();
      
      /// create a new neurons with given pointers and dimensions
      /** If the deepCopyData flag is set to true, internally new prototype and gridpos pointers are allocated
          and filled with the given pointer's data. Otherwise, the ownership of the given pointers is passed to
          the this neuron.
      */
      Neuron(float *gridpos,float *prototype,unsigned int griddim, unsigned int datadim, bool deepCopyData=false);

      /// Deep copy constructor
      Neuron(const Neuron &other);
      
      /// Assignment operator (applies a deep copy of the neuron)
      Neuron &operator=(const Neuron &other);
      
      /// Destructor
      ~Neuron();
      
      /// grid position vector of dim "griddim"
      float *gridpos;
      
      /// prototype vector of dim "datadim"
      float *prototype;

      /// grid dimension
      unsigned int griddim;
      
      /// prototype dimension
      unsigned int datadim;
      
      /// meta-data storage pointer
      void *meta;
    };
    
    /// create a new SOM with given data dimension and grid dimensions. 
    /** @param dataDim dimension of data elements and prototype vectors 
        @param dims grid dimension array. dims.size() defines the SOM'S grid dimension. dims[i] defines
                    the cell count for dimension i. E.g. to create a 2D 40x40 SOM grid dims has to be
                    \f$\{40,40\}\f$.
        @param prototypeBounds Internals for new randomly created prototype vector elements. prototypeBounds.size()
                               must be equal to dataDim. E.g. to initialize a SOM with prototypes randomly 
                               distributed in a dataDim-D unity cube \f$[-1,1]^{dataDim}\f$, prototypeBounds must be 
                               \f$\{(-1,1),(-1,1),...\}\f$ (where the ranges are written as tuples).
        @param epsilon initial learning rate
        @param sigma initial standard deviation for the grid distance function \f$h(a,b)\f$
    */
    SOM(unsigned int dataDim, const std::vector<unsigned int> &dims,const std::vector<Range<float> > &prototypeBounds, float epsilon=0.1, float sigma=1);

    /// Destructor
    ~SOM();
    
    /// trains the net using given input vector and current parameters epsilon and sigma
    void train(const float *input);
    
    /// returns a reference of the winner neuron dependent on a given input vector (const)
    const Neuron &getWinner(const float *input) const;

    /// returns a reference of the winner neuron dependent on a given input vector
    Neuron &getWinner(const float *input);
    
    /// returns the neuron set (const)
    const std::vector<Neuron> &getNeurons() const; 
    
    /// returns the neuron set
    std::vector<Neuron> &getNeurons();
    
    /// returns a neuron at given grid location (const)
    const Neuron &getNeuron(const std::vector<int> &dims) const;

    /// returns a neuron at given grid location
    Neuron &getNeuron(const std::vector<int> &dims);
    
    /// returns the data dimension
    unsigned int getDataDim() const { return m_uiDataDim; }

    /// returns the SOM dimension (e.g. 2D, 3D ..)
    unsigned int getSomDim() const { return m_uiSomDim; }
    
    /// returns the SOM's grid dimension (e.g. 20x40 for a 2D SOM)
    const std::vector<unsigned int> getDimensions() const { return m_vecDimensions; }

    /// sets the current learning rate epsilon
    void setEpsilon(float epsilon);
    
    /// sets the current grid distance function standard deviation
    void setSigma(float sigma);
    
    protected:
    
    /// internal data dimension variable
    unsigned int m_uiDataDim;
    
    /// internal SOM dimension variable ( = m_vecPrototypeBounds.size() = m_vecDimensions.size() )
    unsigned int m_uiSomDim;
    
    /// internal grid dimensions
    std::vector<unsigned int> m_vecDimensions;
    
    /// internal bounds for prototype ranges (todo: is it necessary to store them ?)
    std::vector<Range<float> > m_vecPrototypeBounds;
    
    /// set of neurons
    std::vector<Neuron> m_vecNeurons;
    
    /// internal utility offset vector for each dimension
    std::vector<unsigned int> m_vecDimOffsets;
    
    /// learning rate
    float m_fEpsilon; 

    /// standard deviation for the grid distance function 
    float m_fSigma;   
  };
  

}


#endif
