#ifndef ICL_VECTOR_TRACKER_H 
#define ICL_VECTOR_TRACKER_H 

#include <vector>

namespace icl{

  /// Extension of the position tracker class for N-dimensional positions
  /** Here's a copy of the PositionTracker documentation, which assumes 2D-input data:
      \copydoc icl::PositionTracker
  */
  class VectorTracker{
    public:
    /// Determines how ids are allocated internally
    enum IDmode{
      firstFree, // old unused IDs are re-used
      brandNew   // each new element gets a brand new ID (untils int-range ends)
    };
    
    /// Vector Type
    typedef std::vector<float> Vec;

    /// Creates an empty (null) vector tracker (isNull() returns true then)
    VectorTracker();
    
    /// Creates new VectorTracker instance with given parameters
    /** @param dim data dimension (this must not changed at runtime)
        @param largeDistance to tackle element count changes, the distance matrix is padded with
               largeDistnace values, this must be much (e.g. 100 times ) larger then the largest real distance, that
               can be expected from the data. We can't use some fixed value here, as too large values lead to
               numerical problems
        @param normFactors Internally the euclidian distance metric can be normalized in differenct dimensions seperately:
                          \f[ d(a,b) = \sqrt{ \sum\limits_{i=1}^D \left( \frac{a_i - b_i}{\sigma_i}\right)^2 } \f]
                          In literature this is norm is reference as normalized euclidian distance. Actually we use an
                          adapted instance of this norm:
                          \f[ d(a,b) = \sqrt[4]{ \sum\limits_{i=1}^D \left( \frac{a_i - b_i}{\sigma_i}\right)^2 } \f]
                          As mentioned in the documentation of the PositionTracker, it's compulsory to use the 
                          root of the actual norm to avoid new entries are mixed up with old ones.
                          The norm factor vector contains the \f$\sigma_i\f$ that are used in the formula above. If norm-factor
                          is empty or all entries are set to 1, the default euclidian norm (more precisely the square 
                          root of it) is used, which increases performance slightly. If normFactor contains zeros,
                          div-0 errors would occur, so this is checked during initialization.
        @param idMode This feature is taken from the recent PositionTracker update. It decides whether to re-use
                      old IDs, that got free again due to the disappearing of the associated entry or to assign a
                      brand new ID each time a new entry is found
        @param distanceThreshold As a first optimization a trivial assignment is tested. If entry count hasn't changed
                                 and each old entry can be assigned indisputably to a single new entry and each distance
                                 between estimation and current observation is beyond that threshold, the trivial
                                 assignment is used.
        @param tryOpt enables/disables whether to test for trivial assignment. If a trivial assignment can be expected, this
                      will increase performance significantly. If it's more likely, that trivial assignment will fail, this 
                      also reduce performance a little bit.
    */
    VectorTracker(int dim, float largeDistance, const std::vector<float> &normFactors=std::vector<float>(),
                  IDmode idMode=firstFree, float distanceThreshold=0, bool tryOpt=true); 

    /// Deep copy constructor (all data and current state is copied deeply)
    /** New instance is absolutely independent from the source instance */
    VectorTracker(const VectorTracker &other);
    
    /// assignment (all data and current state is copied deeply)
    /** New instance is absolutely independent from the source instance */
    VectorTracker &operator=(const VectorTracker &other);
    
    /// Destructor
    ~VectorTracker();

    /// next step function most efficient version
    void pushData(const std::vector<Vec> &newData);

    /// returns runtime id of last pushed data element index
    int getID(int index) const;
    
    /// returns whether VectorTracker instance is currently null (created with default constructor)
    bool isNull() const;

    /// return current data dimension
    int getDim() const;
    
    private:
    
    /// internal data structure (declared and used in iclVectorTracker.cpp only)
    struct Data;
    
    /// internal data pointer
    Data *m_data;
  };
  
  
}
#endif
