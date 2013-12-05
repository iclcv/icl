#include <ICLUtils/ConfigFile.h>
#include <ICLQt/Common.h>

// a custom data type
struct MyPoint{
  MyPoint(int x=0, int y=0):x(x),y(y){}
  int x,y;
};

// overloaded ostream-operator
std::ostream &operator<<(std::ostream &s, const MyPoint &p){
  return s << '(' << p.x << ',' << p.y << ')';
}

// overloaded istream-operator (note, this sould actually
// be more robust agains parsing errors)
std::istream &operator>>(std::istream &s, MyPoint &p){
  char dummy;
  return s >> dummy >> p.x >> dummy >> p.y >> dummy;
}


int main(){
  ConfigFile cfg;

  // register type
  ConfigFile::register_type<MyPoint>("MyPoint");

  // add type instance
  cfg["config.p"] = MyPoint(1,2);

  // extract type instance
  MyPoint p = cfg["config.p"];
  
  SHOW(p);
  
  SHOW(cfg);
}
