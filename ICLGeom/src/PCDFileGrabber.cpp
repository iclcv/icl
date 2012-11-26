/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/PCDFileGrabber.cpp                         **
** Module : ICLGeom                                                **
** Authors: Patrick Nobou                                          **
**                                                                 **
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

#include <pcl/point_cloud.h>
#include <pcl/io/pcd_io.h>
#include <pcl/point_types.h>
#include <pcl/io/file_io.h>
#include <pcl/io/pcd_grabber.h>

#include <ICLGeom/PCDFileGrabber.h>
#include <ICLIO/FileGrabber.h>
#include <ICLGeom/PCLPointCloudObject.h>


#include <sstream>


namespace icl{

  using namespace utils;
  using namespace math;
  using namespace io;

  namespace geom{
    /// TODO later: move this functionality to the PointCloudObjectBase class
    static void copy_fields_from_to(const PointCloudObjectBase &src, PointCloudObjectBase &dst){
      // TODO adapt dst size and dim (and organized feature) to the src values
      
      if(src.isOrganized()){
        dst.setSize(src.getSize());
      }else{
        dst.setSize(Size(src.getDim(), 1));
      }

      // copy well known fields:
      DataSegment<float,3> xyz;
      if(!dst.supports(PointCloudObjectBase::XYZ)){
        throw ICLException("Destination's point type don't support XYZ point type!");
      }

      xyz = (const_cast<PointCloudObjectBase& >(src)).selectXYZ();

      if(dynamic_cast<PCLPointCloudObject<pcl::PointXYZ>* >(&dst)){
        PCLPointCloudObject<pcl::PointXYZ> &tmp = (PCLPointCloudObject<pcl::PointXYZ> &)dst;
        for(int i = 0; i < dst.getDim(); ++i){
          tmp.pcl().points[i].x = xyz[i][0];
          tmp.pcl().points[i].y = xyz[i][1];
          tmp.pcl().points[i].z = xyz[i][2];
        }
      }else if(dynamic_cast<PCLPointCloudObject<pcl::PointXYZL>* >(&dst)){
        DataSegment<icl32s,1> label = (const_cast<PointCloudObjectBase& >(src)).selectLabel();
        PCLPointCloudObject<pcl::PointXYZL> &tmp = (PCLPointCloudObject<pcl::PointXYZL> &)dst;
        for(int i = 0; i < dst.getDim(); ++i){
          tmp.pcl().points[i].x = xyz[i][0];
          tmp.pcl().points[i].y = xyz[i][1];
          tmp.pcl().points[i].z = xyz[i][2];
          tmp.pcl().points[i].label = label[i];
        }
      }else if(dst.supports(PointCloudObjectBase::RGBA32f)){
        DataSegment<float,4> rgba32 = (const_cast<PointCloudObjectBase& >(src)).selectRGBA32f();
        PCLPointCloudObject<pcl::PointXYZRGBA> &tmp = (PCLPointCloudObject<pcl::PointXYZRGBA> &)dst;
        for(int i = 0; i < dst.getDim(); ++i){
          tmp.pcl().points[i].x = xyz[i][0];
          tmp.pcl().points[i].y = xyz[i][1];
          tmp.pcl().points[i].z = xyz[i][2];
          tmp.pcl().points[i].r = rgba32[i][0];
          tmp.pcl().points[i].g = rgba32[i][1];
          tmp.pcl().points[i].b = rgba32[i][2];
          //tmp.pcl().points[i].a = rgba32[i][3];
        }
      }else if(dst.supports(PointCloudObjectBase::BGRA)){
        DataSegment<icl8u,4> bgra = (const_cast<PointCloudObjectBase& >(src)).selectBGRA();
        PCLPointCloudObject<pcl::PointXYZRGBA> &tmp = (PCLPointCloudObject<pcl::PointXYZRGBA> &)dst;
        for(int i = 0; i < dst.getDim(); ++i){
          tmp.pcl().points[i].x = xyz[i][0];
          tmp.pcl().points[i].y = xyz[i][1];
          tmp.pcl().points[i].z = xyz[i][2];
          tmp.pcl().points[i].b = bgra[i][0];
          tmp.pcl().points[i].g = bgra[i][1];
          tmp.pcl().points[i].r = bgra[i][2];
          //tmp.pcl().points[i].a = rgba[i][3];
        }
      }else if(dst.supports(PointCloudObjectBase::BGR)){
        DataSegment<icl8u,3> bgr = (const_cast<PointCloudObjectBase& >(src)).selectBGR();
        if(dst.supports(PointCloudObjectBase::Label)){
          DataSegment<icl32s,1> label = (const_cast<PointCloudObjectBase& >(src)).selectLabel();
          PCLPointCloudObject<pcl::PointXYZRGBL> &tmp = (PCLPointCloudObject<pcl::PointXYZRGBL> &)dst;
          for(int i = 0; i < dst.getDim(); ++i){
            tmp.pcl().points[i].x = xyz[i][0];
            tmp.pcl().points[i].y = xyz[i][1];
            tmp.pcl().points[i].z = xyz[i][2];
            tmp.pcl().points[i].b = bgr[i][0];
            tmp.pcl().points[i].g = bgr[i][1];
            tmp.pcl().points[i].r = bgr[i][2];
            tmp.pcl().points[i].label = label[i];
          }
        }else if(dst.supports(PointCloudObjectBase::Normal)){
          DataSegment<float,4> normal = (const_cast<PointCloudObjectBase& >(src)).selectNormal();
          PCLPointCloudObject<pcl::PointXYZRGBNormal> &tmp = (PCLPointCloudObject<pcl::PointXYZRGBNormal> &)dst;
          for(int i = 0; i < dst.getDim(); ++i){
            tmp.pcl().points[i].x = xyz[i][0];
            tmp.pcl().points[i].y = xyz[i][1];
            tmp.pcl().points[i].z = xyz[i][2];
            tmp.pcl().points[i].b = bgr[i][0];
            tmp.pcl().points[i].g = bgr[i][1];
            tmp.pcl().points[i].r = bgr[i][2];
            tmp.pcl().points[i].normal_x = normal[i][0];
            tmp.pcl().points[i].normal_y = normal[i][1];
            tmp.pcl().points[i].normal_z = normal[i][2];
            tmp.pcl().points[i].curvature = normal[i][3];
          }
        }else{
          PCLPointCloudObject<pcl::PointXYZRGB> &tmp = (PCLPointCloudObject<pcl::PointXYZRGB> &)dst;
          for(int i = 0; i < dst.getDim(); ++i){
            tmp.pcl().points[i].x = xyz[i][0];
            tmp.pcl().points[i].y = xyz[i][1];
            tmp.pcl().points[i].z = xyz[i][2];
            tmp.pcl().points[i].b = bgr[i][0];
            tmp.pcl().points[i].g = bgr[i][1];
            tmp.pcl().points[i].r = bgr[i][2];
          }
        }
      }else if(dst.supports(PointCloudObjectBase::Intensity)){
        DataSegment<float,1> intensity = (const_cast<PointCloudObjectBase& >(src)).selectIntensity();
        if(dst.supports(PointCloudObjectBase::Normal)){
          DataSegment<float,4> normal = (const_cast<PointCloudObjectBase& >(src)).selectNormal();
          PCLPointCloudObject<pcl::PointXYZINormal> &tmp = (PCLPointCloudObject<pcl::PointXYZINormal> &)dst;
          for(int i = 0; i < dst.getDim(); ++i){
            tmp.pcl().points[i].x = xyz[i][0];
            tmp.pcl().points[i].y = xyz[i][1];
            tmp.pcl().points[i].z = xyz[i][2];
            tmp.pcl().points[i].intensity = intensity[i];
            tmp.pcl().points[i].normal_x = normal[i][0];
            tmp.pcl().points[i].normal_y = normal[i][1];
            tmp.pcl().points[i].normal_z = normal[i][2];
            tmp.pcl().points[i].curvature = normal[i][3];
          }
        }else{
          PCLPointCloudObject<pcl::PointXYZI> &tmp = (PCLPointCloudObject<pcl::PointXYZI> &)dst;
          for(int i = 0; i < dst.getDim(); ++i){
            tmp.pcl().points[i].x = xyz[i][0];
            tmp.pcl().points[i].y = xyz[i][1];
            tmp.pcl().points[i].z = xyz[i][2];
            tmp.pcl().points[i].intensity = intensity[i];
          }
        }
      }else{
        throw ICLException("Implement use of generic point type!");
      }
    }

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

