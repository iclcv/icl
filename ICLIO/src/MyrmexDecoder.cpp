/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/src/MyrmexDecoder.cpp                            **
** Module : ICLIO                                                  **
** Authors: Carsten Schürmann                                      **
**                                                                 **
** Commercial License                                              **
** ICL can be used commercially, please refer to our website       **
** www.iclcv.org for more details.                                 **
**                                                                 **
** GNU General Public License Usage                                **
** Alternatively, this file may be used under the terms of the     **
** GNU General Public License version 3.0 as published by the      **
** Free Software Foundation and appearing in the file LICENSE.GPL  **
** included in the packaging of this file.  Please review the      **
** following information to ensure the GNU General Public License  **
** version 3.0 requirements will be met:                           **
** http://www.gnu.org/copyleft/gpl.html.                           **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
*********************************************************************/

#include <ICLIO/MyrmexDecoder.h>

using namespace icl::utils;
using namespace icl::core;

namespace icl{
  namespace io{
  //Lookup table for a single module which contains the correct outputbuffer index position for a value from the linear videostream
    unsigned int correctionLUT[] = {
      3, 7, 11, 15, 67, 71, 75, 79, 131, 135, 139, 143, 195, 199, 203, 207, 
      18, 22, 26, 30, 82, 86, 90, 94, 146, 150, 154, 158, 210, 214, 218, 222, 
      2, 6, 10, 14, 66, 70, 74, 78, 130, 134, 138, 142, 194, 198, 202, 206, 
      17, 21, 25, 29, 81, 85, 89, 93, 145, 149, 153, 157, 209, 213, 217, 221, 
      19, 23, 27, 31, 83, 87, 91, 95, 147, 151, 155, 159, 211, 215, 219, 223, 
      35, 39, 43, 47, 99, 103, 107, 111, 163, 167, 171, 175, 227, 231, 235, 239, 
      51, 55, 59, 63, 115, 119, 123, 127, 179, 183, 187, 191, 243, 247, 251, 255, 
      34, 38, 42, 46, 98, 102, 106, 110, 162, 166, 170, 174, 226, 230, 234, 238, 
      50, 54, 58, 62, 114, 118, 122, 126, 178, 182, 186, 190, 242, 246, 250, 254, 
      33, 37, 41, 45, 97, 101, 105, 109, 161, 165, 169, 173, 225, 229, 233, 237, 
      48, 52, 56, 60, 112, 116, 120, 124, 176, 180, 184, 188, 240, 244, 248, 252, 
      32, 36, 40, 44, 96, 100, 104, 108, 160, 164, 168, 172, 224, 228, 232, 236, 
      16, 20, 24, 28, 80, 84, 88, 92, 144, 148, 152, 156, 208, 212, 216, 220, 
      0, 4, 8, 12, 64, 68, 72, 76, 128, 132, 136, 140, 192, 196, 200, 204, 
      1, 5, 9, 13, 65, 69, 73, 77, 129, 133, 137, 141, 193, 197, 201, 205, 
      49, 53, 57, 61, 113, 117, 121, 125, 177, 181, 185, 189, 241, 245, 249, 253}; 
  
  
    MyrmexDecoder::MyrmexDecoder(){
    }
    
    void MyrmexDecoder::decode(const icl16s *data, const Size &size, ImgBase **dst){
  
      //TODO: viewpoint/speed/compression parameter
      if(conversionTable.size()==0) init(data, size, VIEW_M, 6, 0);  //Init on first usage
  
      Size outputImageSize = Size ( image_width,  image_height);
  
      ensureCompatible(dst,depth16s,outputImageSize,1);
    
      icl16s *outputImageData = (*dst)->as16s()->begin(0);
      
      convertImage(data, outputImageData, outputImageSize);
      
    }
  
  
    int MyrmexDecoder::convertImage(const icl16s *data, icl16s *outputImageData, const Size &size){
      unsigned int j;
      unsigned short* words;
      unsigned int module_count;
      float temp;
      int error_mark=0;
      bool result=true;
  
      //access buffer wordwise
      words = (unsigned short*) data; 
  	
  
      module_count = size.getDim()/256;
  
      //copy all image data from linear videostream buffer to linear image buffer:
      // - correct byteorder because USB transmission swapped high and low byte
      // - remove top 4 Bits (they contain ad channel address)
      // - invert values so high pressure = high values (optional)
      // - correct value positions by LUT
      j=0;
      for (unsigned int k=0;k<module_count;k++){
        for (unsigned int i=0;i<256;i++){
          //correct byteorder and invert values, then save into inbuffer
          //printf("M:%d P:%d  = %X \n",k,i,words[j]);
          temp = (4095 -(0xFFF & swap16(words[j]))) ;
  			
          //Check for error frame
          if( (k==module_count-1) && (i==255)) {
            error_mark = (0xF000 & swap16(words[j]));
          }
  
          //-correct pixel position
          //conversionTable sets converted Module position offset
          //bigtarget corrects pixel orientation based on module position
          //correctionLUT corrects pixel position based on AD channel nr
  
          if (module_count>1) { //multimodule has correctionLUT applied onboard
            outputImageData[ flat[ (conversionTable[k]*256) + bigtarget[i]  ] ] = temp ;
          } else {
            outputImageData[ flat[ (conversionTable[k]*256) + bigtarget[correctionLUT[i]]  ] ] = temp ;
          }
  	
          j+=1;
        }
      }
  
  	
      if (error_mark == ERROR_MARK) result = false;
  
      return result;
    }
  
  
  
