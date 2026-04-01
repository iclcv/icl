// SPDX-License-Identifier: LGPL-3.0-or-later
// ICL - Image Component Library (https://github.com/iclcv/icl)
// Copyright (C) 2006-2026 Carsten Schuermann, Christof Elbrechter

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLCore/Img.h>

#define ERROR_MARK  0xA000 //Top 4 Bits code on last pixel of last frame to mark that an error occurred during readout
//Connections
#define WEST 0
#define EAST 1
#define SOUTH 2
#define NORTH 3
#define NOGATE 4
#define SIDE 1
#define TOP 0
//Viewpoints
#define VIEW_W 0
#define VIEW_3 1
#define VIEW_M 2
#define VIEW_E 3


namespace icl{
  namespace io{

    class MyrmexDecoder {

    public:
      ICLIO_API MyrmexDecoder();
      ICLIO_API void decode(const icl16s *data, const utils::Size &size, core::ImgBase **dst);

    private:
      char attachedPosition; //store position of central unit
      int bigtarget[16*16]; //table which maps module orientation
      std::vector<char> conversionTable;  //table which maps usb input texel position to grabber output texel position
      std::vector<unsigned int> flat; //table which maps pixel order from per-module style to per-frame-line style
      unsigned int image_width; //store converted width
      unsigned int image_height; //store converted height

      std::vector<char> getConnectionsMeta(const icl16s *data, int size);
      int grabMetadata(const icl16s *data, unsigned char metadata[256], int size);
      int setCompression(unsigned int i);
      int setSpeed(unsigned int i);
      unsigned short swap16(unsigned short a);
      void parseConnections( std::vector<char> connections, char* attached, int* weite, int* hoehe );
      std::vector<char> makeConversiontable( int width, int height, std::vector<char> connections, char attached, char viewpoint );//generate conversiontable to match real world layout + using our viewpoint
      int convertImage(const icl16s *data, icl16s *outputImageData, const utils::Size &size);
      void init(const icl16s *data, const utils::Size &size,char viewpoint,unsigned char speed, unsigned char compression);
    };

  } // namespace io
}
