#include <vector>
#include <iclMathematics.h>
#ifndef KMEANS2D_H
#define KMEANS2D_H


namespace icl{
  class KMeans2D{
    public:
    static float euclNorm(float *a, float *b);
    typedef std::vector<float*> vectorset;
    typedef std::vector<int> intvec;
    typedef float (*distfunc) (float*, float*);
    
    KMeans2D(int k, distfunc f=&KMeans2D::euclNorm);
    void add(float x,float y);
    void run(int maxSteps, float minQuantisationError=0, const vectorset &prototypes=vectorset());
    void setK(int k);
    void clear();

    int getK() const;
    float getError() const;
    const vectorset &getPrototypes() const;

    protected:
    float m_fError;          /**< current quantisation error */
    vectorset m_vecCBV;      /**< list of codebook vectors */
    vectorset m_vecData;     /**< currently used data vectors */
    distfunc m_funcDist;     /**< used distance functions */
  };
}

#endif
