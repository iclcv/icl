#ifndef KMEANS2D_H
#define KMEANS2D_H

#include <vector>
#include <iclMathematics.h>


namespace icl{
  
  /// Quick implementation of the KMeans vector Quantisation algorithms optimized for 2D data \ingroup G_UTILS
  /** The KMeans algorithm for vector quatisation can be defined as follows:
      <pre>
      Given data set D of |D| elements with D = d_1,...,d_|D|
      Given center count K with 
      Optionally given data set C = c1,..,cK of initial centers (prototypes) of size |C|=K
      
      algorithm:
      
      for i = 1 to MAX-STEPS do
         for k = 1 to K
            Vc : = { di | |di-ck| < |di-co| for all o!=k } ( Voronoi cell for center c )
         endfor

         for k = 1 to K
            ck := mean(Vc)
         endfor
      
         // current quatisation is given by the set of centers C
         
         if current quatisation error < minError 
            break
         endif
      endfor
      </pre>
      
  **/
  class KMeans2D{
    public:
    /// static euclidian norm function
    static float euclNorm(float *a, float *b);
    
    /// internal used type for a set of vectors
    typedef std::vector<float*> vectorset;
    
    /// internal used type for an vector of integers
    typedef std::vector<int> intvec;
    
    /// internal used type for distance mesurement functions
    typedef float (*distfunc) (float*, float*);
    
    /// Creates a new KMeans object initialized with a given prototype count k and distance function pointer
    KMeans2D(int k, distfunc f=&KMeans2D::euclNorm);
    
    /// adds a new vector to the KMeans object
    void add(float x,float y);
    
    /// iterates kmeans maxSteps or until the minQuantisationError is reached
    /** @param maxSteps step count (break criterion 1)
        @param minQuantisationError min quatisation error to reach, if the current error
                                    becomes smaller than minQuantisationError the iteration 
                                    is stopped (break criterion 1)
        @param prototypes optional initial prototype vector set. If not given (size is 0)
                          internally a random set of prototypes is chosen from the current data set
        */
    void run(int maxSteps, float minQuantisationError=0, const vectorset &prototypes=vectorset());
    
    /// internally sets the number of centers to use
    void setK(int k);
    
    /// clears all contained data
    void clear();

    /// returns the currently used center count
    int getK() const;
    
    /// returns the current errro
    float getError() const;
    
    /// returns the current set of prototypes
    const vectorset &getPrototypes() const;


    protected:
    
    float m_fError;          /**!< current quantisation error */
    vectorset m_vecCBV;      /**!< list of codebook vectors */
    vectorset m_vecData;     /**!< currently used data vectors */
    distfunc m_funcDist;     /**!< used distance functions */
  };
}

#endif
