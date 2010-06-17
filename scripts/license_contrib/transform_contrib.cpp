#include <ICLQuick/Common.h>
#include <ICLIO/File.h>

int main(int n, char **ppc){
  painit(n,ppc,"-i(filename=contrib.txt) -mode(int_mode=0)");
  File file(*pa("-i"),File::readText);
  while(file.hasMoreLines()){
    std::string name = file.readLine();
    std::vector<std::string> contr = tok(file.readLine());
    ICLASSERT_RETURN_VAL(contr.size() == 5,-1);
    int ce = parse<int>(tok(contr[0],":")[1]); // celbrech
    int mg = parse<int>(tok(contr[1],":")[1]); // mgoettin
    int rh = parse<int>(tok(contr[2],":")[1]); // rhaschke
    int aj = parse<int>(tok(contr[3],":")[1]); // ajustus
    int fr = parse<int>(tok(contr[4],":")[1]); // freinhar
    int cg = parse<int>(tok(contr[5],":")[1]); // cgroszew
    int ew = parse<int>(tok(contr[6],":")[1]); // eweitnau
    int dd = parse<int>(tok(contr[7],":")[1]); // ddornbus
    
    if(pa("-mode").as<int>() == 0){
      std::cout << name << " " << ce << " " << mg 
                << " " << rh << " " << aj  << " " << fr
                << cg << " " << ew << " " << dd << std::endl;
    }else{
      int sum = ce + mg + rh + aj + fr + cg + ew + dd;
      static std::string names[8] = {
        "celbrech", "mgoettin", "rhaschke", "ajustus", "freinhar",
        "cgroszew","eweitnau","ddornbus"
      };
      int cs[8] = {ce, mg, rh, aj, fr, cg, ew, dd};
      
      std::cout << name << " ";
      if(sum >= 3){
        for(int i=0;i<8;++i){
          if(cs[i] >= 3) std::cout << names[i] << " ";
        }
      }else{
        int max = *max_element(cs,cs+8);
        for(int i=0;i<8;++i){
          if(cs[i] == max) std::cout << names[i] << " ";
        }
      }
    }
    std::cout << std::endl;
  }
}
