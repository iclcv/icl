#ifndef CONFIG_H
#define CONFIG_H

#include "types.h"

static const int A=10;
static string all[A]={
  "IPP","LIBDC","UNICAP","IMAGEMAGICK","LIBJPEG",
  "LIBZ","VIDEODEV","SVS","QT","XCF"
};

static const int S=4;
static const string simple[S]={"SVS","LIBZ","LIBJPEG","IMAGEMAGICK"};

static const string def="/usr";
static const int M=3;
static const string make_defs[M][2]={
  {"IPP","/vol/vision/IPP/6.0"},
  {"XCF","/vol/xcf"},
  {"LIBZ","/usr"}
};
static const int O=4;
static const string overwrite_defs[O][2]={
  {"IPP","/vol/vision/IPP/6.0"},
  {"SVS","/vol/vision/SVS/4.2"},
  {"UNICAP","/vol/video"},
  {"LIBC","/vol/video"}
};

static const string delim="§";
static const string deactivate="SWITCH_OFF";

#define DEFINE_COMPLEX                                  \
  vector<string> complex;                               \
  for(int i=0;i<A;++i){                                 \
    if(find(simple,simple+S,all[i]) == simple+S){       \
      complex.push_back(all[i]);                        \
    }                                                   \
  }                                                     \
  const int C = (int)complex.size(); 




#endif


