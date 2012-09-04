#include <ICLQuick/Common.h>
#include <ICLIO/FileList.h>
#include <ICLIO/File.h>

#include <fstream>

std::map<std::string,std::string> fileToPackage;
std::map<std::string,std::string> packageToNameSpace;
std::map<std::string,std::string> packageToFile;
std::map<std::string,std::string> fileNameToURI;

std::map<std::string,std::string> classToNameSpace;

std::vector<std::string> headers;
std::vector<std::string> sources;
std::vector<std::string> examples;
std::vector<std::string> all;

std::vector<std::string> read_lines(const std::string &filename){
  std::ifstream stream(filename.c_str());
  std::string line;
  std::vector<std::string> lines;
  while(std::getline(stream,line)){
    lines.push_back(line);
  }
  return lines;
}

void write_lines(const std::string &filename, const std::vector<std::string> &lines){
  std::ofstream stream(filename.c_str());
  for(size_t i=0;i<lines.size();++i){
    stream << lines[i] << std::endl;
  }
}

std::map<std::string,int> level;

int main(){
  std::vector<std::string> packages = tok("ICLCore ICLCV ICLFilter ICLGeom ICLIO ICLMarkers ICLMath ICLQt ICLUtils"," ");
  level["ICLUtils"] = level["utils"] = 0;
  level["ICLMath"] = level["math"] = 1;
  level["ICLCore"] = level["core"] = 2;
  level["ICLFilter"] = level["filter"] = 3;
  level["ICLIO"] = level["io"] = 4;
  level["ICLQt"] = level["qt"] = 5;
  level["ICLGeom"] = level["geom"] = 6;
  level["ICLCV"] = level["cv"] = 7;
  level["ICLMarkers"] = level["markers"] = 8;

  int N = packages.size();
  std::cout << "analyzing ..." << std::endl;
  fileToPackage["RSBImage.pb.h"] = "ICLIO";
  packageToFile["ICLIO"] = "RSBImage.pb.h";
  packageToNameSpace["ICLCore"] = "core";
  packageToNameSpace["ICLCV"] = "cv";
  packageToNameSpace["ICLFilter"] = "filter";
  packageToNameSpace["ICLGeom"] = "geom";
  packageToNameSpace["ICLIO"] = "io";
  packageToNameSpace["ICLMarkers"] = "markers";
  packageToNameSpace["ICLMath"] = "math";
  packageToNameSpace["ICLQt"] = "qt";
  packageToNameSpace["ICLUtils"] = "utils";

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
        fileNameToURI[f] = fl[j];
      }
    }

    {
      FileList fl("/home/celbrech/projects/ICL/"+p+"/src/*.cpp");
      for(int j=0;j<fl.size();++j){
        std::string f = fl[j];
        sources.push_back(f);
        all.push_back(f);

        File ff(f);
        f = ff.getBaseName() + ff.getSuffix();
        fileToPackage[f] = p;
        packageToFile[p] = f;
        fileNameToURI[f] = fl[j];
      }
    }

    {
      FileList fl("/home/celbrech/projects/ICL/"+p+"/examples/*.cpp");
      for(int j=0;j<fl.size();++j){
        std::string f = fl[j];
        examples.push_back(f);
        all.push_back(f);

        File ff(f);
        f = ff.getBaseName() + ff.getSuffix();
        fileToPackage[f] = p;
        packageToFile[p] = f;
        fileNameToURI[f] = fl[j];
      }
    }
  } // end analysis ...


  //// ADDING NAMESPACES
  std::vector<std::string> headersAndSources = headers;
  std::copy(sources.begin(),sources.end(),std::back_inserter(headersAndSources));
  

  for(size_t i=0;i<headersAndSources.size();++i){
    std::string fh = headersAndSources[i];
    File ff(fh);
    std::string f = ff.getBaseName() + ff.getSuffix();
    //if(!match(fh,".*/Img.cpp$")) continue; // first, we'll test here!
    std::cout << "processing file: " << fh << std::endl; 
    
    std::string p = fileToPackage[f];
    std::vector<std::string> lines= read_lines(fh);

    bool inNamespace = false;
    for(size_t j=0;j<lines.size();++j){
      std::string &l = lines[j];
      if(inNamespace){
        if(match(l,"^}[^;]*")){
          std::cout << "}-found at ##" << l << "# (line:" << j << ")" <<  std::endl;
          lines.insert(lines.begin()+j,"  } // namespace " + packageToNameSpace[p]);
          inNamespace = false;
        }else{
          l  = "  " + l;
        }
      }else{
        if(match(l,"^namespace icl[ ]*\\{.*")){
          std::cout << "{-found at ##" << l << "# (line:" << j << ")" <<  std::endl;
          lines.insert(lines.begin()+j+1,"  namespace " + packageToNameSpace[p] + "{");
          j++;
          inNamespace = true;
        }
      }
    }
    write_lines(fh,lines);
  }  
  
  return 0;

  std::ofstream out("./add-namespaces.sed");
  out << "#!/bin/sed -f" << std::endl;
  out << "s|class |class__|g" << std::endl;
  out << "s|struct |struct__|g" << std::endl;
  
  out << std::endl;
  /// Create classToPackage map
  for(size_t i=0;i<headers.size();++i){
    std::string fh = headers[i];
    File ff(fh);
    std::string f = ff.getBaseName() + ff.getSuffix();
    std::string p = fileToPackage[f];
    
    //    if(p != "ICLCore") continue;
    std::string n = packageToNameSpace[p];

    //std::cout << p <<  ": processing file " << f << std::endl;
    //if(!match(fh,".*/Img.h$")) continue; // first, we'll test here!

    
    std::vector<std::string> lines = read_lines(fh);
    for(size_t j=0;j<lines.size();++j){
      MatchResult r = match(lines[j],"^ *class ([a-zA-Z0-9_]+)(.*)",3);
      if(!r){

      }else{
        std::string m1 = r.submatches[1];
        std::string m2 = r.submatches[2];
        if( m1[0] != ',' && m1[0] != '>'){
          if(!m2.length() || m2 == "{" || m2 == " {"|| match(m2," *: *public")){
            //std::cout << n <<"::" << m1 << " in ##" << lines[j] << "##" << std::endl;
            if(m1 != "Foo"){
              classToNameSpace[m1] = n;
              std::cout << n << "::" << m1 << std::endl;
              std::string a = "\\([^a-zA-Z0-9_]\\)" + m1 + "\\([^a-zA-Z0-9_]\\)";
              std::string b = "\\1"+n+"::"+m1+"\\2";
              out << "s|" << a << "|" << b<< "|g"<< std::endl;
            }
          }
        }
      }
    }
  }

  out << "s|struct__|struct |g" << std::endl;
  out << "s|class__|class |g" << std::endl;

  out.close();
  system("chmod +x ./add-namespaces.sed");
  
  // todo:
  // 1.) add namespaces to headers and sources:
  // * replace "namespace icl{" by "namspace icl{ \n namespace core{"
  // * find corresponding closing brace and and add add extra closing brace
  // * for each line in between, prepend two spaces

  // 1.) for the headers, add namespace prefixes (e.g. core::) for every used class
  // * create class->package map
  // * find classnames text-parts and prepend package namespace (not for the current package)
  
  // 2.) for source and example files: add using directives for all neccessary namespaces
  // * Example: using namespace icl::core; 
  // * again, find class name expressions;
  // * memorize used classes and their packages;
  // * add using directives after last #include statement
  
}


