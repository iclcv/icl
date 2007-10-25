#ifndef ICL_SOM2D_H
#define ICL_SOM2D_H

#include <iclSOM.h>

namespace icl{

  /// Simple Wrapper class of the generic SOM Implementation for 2D SOMs
  /** As the SOM class provides a <em>very</em> abstract interface for 
      generic K-D Self Organizing Maps, the SOM2D class offers a more convenient
      interface for exactly 2D SOMs.\n
      Therefore it has a specialized constructor (now receiving the count
      of x-Cells and y-Cells directly as well as a specialized getNeuron()-
      function.
  */
  class SOM2D : public SOM{

    /** \cond this function is just used here for an inline vector creation*/
    template<class T>
    static inline std::vector<T> vec2(const T &t1, const T &t2){ 
      std::vector<T> v(2);
      v[0] = t1;
      v[1] = t2;
      return v;
    }
    /** \endcond */
    
    public:
    /// Wrapper constructor for 2D SOMs
    /** @param dataDim dimension of data elements and therewith dimension of prototype vectors
        @param nXCells cell count of the SOM in x-direction
        @param yXCells cell count of the SOM in y-direction
        @param prototypeBounds @see SOM
        @param esplion @see SOM
        @param sigma @see SOM
    */
    SOM2D(unsigned int dataDim, 
          unsigned int nXCells, 
          unsigned int nYCells, 
          const std::vector<Range<float> >& prototypeBounds, 
          float epsilon=0.1, 
          float sigma=1):SOM(dataDim,vec2(nXCells,nYCells),prototypeBounds,epsilon,sigma){}

    /// Wrapper function to access a neuron at a certain grid position
    /** @param x x-grid locatio of the neuron
        @param y y-grid locatio of the neuron
    **/
    const Neuron &getNeuron(int x, int y) const { return m_vecNeurons[x+m_vecDimOffsets[1]*y]; }

  };
}

#endif