    //sets speed/compression parameters, fill conversion tables, get real image dimensions
    void MyrmexDecoder::init(const icl16s *data, const Size &size,char viewpoint, unsigned char speed, unsigned char compression){
      //Speed and compression before first capture!
  
      setSpeed(speed);
      setCompression(compression);  
  
  
      //Discover connections and setup conversion table
      std::vector<char> connections = getConnectionsMeta(data, size.getDim() ); //Get the module connection list frame metadata
  
      //Walk through the connections and determine attachment point and dimension of the module array
      int weite,hoehe;
      parseConnections( connections, &attachedPosition, &weite, &hoehe );
  
      //Create a conversion table, which reorders the image data to match the real world setup
      conversionTable = makeConversiontable( weite, hoehe, connections, attachedPosition, viewpoint );
      image_width = weite * 16; //enclosing rectangle
      image_height = hoehe * 16; //enclosing rectangle
      if (viewpoint==VIEW_3 || viewpoint==VIEW_E){
        int a = image_width;
        image_width = image_height;
        image_height = a;
  	
      }
  
  
      unsigned int p=0;
      unsigned int bcount=0;
      unsigned int ccount=0;
      unsigned int has=0;
      unsigned int hasbase=0;
      unsigned int inc =0;
      unsigned int ybase =0;
  
      //create flat table, which maps pixels from pixel-per-module to pixel-per-frame-line
      flat.resize(image_width*image_height,0);
  
      for(unsigned int x=0;x<image_width;x++){
        for(unsigned int y=0;y<image_height;y++){
          flat[ has  ] = p;
          //printf("%d : %d \n", p, has  );
          p++;
          bcount++;	
          if(p%16==0){
            bcount=0;
            if (p%(image_width)==0){
              //zurück erstes modul
              ccount=0;
              hasbase = inc+16;
              inc = inc +16;
            } else {
              //zwischen zwei modulen
              ccount++;
              hasbase = 256*ccount + inc;
            }
          }
          if (p%(image_width*16)==0){
            ybase = ybase + image_width*16;
            inc = 0;
            hasbase =0;
            bcount =0;
          }	
          has = ybase + hasbase + bcount;
  		
        }	
      }
  
    }
  
    //extract the connectiondata from the image metadata
    std::vector<char> MyrmexDecoder::getConnectionsMeta(const icl16s *data, int size){
      unsigned char metadata[256];
  
      grabMetadata(data,metadata, size);
  
      unsigned char length=(metadata[2]<<4) | metadata[3];
  
      std::vector<char> connections(length);
  
      for(unsigned char i=0; i<length;i++){
        connections[i] = metadata[i+4]; //connection data starts at entry 4
      }
  
      /*printf("Length %X\n",length);
  	printf("C1 %X\n",connections[0]);
  	printf("C2 %X\n",connections[1]);
  	printf("C3 %X\n",connections[2]);
  	printf("C4 %X\n",connections[3]);*/
      return connections;
    }
  
  
    //i=0=no compression, 1=compression with auto threshold, i>1 compression with threshold i  (i=0..255)
    int MyrmexDecoder::setCompression(unsigned int i){
  
      return 0;
  
    }
    //speed divider, higher = slower
    int MyrmexDecoder::setSpeed(unsigned int i){
  
      return 0;
    }
  
  
    //extract metadata from frame (first module)
    int MyrmexDecoder::grabMetadata(const icl16s *data, unsigned char metadata[256], int size){
      unsigned int j;
      unsigned short* words;
      unsigned int module_count;
      int error_mark=0;
      bool result=true;
  	
      words = (unsigned short*) data; 
      module_count = size/256;
  	
      j=0;
      for (unsigned int k=0;k<module_count;k++){ //look at all modules because of error frame
        for (unsigned int i=0;i<256;i++){
          //get top 4 bits metadata
          if(k==0) metadata[i] =	(0xF000 & swap16(words[j]))>>12;
  	
          //Check for error frame
          if( (k==module_count-1) && (i==255)) {
            error_mark = (0xF000 & swap16(words[j]));
          }
          j+=1;
        }
      }
  		
      //check data integrity, if false should read new frame
      if (error_mark == ERROR_MARK) result = false;	//transmission error check
      if (((metadata[0]<<4) | metadata[1]) != 0xB0)  result = false; //header check
  
      return result;
    }
  
