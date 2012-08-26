#include <ICLQuick/Common.h>
#include <ICLIO/FileList.h>
#include <ICLIO/File.h>

#include <fstream>

std::map<std::string,std::string> fileToPackage;
std::map<std::string,std::string> packageToFile;

std::vector<std::string> headers;
std::vector<std::string> sources;
std::vector<std::string> examples;
std::vector<std::string> all;

int main(){
  std::vector<std::string> packages = tok("ICLCore ICLCV ICLFilter ICLGeom ICLIO ICLMarkers ICLMath ICLQt ICLUtils"," ");
  int N = packages.size();
  std::cout << "analyzing ..." << std::endl;
  fileToPackage["RSBImage.pb.h"] = "ICLIO";
  packageToFile["ICLIO"] = "RSBImage.pb.h";
  
  for(int i=0;i<N;++i){
    std::string p = packages[i];
    std::cout << "package:" << p << std::endl;
    {
      FileList fl("/home/celbrech/projects/ICL/include/"+p+"/*.h");
      for(int j=0;j<fl.size();++j){
        std::string f = fl[j];
        headers.push_back(f);
        all.push_back(f);
        File ff(f);
        f = ff.getBaseName() + ff.getSuffix();
        fileToPackage[f] = p;
        packageToFile[p] = f;
      }
    }

    {
      FileList fl("/home/celbrech/projects/ICL/"+p+"/src/*.cpp");
      for(int j=0;j<fl.size();++j){
        std::string f = fl[j];
        sources.push_back(f);
        all.push_back(f);
      }
    }

    {
      FileList fl("/home/celbrech/projects/ICL/"+p+"/examples/*.cpp");
      for(int j=0;j<fl.size();++j){
        std::string f = fl[j];
        examples.push_back(f);
        all.push_back(f);
      }
    }
  } // end analysis ...

  int M = all.size();
  for(int i=0;i<M;++i){
    std::string fn = all[i];
    //    File f2(fn+".tmp",File::writeText);
    File f2("test.tmp",File::writeText);

    std::ifstream fstr(fn.c_str());
    string l;
    while(std::getline(fstr,l)){
      if(l.length() > 13 && l.substr(0,13) == "#include <ICL"){
        std::cout << l << std::endl;
        MatchResult r = match(l,"#include <ICL([^/]*)/([^.]*)\\.h>",3);
        std::cout << "  " << "--" << r.submatches[1] << "--" <<  r.submatches[2] << std::endl;
        std::string ifile = r.submatches[2]+".h";
        std::string targetPackage = fileToPackage[ifile];
        std::string curretPackage = "ICL"+r.submatches[1];
        if(targetPackage != curretPackage){
          std::string newLine = "#include <"+targetPackage+"/"+ifile+">";
          std::cout << "found: " << l << " replacing by: " << newLine << std::endl;
          f2.write(newLine + "\n");
        }else{
          f2.write(l + "\n");
        }
      }else{
        f2.write(l + "\n");
      }
      //f2.writeLine(l);
    }
  }

}
