#include "iclMultiThreader.h"
#include <QCoreApplication>
#include <iclUnaryCompareOp.h>
#include <iclQuick.h>
#include <iclMacros.h>

static int next_functor_id = 0;

class CompareFunctor{
  UnaryCompareOp *m_poOp;
  const ImgBase *m_poSrc;
  ImgBase **m_ppoDst;
public:
  CompareFunctor(UnaryCompareOp::optype ot= UnaryCompareOp::lt, icl64f value=128, icl64f tolerance=0):
    m_poOp(new UnaryCompareOp(ot,value,tolerance)),m_poSrc(0),m_ppoDst(0){
    id = next_functor_id++;
    iCalled = 1;
  }  
  void setup(const ImgBase *src, ImgBase **dst){
    m_poSrc = src;
    m_ppoDst = dst;
  }
  void operator()(){
    printf("compare functor %d called %d times \n",id,iCalled++);
    //    for(int i=0;i<1;i++){
      m_poOp->apply(m_poSrc,m_ppoDst);
      // }
  }
  int iCalled;
  int id;
};

vector<Rect> split_rect(const Rect &r, int n){
  
  vector<Rect> rs(n);
  ICLASSERT_RETURN_VAL(n>0,rs);
  ICLASSERT_RETURN_VAL(( r.height%n) == 0, rs);
  int part = r.height/n;
  for(int i=0;i<n;i++){
    rs[i] = Rect(r.x,r.y+i*part,r.width,part);
  }
  return rs;
}

void spit_image(ImgBase *src,ImgBase *dst, vector<ImgBase*> &srcs, vector<ImgBase*> &dsts, int n){
  ICLASSERT_RETURN(src);
  ICLASSERT_RETURN(dst);
  ICLASSERT_RETURN(src->getROISize() == dst->getROISize());

  srcs.resize(n);
  dsts.resize(n);
  
  vector<Rect> srcROIs = split_rect(src->getROI(),n);
  vector<Rect> dstROIs = split_rect(dst->getROI(),n);
  
  for(int i=0;i<n;i++){
    srcs[i] = src->shallowCopy(srcROIs[i]);
    dsts[i] = dst->shallowCopy(dstROIs[i]);
  }
}

int main (int n, char **ppc){
  
  const int N = 2;  
  printf("main 0.1 \n");
  MultiThreader<CompareFunctor> mt(N);
  printf("main 0.2 \n");
  
  ImgQ image = create("parrot");
  printf("main 0.3 \n");

  ImgBase *dst = new Img8u(image.getSize(),image.getFormat());
  printf("main 1 \n");
  vector<ImgBase*> srcs;
  vector<ImgBase*> dsts;
  printf("main 2 \n");
  spit_image(&image,dst,srcs,dsts,N);
  printf("main 3 \n");
  vector<CompareFunctor*> functors(N);

  printf("main 4 \n");
  for(int i=0;i<N;i++){
    functors[i] = new CompareFunctor;
  }
  printf("main 5 \n");
  for(int i=0;i<20;i++){

    for(int j=0;j<N;j++){
      functors[j]->setup(srcs[j],&(dsts[j]));
    }
    mt.apply(functors);
  }
  printf("main 9 \n");
  
  return 0;
}
