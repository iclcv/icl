#ifndef ICL_SCAN_LINE_H
#define ICL_SCAN_LINE_H
namespace icl{
  class ScanLine{
    public:
    ScanLine(int x=0,int y=0, int len=0):x(x),y(y),len(len){}
    int x;
    int y;
    int len;
  };
}

#endif
