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
#include <ICLIO/FileGrabber.h>
#include <ICLGeom/PCLPointCloudObject.h>

#include <bitset>
#include <sstream>


namespace icl{

  using namespace utils;
  using namespace math;
  using namespace io;

  namespace geom{
    static bool has_fields(const std::vector<std::string> &v, const char *a,
                           const char *b=0, const char *c=0, const char *d=0,
                           const char *e=0, const char *f=0){
      if(std::find(v.begin(),v.end(), std::string(a)) == v.end()) return false;
      if(b && std::find(v.begin(),v.end(), std::string(b)) == v.end()) return false;
      if(c && std::find(v.begin(),v.end(), std::string(c)) == v.end()) return false;
      if(d && std::find(v.begin(),v.end(), std::string(d)) == v.end()) return false;
      if(e && std::find(v.begin(),v.end(), std::string(e)) == v.end()) return false;
      if(f && std::find(v.begin(),v.end(), std::string(f)) == v.end()) return false;
      return (!!a + !!b + !!c + !!d + !!e + !!f) == (int)v.size();
    }
    
    template<class T> static int pcl_type_id() { return -1; }
#define ID(T,i) template<> int pcl_type_id<pcl::T>() { return i; }
    ID(PointXYZ,0)
    ID(PointXYZI,1)
    ID(PointXYZL,2)
    ID(PointXYZRGB,3)
    ID(PointXYZRGBA,4)
    ID(PointXYZRGBL,5)
    ID(InterestPoint,6)
    ID(PointXYZRGBNormal,7)
    ID(PointXYZINormal,8)
#undef ID

    struct PCDFileGrabber::Data{
      std::string properties;  //!< internal variable
      std::string value;       //!< internal variable
      std::string pattern;     //!< file pattern
      bool loop;               //!< whether to play PCD file in an endless loop or not.
      bool forceExactPCLType;  //!< Flag to specify the use of wellknow point type
      FileGrabber *f;          //!< internally used for filename generation
      int counter;             //!< current file index counter
      PointCloudObjectBase *buffers[8]; //!< set of buffers

      Data(){
        std::fill(buffers,buffers+8,(PointCloudObjectBase *)0);
      }
      
      ~Data(){
        for(int i=0;i<8;++i) ICL_DELETE(buffers[i]);
        ICL_DELETE(f);
      }

      template<class T>
      PCLPointCloudObject<T> &cast(PointCloudObjectBase &o){
        try{
          return dynamic_cast<PCLPointCloudObject<T>&>(o);
        }catch(...){
          if(forceExactPCLType){
            throw ICLException("PCLFileGrabber::grab: wrong destination object type (try to switch forceExactType off)");
          }else{
            if(!buffers[pcl_type_id<T>()]) buffers[pcl_type_id<T>()] = new PCLPointCloudObject<T>;
            return *reinterpret_cast<PCLPointCloudObject<T>*>(buffers[pcl_type_id<T>()]);
          }
        }
        // satisfies the compiler, but never happens
        return reinterpret_cast<PCLPointCloudObject<T>&>(o);
      }
        
    };

    PCDFileGrabber::PCDFileGrabber(void):m_data(0){}


    PCDFileGrabber::PCDFileGrabber(const std::string &pattern, bool loop, bool forceExactPCLType, const int offset):
      m_data(new Data){
      if(!pattern.length()){
        throw ICLException("PCDFileGrabber::PCDFileGrabber: Wrong file/pattern name!");
      }
      m_data->pattern = pattern;
      m_data->properties = "next";
      m_data->value = "";
      m_data->loop = loop;
      m_data->forceExactPCLType = forceExactPCLType;
      m_data->f = new FileGrabber(pattern);
      m_data->counter = 0;
    }

    PCDFileGrabber::~PCDFileGrabber() throw(){
      ICL_DELETE(m_data);
    }

    std::vector<std::string> PCDFileGrabber::getFields() const{
      std::vector<std::string> result;
      pcl::PCDReader p;

      sensor_msgs::PointCloud2 h;
      Eigen::Vector4f orig;
      Eigen::Quaternionf angles;
      int version(0),datatype(0),offset(0);
      unsigned int dataIdx(0);

      if(p.readHeader(m_data->f->getNextFileName(), h, orig,
                      angles,version,datatype,dataIdx,offset) == -1){
        throw ICLException("unable to read \"pcl\" point cloud file " 
                           + m_data->f->getNextFileName() + " from pattern " 
                           + m_data->pattern);
      }
      
      for(std::vector< ::sensor_msgs::PointField>::iterator it = h.fields.begin(); 
      it != h.fields.end(); ++it){
        result.push_back(it->name);
        
        if(it == h.fields.begin()+2){
          std::stringstream ss;
          std::copy(result.begin(), result.begin()+3, std::ostream_iterator<std::string>(ss));
          result.clear();
          result.push_back(ss.str());
        }
      }
      return result;
    }
    
  
    void PCDFileGrabber::grab(PointCloudObjectBase &dst){
      std::string filename = m_data->f->getNextFileName();
      std::vector<std::string> fields = getFields();

#define CAST_AND_ASSIGN(T)                                        \
      PCLPointCloudObject<pcl::T> &o = m_data->cast<pcl::T>(dst); \
      pcl::io::loadPCDFile<pcl::T>(filename, o.pcl());            \
      if(&o != &dst) o.deepCopy(dst);
      
      if(has_fields(fields,"xyz")){
          CAST_AND_ASSIGN(PointXYZ);
      }else if(has_fields(fields,"xyz","intensity")){
        CAST_AND_ASSIGN(PointXYZI);
      }else if(has_fields(fields,"xyz","label")){
          CAST_AND_ASSIGN(PointXYZL);
      }else if(has_fields(fields,"xyz","rgb")){
        CAST_AND_ASSIGN(PointXYZRGB);
      }else if(has_fields(fields,"xyz","rgba")){
        CAST_AND_ASSIGN(PointXYZRGBA);
      }else if(has_fields(fields,"xyz","rgba", "label")){
        CAST_AND_ASSIGN(PointXYZRGBL);
      }else if(has_fields(fields,"xyz","streangth")){
        CAST_AND_ASSIGN(InterestPoint);
      }else if(has_fields(fields, "xyz", "rgb", "normal_x", "normal_y", "normal_z", "curvature")){
        CAST_AND_ASSIGN(PointXYZRGBNormal);
      }else if(has_fields(fields,"xyz", "intensity", "normal_x", "normal_y", "normal_z", "curvature")){
        CAST_AND_ASSIGN(PointXYZINormal);
      }else{
        throw ICLException("PCDFileGrabber::grab: unsupported input file type with fields: "
                           + cat(fields,","));
      }

#undef CAST_AND_ASSIGN

      m_data->f->setPropertyValue(m_data->properties, m_data->value);
      if(!m_data->loop && m_data->counter == (int)m_data->f->getFileCount()){
        m_data->properties = "loop";
        m_data->value = "false";
        throw ICLException("No more PCD files available!\n  Set the loop-flag to true, to play PCD file in an endless loop.");
      }
      ++m_data->counter;
    }     
  }
}
