#ifndef ICLMEDIAN_H
#define ICLMEDIAN_H

#include <ICLBase.h>
#include <ICLFilter.h>

namespace icl{
  class ICLMedian : public ICLFilter{
    public:
    ICLMedian(int iWidth=3, int iHeight=3);
    ~ICLMedian();
    virtual void apply(ICLBase *poSrc, ICLBase *poDst);
    private:
    int iWidth,iHeight;
  };
}

#endif
