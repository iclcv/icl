/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/PCDFileGrabber.cpp                 **
** Module : ICLGeom                                                **
** Authors: Patrick Nobou                                          **
**                                                                 **
**                                                                 **
** GNU LESSER GENERAL PUBLIC LICENSE                               **
** This file may be used under the terms of the GNU Lesser General **
** Public License version 3.0 as published by the                  **
**                                                                 **
** Free Software Foundation and appearing in the file LICENSE.LGPL **
** included in the packaging of this file.  Please review the      **
** following information to ensure the license requirements will   **
** be met: http://www.gnu.org/licenses/lgpl-3.0.txt                **
**                                                                 **
** The development of this software was supported by the           **
** Excellence Cluster EXC 277 Cognitive Interaction Technology.    **
** The Excellence Cluster EXC 277 is a grant of the Deutsche       **
** Forschungsgemeinschaft (DFG) in the context of the German       **
** Excellence Initiative.                                          **
**                                                                 **
********************************************************************/

#include <ICLGeom/PCDFileGrabber.h>
//#include <ICLIO/FileGrabber.h>
#include <ICLIO/FileList.h>
#include <ICLUtils/File.h>
#include <ICLUtils/PluginRegister.h>

#include <bitset>
#include <fstream>


namespace icl{

  using namespace utils;
  using namespace math;
  using namespace io;
  using namespace core;

  namespace geom{
    namespace{
      struct RGBFloat{
        icl8u r,g,b,a;
      };

      template<int N>
      inline FixedColVector<float,N> create_xyz_vec(float x, float y, float z){
        return FixedColVector<float,N>(x,y,z);
      }
      template<>
      inline FixedColVector<float,4> create_xyz_vec(float x, float y, float z){
        return FixedColVector<float,4>(x,y,z,1);
      }
      template<class T>
      bool test_type(const std::string &type, int size){
        return false;
      }
      template<>
      bool test_type<double>(const std::string &type, int size){
        return type == "F" && size==8;
      }

      template<>
      bool test_type<float>(const std::string &type, int size){
        return type == "F" && size==4;
      }

      template<>
      bool test_type<int8_t>(const std::string &type, int size){
        return type == "I" && size==1;
      }

      template<>
      bool test_type<int16_t>(const std::string &type, int size){
        return type == "I" && size==2;
      }

      template<>
      bool test_type<int32_t>(const std::string &type, int size){
        return type == "I" && size==4;
      }

      template<>
      bool test_type<uint8_t>(const std::string &type, int size){
        return type == "U" && size==1;
      }
      template<>
      bool test_type<uint16_t>(const std::string &type, int size){
        return type == "U" && size==2;
      }
      template<>
      bool test_type<uint32_t>(const std::string &type, int size){
        return type == "U" && size==4;
      }


    }

    struct PCDFileGrabber::Data{
      FileList flist;
      bool loop;
      int nextFile;

      static inline std::string trim(const std::string &t){
        size_t start = 0;
        size_t end = 0;
        for(;start<t.size() && isspace(t[start]); ++start);
        for(end=t.size()-1;end>start && isspace(t[end]);--end);
        return t.substr(start, end - start+1);
      }

      struct LineParser{

        struct Parser{
          virtual void parse(std::istream &str) = 0;
        };

        template<class T>
        struct TParser : public Parser{
          int n;
          T *dst;
          TParser(int n, std::vector<icl8u> &dstV, int dim):n(n){
            dstV.resize(dim*sizeof(T));
            dst = (T*)dstV.data();
          }
          virtual void parse(std::istream &str){
            for(int i=0;i<n;++i){
              str >> *dst++;
            }
          }
        };

        std::vector<Parser*> ps;
        std::vector<std::vector<icl8u> > data;
        std::vector<std::string> fields;

        struct FieldDef{
          int dataIdx;
          std::string name;
          std::string type;
          int size;
          int count;
        };

        std::map<std::string,FieldDef> m_typeLUT;

