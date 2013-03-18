#include <ICLQt/Common.h>
#include <ICLUtils/Random.h>

#include <set>

GUI gui;
std::set<std::string> used;
const int N = 100;

std::vector<int> extr;

void init(){
  for(int i=0;i<N;++i){
    std::string s(' ',30);
    std::fill(s.begin(),s.end(), URand('a','z'));
    s[9] = '\0';
    gui << Int(0).out(s);
    used.insert(s);
  }

  gui << Show();
  extr.reserve(500000);
}

void run(){
  Time t = Time::now();
  for(int i=0;i<1000;++i){
    for(std::set<std::string>::const_iterator it = used.begin(); it != used.end(); ++it){
      int x = gui[*it];
      extr.push_back(x);
    }
  }
  double dt = t.age().toMilliSecondsDouble() / (N);
  SHOW(dt);
}

int main(int n, char **args){
  return ICLApp(n,args,"",init,run).exec();
}
