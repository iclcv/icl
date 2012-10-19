#include <ICLUtils/StringUtils.h>
#include <ICLUtils/Any.h>
#include <ICLUtils/Random.h>
#include <ICLUtils/Time.h>

using namespace icl::utils;

void bench(){
  std::vector<float> v(1000);
  std::fill(v.begin(),v.end(),GRand(0,1));
  
  Time t = Time::now();
  for(int i=0;i<1000;++i){
    Any a = v;
    std::vector<float> b = a;
  }
  t.showAge("any");


  t = Time::now();
  for(int i=0;i<1000;++i){
    std::string a = cat(v,",");
    std::vector<float> b = parseVecStr<float>(a);
  }
  t.showAge("cat / parse");
}

int main(int n, char **ppc){
  float data[] = { 1,2,3,4,5,6,7,8,9,10 };
  std::vector<float> v(data,data+10);
  
  Any a = v;
  
  std::vector<float> v2 = a;
  
  bench();
  
  std::cout << "[" << cat(v2,",") << "]" << std::endl;
}
