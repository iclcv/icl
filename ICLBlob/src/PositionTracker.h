#include "Extrapolator.h"
#include <vector>
#include <deque>

/**
                         | o   o  x(t)  o   o
-------------------------+-----------------------
   o      o     o      o | d   d   d    d   d
                         |
   o      o     o      o | d   ...
                         |
x(t-3) x(t-2) x(t-1) ŷ(t)|
                         |
   o      o     o      o |
                         |
   o      o     o      o |
                         |
*/
namespace icl{
  
  
  template<class valueType>
  class PositionTracker{
    public:
    void pushData(valueType *xys, int n);
    void pushData(const std::vector<valueType> &xs, const std::vector<valueType> &ys);
    int getID(valueType x, valueType y);
    
    private:
    typedef std::vector<valueType> Vec;
    typedef std::vector<Vec> Mat;
    typedef std::deque<Vec> QMat;

    QMat m_matData[2];
    std::vector<int> m_vecCurrentAssignment;
    std::vector<int> m_vecIDs;
  };
  
  
}