    struct PCDFileGrabber::Data{
      /// file name or pattern
      std::string sPattern;

      /// the resultant point cloud dataset (only the header will be filled)
      sensor_msgs::PointCloud2 nextCloudHeader;

      /// the sensor acquisition origin (only for > PCD_V7 - null if not present)
      Eigen::Vector4f origin;

      /// the sensor acquisition orientation (only for > PCD_V7 - identity if not present)
      Eigen::Quaternionf orientation;

      /// the PCD version of the file (i.e., PCD_V6, PCD_V7)
      int pcdVersion;

      /// the type of data (0 = ASCII, 1 = Binary, 2 = Binary compressed)
      int dataType;

      /// the offset of cloud data within the file
      unsigned int dataIdx;

      ///
      std::string properties;

      ///
      std::string value;

      /// whether to play PCD file in an endless loop or not.
      bool loop;

      /// Flag to specify the use of wellknow point type
      bool forceExactPCLType;

      /// Data offset
      int offset;

      /// pcdFilenameList of point cloud file name
      std::vector<std::string> pcdFilenameList;

      /// List of point cloud file name (we abuse the filegrabber here for the loop logic)
      FileGrabber *f;

      ///
      int counter;

      EIGEN_MAKE_ALIGNED_OPERATOR_NEW
    };