        LineParser(const std::string &fieldsS,
                   const std::string &sizesS,
                   const std::string &typesS,
                   const std::string &countsS,
                   int dim){
          this->fields = tok(trim(fieldsS)," ");

          std::vector<int> sizes = parseVecStr<int>(trim(sizesS)," ");
          std::vector<std::string> types = tok(trim(typesS)," ");
          std::vector<int> counts = parseVecStr<int>(trim(countsS)," ");

          size_t nFields = fields.size();
          if(nFields != sizes.size() || nFields != types.size() || nFields != counts.size()){
            throw ICLException("invalid PCD file format: expected fields, "
                               "sizes, types and counts to have the same number of tokens");
          }

          if(data.size() != nFields){
            data.resize(nFields);
          }

          for(size_t i=0;i<nFields;++i){
            int s = sizes[i], n = counts[i];
            FieldDef def = {(int)i, this->fields[i], types[i], s, n};
            m_typeLUT[this->fields[i]]= def;
            switch(types[i].at(0)){
              case 'I':
                if(s == 1) ps.push_back(new TParser<int8_t>(n,data[i],dim));
                else if(s == 2) ps.push_back(new TParser<int16_t>(n,data[i],dim));
                else if(s == 4) ps.push_back(new TParser<int32_t>(n,data[i],dim));
                else throw ICLException("Invalid PCD file formation: type 'I' can only have 1,2 or 4 bytes (found: " + str(s) + ")");
                break;
              case 'U':
                if(s == 1) ps.push_back(new TParser<uint8_t>(n,data[i],dim));
                else if(s == 2) ps.push_back(new TParser<uint16_t>(n,data[i],dim));
                else if(s == 4) ps.push_back(new TParser<uint32_t>(n,data[i],dim));
                else throw ICLException("Invalid PCD file formation: type 'U' can only have 1,2 or 4 bytes (found: " + str(s) + ")");
                break;
              case 'F':
                if(s == 4) ps.push_back(new TParser<float>(n,data[i],dim));
                else if(s == 8) ps.push_back(new TParser<double>(n,data[i],dim));
                else throw ICLException("Invalid PCD file formation: type 'F' can only have 4 or 8 bytes (found: " + str(s) + ")");
                break;
              default: break;
            }
          }
        }

        void parseData(std::istream &str, int n){
          for(int i=0;i<n;++i){
            for(size_t i=0;i<ps.size();++i){
              ps[i]->parse(str);
            }
          }
        }

        const FieldDef &findDef(const std::string &name) {
          std::map<std::string,FieldDef>::const_iterator it = m_typeLUT.find(name);
          if(it == m_typeLUT.end()){
            throw ICLException("PCDFileGrabber: feature type " + name + " not found");
          }
          return it->second;
        }

        template<class T>
        const T *find(const std::string &name, int count=1){
          const FieldDef &def = findDef(name);
          if(!test_type<T>(def.type, def.size)){
            throw ICLException("PCDFileGrabber: invalid type for feature " + name);
          }
          if(def.count != count){
            throw ICLException("PCDFileGrabber: invalid count for feature " + name);
          }
          return reinterpret_cast<const T*>( data[def.dataIdx].data());
        }

        template<int N>
        void copy_xyz(DataSegment<float,N> xyz, int dim){
          const float *px = find<float>("x");
          const float *py = find<float>("y");
          const float *pz = find<float>("z");
          for(int i=0;i<dim;++i){
            xyz[i] = create_xyz_vec<N>(px[i],py[i],pz[i]);
          }
        }

        template<int R, int G, int B, int A, class T, int N>
        void copy_rgb(const RGBFloat *src, DataSegment<T,N> dst, int dim){
          for(int i=0;i<dim;++i){
            FixedColVector<T,N> &d = dst[i];
            T s = src[i].a;
            d[A] = s; //src[i].a;
            d[R] = src[i].r;
            d[G] = src[i].g;
            d[B] = src[i].b;
          }
        }