    //swap16: Returns swapped low and high byte of value a
    unsigned short MyrmexDecoder::swap16(unsigned short a){
      unsigned char *b = (unsigned char *) &a;
      return ( *(b+1) + (*b << 8) );
    }
  
  
    //parse Connections:
    // find out at which corner the avr is attached (attached)
    // find out width & height of the module array (weite / hoehe) (rounded up if incomplete row)
    void MyrmexDecoder::parseConnections( std::vector<char> connections, char* attached, int* weite, int* hoehe ){
  #define TRUE 1
  #define FALSE 0
  
  
      char smart_dir=0;// = corner attached, 0 = upper left, 1 = lower left side, 2 = lower left bottom, 3 = lower right bottom
  
      int top_counter=0;// = max in vert richtung
      int side_counter=0;// = max in hor richtung
      int max_top =0;
      int max_side =0;
      int not_yet_side_corner = TRUE;
      int not_yet_up_corner = TRUE;
      int dir=-1;
  
      //get first connection to smart detect attachment point
      if (connections[0] == NORTH){ 
        smart_dir = (smart_dir | 2);
        not_yet_up_corner = FALSE; 
      } else if (connections[0] == WEST){
        smart_dir = (smart_dir | 0);
        not_yet_side_corner = FALSE;
      }
  
      for(unsigned int i=1;i<connections.size();i++){
  
        //get first corner direction to clarify attachment point
        if ( not_yet_side_corner == TRUE ){
          if ( connections[i] == EAST ){
            smart_dir = (smart_dir | 1);
            not_yet_side_corner = FALSE;
          } else if ( connections[i] == WEST ){
            smart_dir = (smart_dir | 0);
            not_yet_side_corner = FALSE;	
          }
        }
  
  
        if (not_yet_up_corner){
  	
          if ( connections[i] == NORTH ){
            smart_dir = (smart_dir | 1);
            not_yet_up_corner = FALSE;
          } else if ( connections[i] == SOUTH ){
            smart_dir = (smart_dir | 0);
            not_yet_up_corner = FALSE;	
          }
        }
  
        //count side and top steps to get dimensions
        //DEPENDS ON DISCOVER ALGO!!!  
  
        if (smart_dir==0){//upper left, so we dont get a 'maze'
  
  
  	if ((connections[i] == NORTH) or (connections[i] == SOUTH)){
            if (dir == TOP){ //direction in which we are moving
              top_counter++;
  
            } else { //corner?
              top_counter++;
  			
              if (side_counter > max_side) max_side = side_counter; //side finished
              side_counter = 0;
            }	
            dir = TOP;
            //printf("TOP\n");
  	}
  
  	if ((connections[i] == EAST) or (connections[i] == WEST)){
            if (dir == SIDE){
              side_counter++;
  
            } else { //corner?
              side_counter++;
              if (top_counter > max_top) max_top = top_counter; //top finished
              //top_counter=0; //because maze gives single style TOP moves
            }	
            dir = SIDE;
            //printf("SIDE\n");
  	}
  
  
  
        } else { //we get a 'maze' type discovery which gives max width/height from longest walks
  	if ((connections[i] == NORTH) or (connections[i] == SOUTH)){
            if (dir == TOP){ //direction in which we are moving
              top_counter++;
  
            } else { //corner?
              top_counter++;
  			
              if (side_counter > max_side) max_side = side_counter; //side finished
              side_counter = 0;
            }	
            dir = TOP;
            //printf("TOP\n");
  	}
  
  	if ((connections[i] == EAST) or (connections[i] == WEST)){
            if (dir == SIDE){
              side_counter++;
  
            } else { //corner?
              side_counter++;
              if (top_counter > max_top) max_top = top_counter; //top finished
              top_counter=0;
            }	
            dir = SIDE;
            //printf("SIDE\n");
  	}
  	
        }
      }
      //If we have a single row
      if (top_counter > max_top) max_top = top_counter; //top finished
      if (side_counter > max_side) max_side = side_counter; //side finished
  
      *attached = smart_dir;
      *weite = max_side+1;
      *hoehe = max_top+1;
  
  
      //printf("Top/Side: %d %d\n", max_top, max_side);
    }
  
  
    //create conversion table:
    //to place modules in right location, independent of routing alogrithm, viewpoint be given as parameter
    //
    //create target array with module locations for given view
    //follow connections and create conversion array with module locations from target array
    //startpoint for connections is determined by attachment point
    //
    // bigtarget array is to reorderthe pixels inside a module to a new rotated position
    std::vector<char> MyrmexDecoder::makeConversiontable( int width, int height, std::vector<char> connections, char attached, char viewpoint ){
  	
  
      //char actual[width][height];
  
      //conversion array
      std::vector<char> conversion(width*height);
      std::vector<char> conversion2(width*height);
  
      /*for(unsigned int i=1;i < connections.size();i++){
          conversion[i] =  -1;
  	}*/
  
      //target array
      char target[width][height];
      int bigtargetB[16][16];
  
  
      int index = 0;
  
  
      //Create target depending on viewpoint
      //module targets for W
      if (viewpoint == VIEW_W ){
        for (int y=0;y<height;y++){
          for (int x=0;x<width;x++){
            target[x][y] =  index++;
          }
        }
        //W Pixel reorder
        index =0; 
        for (int y=0;y<16;y++){
          for (int x=0;x<16;x++){		 
            bigtargetB[x][y] = index++;
          }
        }
        int findex =0;
  	
        for (int y=0;y<16;y++){
          for (int x=0;x<16;x++){	
            bigtarget[findex++] =  bigtargetB[x][y];
          }
        }
  
  
      }
      //module targets for 3
      if (viewpoint == VIEW_3 ){
        for (int x=width-1;x>=0;x--){
          for (int y=0;y<height;y++){
            target[x][y] =  index++;
          }
        }
  
  
        //3 Pixel reorder
        index =0; 
        for (int x=15;x>=0;x--){
          for (int y=0;y<16;y++){
            bigtargetB[x][y] = index++;
          }
        }
        int findex =0;
  	
        for (int y=0;y<16;y++){
          for (int x=0;x<16;x++){	
            bigtarget[findex++] =  bigtargetB[x][y];
          }
        }
      }
      //module targets for M
      if (viewpoint == VIEW_M ){	  
        for (int y=height-1;y>=0;y--){
          for (int x=width-1;x>=0;x--){
            target[x][y] =  index++;		
          }
        }
        //M Pixel reorder
        index =0; 
        for (int y=15;y>=0;y--){
          for (int x=15;x>=0;x--){	
            bigtargetB[x][y] = index++;
          }
        }
        int findex =0;
  	
        for (int y=0;y<16;y++){
          for (int x=0;x<16;x++){	
            bigtarget[findex++] =  bigtargetB[x][y];
          }
        }
      }
      //module targets for E
      if (viewpoint == VIEW_E ){
        for (int x=0;x<width;x++){
          for (int y=height-1;y>=0;y--){
            target[x][y] =  index++;
          }
        }
  	
  
   
        //E Pixel reodering
        index =0; 
        for (int x=0;x<16;x++){
          for (int y=15;y>=0;y--){
  
            bigtargetB[x][y] = index++;
          }
        }
  
  
        int findex =0;
  	
        for (int y=0;y<16;y++){
          for (int x=0;x<16;x++){	
            bigtarget[findex++] =  bigtargetB[x][y];
          }
        }
      }
  
      //start point at attachment
      char startx;
      char starty;
  
      if (attached == 0){// Upper left
        startx = 0;
        starty = 0;
      }
      if (attached == 1){//Low left side
        startx = 0;
        starty = height-1;
      }
      if (attached == 2){//Low Left Bottom
        startx = 0;
        starty = height-1;
      }
      if (attached == 3){ //Low right bottom
        startx = width-1;
        starty = height-1;
      }
      /*
          printf("Width / Height %d %d\n", width, height);
          printf("Start X/Y %d %d\n", startx, starty);
          */
      //create conversion	
      int x=startx;
      int y=starty;
      int ccounter=0;
      conversion[ccounter++] = target[x][y];
      //actual[x][y]=0; 
      //We walk through the target array using the connection information
      //and create the actual module positions and the conversion ones
      for(unsigned int i=1;i<connections.size();i++){
        if (connections[i] == NORTH ) y-=1;
        if (connections[i] == SOUTH ) y+=1;
        if (connections[i] == EAST ) x-=1;
        if (connections[i] == WEST ) x+=1;
        conversion[ccounter++] = target[x][y];
        //actual[x][y]=i;
  
      }
  
  
      return conversion;
    }
  
  
  } // namespace io
}
