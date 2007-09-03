#include "iclMultiThreader.h"
#include <QCoreApplication>
#include <iclUnaryCompareOp.h>
#include <iclQuick.h>
#include <iclMacros.h>

static int next_functor_id = 0;

static const int LARGE_SIZE = 10000000;
static float *large_array = new float[LARGE_SIZE];
static float *large_array2 = new float[LARGE_SIZE];
static icl8u *large_array3 = new icl8u[LARGE_SIZE];
void apply_large_func(int start=0, int end = LARGE_SIZE){
  int size = end-start;
  int h = size/100;
  int w = 100;
  //ippiFilterGauss_32f_C1R(large_array+w+1, sizeof(float)*(w-2), large_array2+w+1, sizeof(float)*(w-2), Size(h-2,w-2),(IppiMaskSize)33);
  ippiCompareC_32f_C1R(large_array, w*sizeof(float), 5.3, large_array3,w,Size(w,h),ippCmpLess);
}

class CompareFunctor{
  UnaryCompareOp *m_poOp;
  const ImgBase *m_poSrc;
  ImgBase **m_ppoDst;
public:
  CompareFunctor(UnaryCompareOp::optype ot= UnaryCompareOp::eq, icl64f value=128, icl64f tolerance=5):
    m_poOp(new UnaryCompareOp(ot,value,tolerance)),m_poSrc(0),m_ppoDst(0){
    m_poOp->setClipToROI(false);
    m_poOp->setCheckOnly(true);
    id = next_functor_id++;
    iCalled = 1;
  }  
  void setup(const ImgBase *src, ImgBase **dst){
    m_poSrc = src;
    m_ppoDst = dst;
  }
  void operator()(){
    m_poOp->apply(m_poSrc,m_ppoDst);
    apply_large_func(0,LARGE_SIZE/2);
  }
  int iCalled;
  int id;
};

class LargeFuncFunctor{
public:
  LargeFuncFunctor(int iStart, int iEnd):a(iStart),b(iEnd){}
  void operator()(){
    apply_large_func(a,b);
  }
private:
  int a,b;
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

int main2(){
  const int N = 2;
  const int K = 100;
  MultiThreader<LargeFuncFunctor> mt(N);
  vector<LargeFuncFunctor*> functors(N);
  for(int i=0;i<N;i++){
    functors[i] = new LargeFuncFunctor(i?LARGE_SIZE/2:0,i?LARGE_SIZE:LARGE_SIZE/2);
  }
  tic();
  for(int i=0;i<K;i++){
    mt.apply(functors);
  }
  toc();

  mt.stopThreads();

  
  tic();
  for(int i=0;i<K;i++){
    apply_large_func();
  }
  toc();

  return 0;
}

int main(int n, char **ppc){
  
  const int N = 2;  

  MultiThreader<CompareFunctor> mt(N);
  
  ImgQ image = create("parrot");

  ImgBase *dst = new Img8u(image.getSize(),image.getFormat());
  vector<ImgBase*> srcs;
  vector<ImgBase*> dsts;

  spit_image(&image,dst,srcs,dsts,N);
  vector<CompareFunctor*> functors(N);

  for(int i=0;i<N;i++){
    functors[i] = new CompareFunctor;
  }
  tic();
  for(int i=0;i<100;i++){
    for(int j=0;j<N;j++){
      functors[j]->setup(srcs[j],&(dsts[j]));
    }
    mt.apply(functors);
  }
  toc();
  printf("ok! stopping threads! \n");
  mt.stopThreads();
  printf("ok 2 threads stopped! \n");
  
  ImgQ image2 = create("parrot");
  ImgBase *dst2=0;
  UnaryCompareOp op(UnaryCompareOp::eq,128,5);
  op.apply(&image2,&dst2);
  tic();
  for(int i=0;i<100;i++){
    op.apply(&image2,&dst2);
    apply_large_func(0,LARGE_SIZE);
  }
  toc();
  return 0;
}