        void copyData(PointCloudObjectBase &dst, const Size &size, bool extend){
          if(size.height > 1){
            dst.setSize(size);
          }else{
            dst.setDim(size.width);
          }
          const int dim = dst.getDim();

          // x y z are combined
          if(dst.supports(PointCloudObjectBase::XYZH)){
            copy_xyz(dst.selectXYZH(),dim);
          }else if(dst.supports(PointCloudObjectBase::XYZ)){
            copy_xyz(dst.selectXYZ(),dim);
          }else if(extend && dst.canAddFeature(PointCloudObjectBase::XYZH)){
            dst.addFeature(PointCloudObjectBase::XYZH);
            copy_xyz(dst.selectXYZH(),dim);
          }else if(extend && dst.canAddFeature(PointCloudObjectBase::XYZ)){
            dst.addFeature(PointCloudObjectBase::XYZ);
            copy_xyz(dst.selectXYZ(),dim);
          }else{
            throw ICLException("PCDFileGrabber: destination point cloud neither accepts any "
                               "x/y/z feature not allows to add one");
          }

          try{

            const RGBFloat * rgb = reinterpret_cast<const RGBFloat*>(find<float>("rgb"));

            if(dst.supports(PointCloudObjectBase::BGR)){
              copy_rgb<2,1,0,0,icl8u,3>(rgb, dst.selectBGR(), dim);
            }else if(dst.supports(PointCloudObjectBase::BGRA)){
              copy_rgb<2,1,0,3,icl8u,4>(rgb, dst.selectBGRA(), dim);
            }else if(dst.supports(PointCloudObjectBase::BGRA32s)){
              // should not happen, it should also support BGRA in this case!
            }else if(dst.supports(PointCloudObjectBase::RGBA32f)){
              copy_rgb<0,1,2,3,float,4>(rgb, dst.selectRGBA32f(), dim);
            }else if(extend && dst.canAddFeature(PointCloudObjectBase::BGR)){
              dst.addFeature(PointCloudObjectBase::BGR);
              copy_rgb<2,1,0,0,icl8u,3>(rgb, dst.selectBGR(), dim);
            }else if(extend && dst.canAddFeature(PointCloudObjectBase::BGRA)){
              dst.addFeature(PointCloudObjectBase::BGRA);
              copy_rgb<2,1,0,3,icl8u,4>(rgb, dst.selectBGRA(), dim);
            }else if(extend && dst.canAddFeature(PointCloudObjectBase::BGRA32s)){
              dst.addFeature(PointCloudObjectBase::BGRA32s);
              // hmmm
            }else if(extend && dst.canAddFeature(PointCloudObjectBase::RGBA32f)){
              dst.addFeature(PointCloudObjectBase::RGBA32f);
              copy_rgb<0,1,2,3,float,4>(rgb, dst.selectRGBA32f(), dim);
            }

          }catch(ICLException &){
          }

        }
      };
    };



    PCDFileGrabber::PCDFileGrabber(const std::string &filepattern, bool loop):
      m_data(new Data){
      m_data->flist = FileList(filepattern);
      m_data->loop = loop;
      if(!m_data->flist.size()){
        delete m_data;
        throw ICLException("PCDFileGrabber: empty file list (pattern was: " + filepattern + ")");
      }
      m_data->nextFile = 0;
    }


    PCDFileGrabber::~PCDFileGrabber(){
      delete m_data;
    }


    void PCDFileGrabber::grab(PointCloudObjectBase &dst){
      if(m_data->nextFile >= m_data->flist.size()){
        if(m_data->loop) m_data->nextFile = 0;
        else throw ICLException("PCDFileGrabber::grab: no more files (looping was diabled)");
      }
      int idx = m_data->nextFile++;
      std::ifstream str(m_data->flist[idx].c_str());
      std::string name, version, fields, sizes, types, counts, width, height, viewpoint,points;
      std::string *s[9] = {&version, &fields, &sizes, &types, &counts, &width, &height, &viewpoint, &points};
      for(int i=0;i<9;++i){
        char c;
        str.get(c);
        if(c == '#'){
          std::getline(str,name); --i;
          continue;
        }else{
          str.unget();
        }
        str >> name;
        std::getline(str,*s[i]);
      }
      Size ds(utils::parse<int>(Data::trim(width)),
              utils::parse<int>(Data::trim(height)));

      Data::LineParser parser(fields, sizes, types, counts, ds.getDim());
      parser.parseData(str,parse<int>(Data::trim(points)));
      parser.copyData(dst,ds,true);


      DataSegment<float,4> aa = dst.selectXYZH();
      DataSegment<float,4> bb = dst.selectRGBA32f();

      SHOW(aa.getSize());

      for(int i=0;i<10;++i){
        SHOW(aa[i].transp());
        SHOW(bb[i].transp());
      }
    }





    static PointCloudGrabber *create_pcd_file_grabber(const std::map<std::string,std::string> &d){
      std::map<std::string,std::string>::const_iterator it = d.find("creation-string");
      if(it == d.end()) return 0;
      const std::string &params = it->second;

      std::vector<std::string> ts = tok(params,"@");
      if(ts.size() == 1){
        return new PCDFileGrabber(ts[0], true);
      }else if(ts.size() > 1){
        return new PCDFileGrabber(ts[0], ts[1]!="loop=off");
      }else {
        return 0;
      }

    }

    REGISTER_PLUGIN(PointCloudGrabber,pcd,create_pcd_file_grabber,
                    "Point cloud grabber using an input patter for grabbing pcd-files",
                    "creation-string: filepattern[@loop=off]");







  }
}
