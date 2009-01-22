#include "create_combinations.h"

#include <map>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <sstream>
#include <cmath>
#include <bitset>

#include <QString>
#include <QStringList>

#include <config.h>

QString to_q_str(const string &s) {
  return s.c_str();
}
void add_deactivation(QString &s){
  s+=deactivate.c_str();
}

void add_remaining_combis(const vector<string> &c, const arglist &part1, std::vector<arglist> &l){
  const int C=(int)c.size();
  for(int i=0;i<pow(2.0,C);++i){
    bitset<32> activation(i);
    l.push_back(part1);
    
    for(int p=0;p<C;++p){
      string tmp = c[p] + (activation[p] ? deactivate : string(""));
      l.back().push_back(tmp.c_str());
    }
  }
}


std::vector<arglist> create_combinations( ){
  DEFINE_COMPLEX; (void)C;
  
  vector<string> c=complex;
  create_args(&c[0],&c[0]+c.size());
  
  vector<string> s(simple,simple+S);
  create_args(&s[0],&s[0]+s.size());
  
  //  cout << "-- complex: --" << endl; 
  //copy(c.begin(),c.end(),ostream_iterator<string>(cout,"\n"));

  //cout << "-- simple: --" << endl; 
  //copy(s.begin(),s.end(),ostream_iterator<string>(cout,"\n"));

  std::vector<arglist> l;
  
  // create full compilation
  l.push_back(arglist());
  std::transform(s.begin(),s.end(),back_inserter(l.back()),to_q_str);
  std::transform(c.begin(),c.end(),back_inserter(l.back()),to_q_str);
  
  // create empty compilation
  l.push_back(arglist());
  std::transform(s.begin(),s.end(),back_inserter(l.back()),to_q_str);
  std::transform(c.begin(),c.end(),back_inserter(l.back()),to_q_str);
  for(arglist::iterator it=l.back().begin();it != l.back().end(); ++it){
    (*it)+=deactivate.c_str();
  }
  
  // create complex combinations with all simples
  arglist part1;
  std::transform(s.begin(),s.end(),back_inserter(part1),to_q_str);
  add_remaining_combis(c,part1,l);
  
  // create complex combinations with none of the simples
  for_each(part1.begin(),part1.end(),add_deactivation);
  add_remaining_combis(c,part1,l);

  // create each simple one on alone
  for(int i=0;i<S;++i){
    l.push_back(arglist());
    std::transform(s.begin(),s.end(),back_inserter(l.back()),to_q_str);
    std::transform(c.begin(),c.end(),back_inserter(l.back()),to_q_str);
    int j=0;
    for(arglist::iterator it=l.back().begin();it != l.back().end(); ++it,++j){
      if(i!=j){
        (*it)+=deactivate.c_str();
      }
    }
  }
  return l;
}
  





string get_make_def(const std::string &s){
  for(int i=0;i<M;++i){
    if(make_defs[i][0]==s)return make_defs[i][1];
  }
  return "";
}
string get_overwrite_def(const std::string &s){
  for(int i=0;i<O;++i){
    if(overwrite_defs[i][0]==s)return overwrite_defs[i][1];
  }
  return "";
}

string create_arg(const string &a, const string &def){
  return string("--with-")+a+"-Root="+def;
}

void create_args(string *begin, string *end){
  while(begin != end){
    string od=get_overwrite_def(*begin);
    if(od != ""){
      *begin=create_arg(*begin,od);
    }else{
      string md=get_make_def(*begin);
      if(md != ""){
        *begin=create_arg(*begin,md);
      }else{
        *begin=create_arg(*begin,def);
      }
    }
    begin++;
  }
}
