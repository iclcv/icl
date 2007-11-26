#include "stdio.h"
#include <string>
#include <vector>


using namespace std;

int usage(){
  printf("jpg2cpp converts jpeg images into a c++ compatible data format.\n"
         "usage: jpg2cpp <FILE> > <DESTINATIONFILE> \n"
         "  <FILE> must be something like myImage.jpg without any \"./foo\"-\n "
         "  or \"../../foo/bar/\"-prefix.\n"
         "  <DESTINATIONFILE> is something like \"ICL/ICLIO/src/demoImageMyImage.cpp\"\n"
         "  The destination file is created in 4 blocks:\n"
         "     - main data block containing most of the binary data of the image\n"
         "     - extra data block containing some additional bytes for the image\n"
         "     - the createImage_xxx() -function block. This is the implementation of\n"
         "       a function, that converts the data- and the extraData-block into an\n"
         "       appropriate ImgBase object. The image is created by using a temporary\n"
         "       jpg file on the hard-disk, which is read using a FileReader object.\n"
         "     - the header information block containing the function declaration for\n"
         "       the according header file\n\n" );
  
  return 1;
}
int fileNotFound(char *pc){
  printf("file %s not found\n",pc); return 1;
}



void writeLine(vector<unsigned char>::iterator &it, int len, bool isLastRow){
  printf("  {");
  
  vector<unsigned char>::iterator end = it+len-1;
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
  vector<unsigned char> dataVec;
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
  string imageName = string(ppc[1]).substr(0,strlen(ppc[1])-4);
  string arrayName = string("aauc_Data_")+imageName;
  string extraArrayName = string("auc_ExtraData_")+imageName;

  printf("#include \"ICLFileReader.h\"\n");
  printf("#include \"ICLImg.h\"\n");
  

  //-------------------------------------------
  printf("using namespace icl;\n");
  printf("using namespace std;\n");
  printf("namespace icl{\n");
  printf("namespace{\n");
  const int COLS = 30;
  const int ROWS = nBytesRead/COLS;
  const int NEXTRA = nBytesRead - COLS*ROWS;
  printf("const int NROWS = %d;\n",ROWS);
  printf("const int NCOLS = %d;\n",COLS);
  printf("const int NEXTRA = %d;\n",NEXTRA);
  
  printf("unsigned char %s[NROWS][NCOLS] = {\n",arrayName.c_str());
  printf("  // {{{ open\n");
  vector<unsigned char>::iterator it=dataVec.begin();
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
  
  printf("ImgBase* createImage_%s(){\n",imageName.c_str());
  printf("  // {{{ open\n");
  printf("  static ImgBase *image = 0;\n"
         "  if(image) return image->deepCopy();\n"
         "  FILE *f = fopen(\"./.tmp_image_buffer.jpg\",\"wb\");\n"
         "  const int DIM = NROWS*NCOLS+NEXTRA;\n"
         "  char *buf= new char[DIM];\n"
         "  int j=0;\n"
         "  for(int i=0;i<NROWS;++i){\n"
         "     for(int k=0;k<NCOLS;k++,j++){\n"
         "        buf[j] = %s[i][k];\n"
         "     }\n"
         "  }\n"
         "  for(int i=0;i<NEXTRA;i++,j++){\n"
         "     buf[j] = %s[i];\n"
         "  }\n"
         "  fwrite(buf,1,DIM,f);\n"
         "  fclose(f);\n"
         "  delete [] buf;\n"
         "  image = FileReader(\"./.tmp_image_buffer.jpg\").grab()->deepCopy();\n"
         "  remove(\"./.tmp_image_buffer.jpg\");\n"
         "  return image->deepCopy();\n"
         "}\n// }}}\n\n",arrayName.c_str(),extraArrayName.c_str());

  printf("} // end namespace icl\n\n\n");
  
  printf("!!! put this into the header file:\n"
         "namespace icl{\n"
         "  /// Create the image named %s\n"
         "  ImgBase *createImage_%s();\n"
         "}\n\n",imageName.c_str(),imageName.c_str());
        
}


