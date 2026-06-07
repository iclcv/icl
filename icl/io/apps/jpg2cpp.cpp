// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Christof Elbrechter

#include "stdio.h"
#include <string>
#include <vector>
#include <cstring>
#include <filesystem>
int usage(){
  printf("jpg2cpp converts jpeg images into a c++ compatible data format.\n"
         "usage: jpg2cpp <FILE> > <DESTINATIONFILE> \n"
         "  <FILE> is the input jpeg (a path prefix like ./foo or ../bar is\n"
         "  fine — the C++ identifier is derived from the filename stem).\n"
         "  <DESTINATIONFILE> is something like \"ICL/ICLIO/src/demoImageMyImage.cpp\"\n"
         "  The destination file is created in 4 blocks:\n"
         "     - main data block containing most of the binary data of the image\n"
         "     - extra data block containing some additional bytes for the image\n"
         "     - the createImage_xxx() -function block. This is the implementation of\n"
         "       a TU-local (static) function that converts the data- and extraData-\n"
         "       block into a core::Image.\n"
         "     - the REGISTER_TEST_IMAGE(...) line that wires the factory into\n"
         "       testImageRegistry() so TestImages::create(\"xxx\") finds it.  The\n"
         "       function is intentionally NOT declared in any header — users call\n"
         "       TestImages::create(name, ...) instead.\n\n" );

  return 1;
}
int fileNotFound(char *pc){
  printf("file %s not found\n",pc); return 1;
}



void writeLine(std::vector<unsigned char>::iterator &it, int len, bool isLastRow){
  printf("  {");

  std::vector<unsigned char>::iterator end = it+len-1;
  while(it != end){
    printf("%3d,",*it++);
  }
  if(!isLastRow){
    printf("%3d},\n",*it++);
  }else{
    printf("%3d}\n};\n",*it++);
  }
}


int main(int n, char **ppc){


  if(n!=2) return usage();

  FILE *f = fopen(ppc[1],"rb");
  if(!f) return fileNotFound(ppc[1]);

  char buf[10000];
  std::vector<unsigned char> dataVec;
  int nBytesRead = 0;
  while(!feof(f)){
    int read = fread(buf,1,10000,f);
    nBytesRead += read;
    for(int i=0;i<read;++i){
      dataVec.push_back((unsigned char)buf[i]);
    }
  }
  fclose(f);
  //-------------------------------------------
  // Use just the filename stem as the C++ identifier base — paths and
  // extensions in the input filename would produce invalid identifiers
  // (slashes, dots) when concatenated into array names.
  std::string imageName = std::filesystem::path(ppc[1]).stem().string();
  std::string arrayName = std::string("aauc_Data_")+imageName;
  std::string extraArrayName = std::string("auc_ExtraData_")+imageName;

  printf("#include <icl/io/detail/file-plugins/JPEGDecoder.h>\n");
  printf("#include <icl/core/Image.h>\n");
  printf("#include <icl/core/Img.h>\n");
  printf("#include <icl/io/source/TestImages.h>\n");
  printf("#include <vector>\n");


  //-------------------------------------------
  printf("using namespace icl::utils;\n");
  printf("using namespace icl::core;\n");
  printf("namespace icl::io {\n");
  printf("namespace{\n");
  const int COLS = 30;
  const int ROWS = nBytesRead/COLS;
  const int NEXTRA = nBytesRead - COLS*ROWS;
  printf("const int NROWS = %d;\n",ROWS);
  printf("const int NCOLS = %d;\n",COLS);
  printf("const int NEXTRA = %d;\n",NEXTRA);

  printf("unsigned char %s[NROWS][NCOLS] = {\n",arrayName.c_str());
  printf("  // {{{ open\n");
  std::vector<unsigned char>::iterator it=dataVec.begin();
  for(int l=0;l<ROWS;l++){
    writeLine(it,COLS,l==ROWS-1);
  }
  printf("// }}}\n");
  printf("unsigned char %s[NEXTRA] = {\n",extraArrayName.c_str());
  printf("  // {{{ open\n");
  for(int i=0;i<NEXTRA;++i,++it){
    printf("%3d",*it);
    if(i!=(NEXTRA-1)){
      printf(",");
    }
  }
  printf("\n};\n// }}}\n\n}//end namespace\n");

  printf("static core::Image createImage_%s(){\n",imageName.c_str());
  printf("  // {{{ open\n");
  printf("  static core::Image cached;\n"
         "  if(!cached.isNull()) return cached.deepCopy();\n"
         "  const int DIM = NROWS*NCOLS+NEXTRA;\n"
         "  std::vector<unsigned char> buf(DIM);\n"
         "  int j=0;\n"
         "  for(int i=0;i<NROWS;++i){\n"
         "    for(int k=0;k<NCOLS;k++,j++){\n"
         "      buf[j] = %s[i][k];\n"
         "    }\n"
         "  }\n"
         "  for(int i=0;i<NEXTRA;i++,j++){\n"
         "    buf[j] = %s[i];\n"
         "  }\n"
         "  core::ImgBase *raw = 0;\n"
         "  JPEGDecoder::decode(buf.data(), DIM, &raw);\n"
         "  cached = core::Image(raw);\n"
         "  return cached.deepCopy();\n"
         "}\n// }}}\n\n",arrayName.c_str(),extraArrayName.c_str());

  printf("  REGISTER_TEST_IMAGE(%s, createImage_%s)\n",imageName.c_str(),imageName.c_str());
  printf("  } // namespace icl::io\n\n\n");

}