    PCDFileGrabber::PCDFileGrabber(void):m_data(0){}


    PCDFileGrabber::PCDFileGrabber(const std::string &pattern, bool loop, bool forceExactPCLType, const int offset):m_data(new Data)
    {
      if(!pattern.length()){
        throw ICLException("PCDFileGrabber::PCDFileGrabber: Wrong file/pattern name!");
      }

      m_data->sPattern = pattern;
      m_data->nextCloudHeader = sensor_msgs::PointCloud2();
      m_data->origin = Eigen::Vector4f();
      m_data->orientation = Eigen::Quaternionf();
      //        m_data->pcdVersion = 0;
      //        m_data->dataType = 0;
      //        m_data->dataIdx = 0;
      m_data->properties = "next";
      m_data->value = "";
      m_data->loop = loop;
      m_data->forceExactPCLType = forceExactPCLType;
      m_data->offset = offset;
      m_data->f = new FileGrabber(pattern);
      m_data->counter = 0;
    }

    PCDFileGrabber::~PCDFileGrabber(void) throw(){
      ICL_DELETE(m_data);
    }

    std::vector<std::string> PCDFileGrabber::getFields() const{
      std::vector<std::string> result;
      pcl::PCDReader p;

      if(p.readHeader(m_data->f->getNextFileName(), m_data->nextCloudHeader, m_data->origin, m_data->orientation, m_data->pcdVersion, m_data->dataType, m_data->dataIdx, m_data->offset) == -1){
        throw ICLException("unable to read \"pcl\" point cloud file " + m_data->f->getNextFileName() + " from pattern " + m_data->sPattern);
      }

      for(std::vector< ::sensor_msgs::PointField>::iterator it = m_data->nextCloudHeader.fields.begin(); it != m_data->nextCloudHeader.fields.end(); ++it){
        result.push_back(it->name);

        if(it == m_data->nextCloudHeader.fields.begin()+2){
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

      if(has_fields(fields,"xyz")){
        if(m_data->forceExactPCLType){
          if(!dynamic_cast<PCLPointCloudObject<pcl::PointXYZ>* >(&dst)){
            throw ICLException("PCDFileGrabber::grab: type missmatch. Try to put the forceExactPCLType's flag to false.");
          }
          PCLPointCloudObject<pcl::PointXYZ>& tmp = (PCLPointCloudObject<pcl::PointXYZ> &)dst;
          pcl::io::loadPCDFile<pcl::PointXYZ>(filename, tmp.pcl());
        }else{ // not force
          pcl::PointCloud<pcl::PointXYZ> t;
          PCLPointCloudObject<pcl::PointXYZ> tmp;
          tmp.setPCL(t);

          pcl::io::loadPCDFile<pcl::PointXYZ>(filename, tmp.pcl());
          copy_fields_from_to(tmp,dst);
        }
      }else if(has_fields(fields,"xyz","intensity")){
        if(m_data->forceExactPCLType){
          if(!dynamic_cast<PCLPointCloudObject<pcl::PointXYZI>* >(&dst)){
            throw ICLException("PCDFileGrabber::grab: type missmatch. Try to put the forceExactPCLType's flag to false.");
          }
          PCLPointCloudObject<pcl::PointXYZI>& tmp = (PCLPointCloudObject<pcl::PointXYZI> &)dst;
          pcl::io::loadPCDFile<pcl::PointXYZI>(filename, tmp.pcl());
        }else{ // not force
          pcl::PointCloud<pcl::PointXYZI> t;
          PCLPointCloudObject<pcl::PointXYZI> tmp;
          tmp.setPCL(t);

          pcl::io::loadPCDFile<pcl::PointXYZI>(filename, tmp.pcl());
          copy_fields_from_to(tmp,dst);
        }
      }else if(has_fields(fields, "xyz", "label")){
        if(m_data->forceExactPCLType){
          if(!dynamic_cast<PCLPointCloudObject<pcl::PointXYZL>* >(&dst)){
            throw ICLException("PCDFileGrabber::grab: type missmatch. Try to put the forceExactPCLType's flag to false.");
          }
          PCLPointCloudObject<pcl::PointXYZL>& tmp = (PCLPointCloudObject<pcl::PointXYZL> &) dst;
          pcl::io::loadPCDFile<pcl::PointXYZL>(filename, tmp.pcl());
        }else{
          pcl::PointCloud<pcl::PointXYZL> t;
          PCLPointCloudObject<pcl::PointXYZL> tmp;
          tmp.setPCL(t);

          pcl::io::loadPCDFile<pcl::PointXYZL>(filename, tmp.pcl());
          copy_fields_from_to(tmp,dst);
        }
      }else if(has_fields(fields, "xyz", "rgb")){
        if(m_data->forceExactPCLType){
          if(!dynamic_cast<PCLPointCloudObject<pcl::PointXYZRGB>* >(&dst)){
            throw ICLException("PCDFileGrabber::grab: type missmatch. Try to put the forceExactPCLType's flag to false.");
          }
          PCLPointCloudObject<pcl::PointXYZRGB>& tmp = (PCLPointCloudObject<pcl::PointXYZRGB> &)dst;
          pcl::io::loadPCDFile<pcl::PointXYZRGB>(filename, tmp.pcl());
        }else{
          pcl::PointCloud<pcl::PointXYZRGB> t;
          PCLPointCloudObject<pcl::PointXYZRGB> tmp;
          tmp.setPCL(t);

          pcl::io::loadPCDFile<pcl::PointXYZRGB>(filename, tmp.pcl());

          copy_fields_from_to(tmp,dst);
        }
      }else if(has_fields(fields, "xyz" , "rgba")){
        if(m_data->forceExactPCLType){
          if(!dynamic_cast<PCLPointCloudObject<pcl::PointXYZRGBA>* >(&dst)){
            throw ICLException("PCDFileGrabber::grab: type missmatch. Try to put the forceExactPCLType's flag to false.");
          }
          PCLPointCloudObject<pcl::PointXYZRGBA>& tmp = (PCLPointCloudObject<pcl::PointXYZRGBA> &)dst;
          pcl::io::loadPCDFile<pcl::PointXYZRGBA>(filename, tmp.pcl());
        }else{
          pcl::PointCloud<pcl::PointXYZRGBA> t;
          PCLPointCloudObject<pcl::PointXYZRGBA> tmp;
          tmp.setPCL(t);

          pcl::io::loadPCDFile<pcl::PointXYZRGBA>(filename, tmp.pcl());
          copy_fields_from_to(tmp,dst);
        }
      }else if(has_fields(fields, "xyz", "rgba" ,"label")){
        if(m_data->forceExactPCLType){
          if(!dynamic_cast<PCLPointCloudObject<pcl::PointXYZRGBL>* >(&dst)){
            throw ICLException("PCDFileGrabber::grab: type missmatch. Try to put the forceExactPCLType's flag to false.");
          }
          PCLPointCloudObject<pcl::PointXYZRGBL>& tmp = (PCLPointCloudObject<pcl::PointXYZRGBL> &)dst;
          pcl::io::loadPCDFile<pcl::PointXYZRGBL>(filename, tmp.pcl());
        }else{
          pcl::PointCloud<pcl::PointXYZRGBL> t;
          PCLPointCloudObject<pcl::PointXYZRGBL> tmp;
          tmp.setPCL(t);

          pcl::io::loadPCDFile<pcl::PointXYZRGBL>(filename, tmp.pcl());
          copy_fields_from_to(tmp,dst);
        }
      }else if(has_fields(fields, "xyz", "strength")){
        if(m_data->forceExactPCLType){
          if(!dynamic_cast<PCLPointCloudObject<pcl::InterestPoint>* >(&dst)){
            throw ICLException("PCDFileGrabber::grab: type missmatch. Try to put the forceExactPCLType's flag to false.");
          }
          PCLPointCloudObject<pcl::InterestPoint>& tmp = (PCLPointCloudObject<pcl::InterestPoint> &)dst;
          pcl::io::loadPCDFile<pcl::InterestPoint>(filename, tmp.pcl());
        }else{
          pcl::PointCloud<pcl::InterestPoint> t;
          PCLPointCloudObject<pcl::InterestPoint> tmp;
          tmp.setPCL(t);

          pcl::io::loadPCDFile<pcl::InterestPoint>(filename, tmp.pcl());
          copy_fields_from_to(tmp,dst);
        }
      }else if(has_fields(fields, "xyz", "rgb", "normal_x", "normal_y", "normal_z", "curvature")){
        if(m_data->forceExactPCLType){
          if(!dynamic_cast<PCLPointCloudObject<pcl::PointXYZRGBNormal>* >(&dst)){
            throw ICLException("PCDFileGrabber::grab: type missmatch. Try to put the forceExactPCLType's flag to false.");
          }
          PCLPointCloudObject<pcl::PointXYZRGBNormal>& tmp = (PCLPointCloudObject<pcl::PointXYZRGBNormal> &)dst;
          pcl::io::loadPCDFile<pcl::PointXYZRGBNormal>(filename, tmp.pcl());
        }else{
          pcl::PointCloud<pcl::PointXYZRGBNormal> t;
          PCLPointCloudObject<pcl::PointXYZRGBNormal> tmp;
          tmp.setPCL(t);

          pcl::io::loadPCDFile<pcl::PointXYZRGBNormal>(filename, tmp.pcl());
          copy_fields_from_to(tmp,dst);
        }
      }else if(has_fields(fields, "xyz", "intensity", "normal_x", "normal_y", "normal_z", "curvature")){
        if(m_data->forceExactPCLType){
          if(!dynamic_cast<PCLPointCloudObject<pcl::PointXYZINormal>* >(&dst)){
            throw ICLException("PCDFileGrabber::grab: type missmatch. Try to put the forceExactPCLType's flag to false.");
          }
          PCLPointCloudObject<pcl::PointXYZINormal>& tmp = (PCLPointCloudObject<pcl::PointXYZINormal> &)dst;
          pcl::io::loadPCDFile<pcl::PointXYZINormal>(filename, tmp.pcl());
        }else{
          pcl::PointCloud<pcl::PointXYZINormal> t;
          PCLPointCloudObject<pcl::PointXYZINormal> tmp;
          tmp.setPCL(t);

          pcl::io::loadPCDFile<pcl::PointXYZINormal>(filename, tmp.pcl());
          copy_fields_from_to(tmp,dst);
        }
      }else{
        throw ICLException("PCDFileGrabber::grab: Used point type don't match point type in the files!");
      }
      
      // TODO: throw exception if loop is of and last file was read

      m_data->f->setProperty(m_data->properties, m_data->value);
      if(!m_data->loop && m_data->counter == (int)m_data->f->getFileCount()){
        m_data->properties = "loop";
        m_data->value = "false";

        throw ICLException("No more PCD files available!\n  Set the loop's flag to true, to play PCD file in an endless loop.");
      }
      ++m_data->counter;
    }
  }
}
