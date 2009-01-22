#include "show_arglists.h"
#include <iterator>
#include <algorithm>
#include <iostream>

#include "config.h"

using namespace std;

void show_arglists(const std::vector<arglist> &l, bool concise){
  DEFINE_COMPLEX; (void)C;

  cout << "simple: " << endl;
  std::copy(simple,simple+S,ostream_iterator<std::string>(cout," "));
  cout << endl << "complex: " << endl;
  std::copy(complex.begin(),complex.end(),ostream_iterator<std::string>(cout," "));
  cout << endl;

  for(unsigned int i=0;i<l.size();++i){
    const arglist &a = l[i];
    for(arglist::const_iterator it=a.begin(); it != a.end() ;++it){
      if(concise){
        cout << (  it->endsWith(deactivate.c_str()  ) ? "X" : "O" ) << " ";
      }else{
        cout << (*it).toLatin1().data() << " ";
      }
    }
    cout << endl; 
  }
}
