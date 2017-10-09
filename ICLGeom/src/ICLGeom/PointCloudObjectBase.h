/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2013 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLGeom/src/ICLGeom/PointCloudObjectBase.h             **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Patrick Nobou, Andre Ueckermann   **
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

#pragma once

#include <ICLUtils/CompatMacros.h>
#include <ICLUtils/Time.h>
#include <ICLGeom/SceneObject.h>
#include <ICLCore/DataSegment.h>
#include <ICLCore/ImgBase.h>
#include <map>

namespace icl{
  namespace geom{

    /// Base class for point cloud data types
    /** The PointCloudObjectBase class provides a generic but abstract
        interface for point cloud data types. By inheriting the
        SceneObject class, it allows for easy visualization as an object
        in an instance of an icl::Scene. The interface includes querying
        important, but abstract information about the acutal point cloud,
        such as it's size or which features are supported.
        In addition to this, it's select*** methods allow for accessing
        the internal point feature information in a generic manner.

        \section _ORG_ Organized vs not Organized We use the same
        expression here, to avoid misconceptions with the PCL
        nomenclature. Organized point clouds are assumed to have a 2D
        row-major data layout, which is usually the case for point clouds
        acquired form 3D cameras such as kinect. utils::Point clouds whose height
        is less than 1 are assumed to be just 1D, i.e. they contain just an
        unordered set of 3D points and features.

        \section _FEAT_BASE_ Feature Selection
        The generic feature selection mechanism uses the DataSegment utility
        class template. Each accessed feature (such as xyz or rgb) is provided
        in form of a DataSegment whose entries have a compile-time type and
        dimension. The data segment can then easily be used to iterate through
        the data.

        \section _FEAT_ Feature Types
        Basically, the PointCloudObjectBase supports two different types of features:
        -# well known features
        -# arbitrary dynamic features

        \subsection _WKF_ Well Known Features
        For the very common features, such as xyz or rgb/gbr color, a special
        interfaces are provided. These methods have a pre-defined DataSegment
        type, i.e. the type and dimension of their entries is fixed, and must
        not be changed in sub-classes.

        \subsection _CF_ Color Features
        In particular, using color information is unfortunately more
        complex then using the other features, because there are
        different well known predefined color feature types. A method,
        that works generically on an object's color, must usually check
        which color feature type is available, and then use the
        corresponding select method to access the data. Particularly the
        types BGRA and RGBA32f have to be treated differently, because
        BGRA uses byte color values in range [0,255] and swapped blue
        and red channels while RGBA32f uses normal rgb-ordered float
        color values in range [0,1].

        \subsection _DYN_ Dynamic Features
        For all non-common features, a dynamic string-base interface is
        provided. The <tt>select(const std:string&)</tt> method returns
        an abstract data segment instance of type DataSegmentBase, which
        provides binary access to it's referenced data and run-time
        information about it's actual type and feature dimension.
    */
    class ICLGeom_API PointCloudObjectBase : public SceneObject{
      /// timestamp associated with the point cloud
      utils::Time timestamp;

      protected:

      static float length(Vec d){
         float l = sqrt(d[0]*d[0]+d[1]*d[1]+d[2]*d[2]);
         return l;
      }

      /// default color used to render points that have no color information
      GeomColor m_defaultPointColor;

      /// internal map of meta data
      std::map<std::string,std::string> m_metaData;

      /// internally used utility method that throws verbose exceptions
      template<class T, int N>
      core::DataSegment<T,N> &error(const std::string &fname) throw (utils::ICLException){
        throw utils::ICLException("static feature "+fname+" is not supported by this PointCloudObjectBase instance");
        static core::DataSegment<T,N> dummy; return dummy;
      }

      /// internally used utility method that throws verbose exceptions
      core::DataSegmentBase &error_dyn(const std::string &featureName) throw (utils::ICLException){
        throw utils::ICLException("dynamic feature "+featureName+" is not supported by this PointCloudObjectBase instance");
        static core::DataSegmentBase dummy; return dummy;
      }

      /// draw normal lines
      virtual void drawNormalLines();

      bool useDrawNormalLines;
      float normalLineLength;
      int normalLineGranularity;

      bool useMasking;
      core::Img8u maskImage;
      bool useTriangulation;
      float maxDeltaValue;
      bool useTexturing;
      core::Img8u textureImage;
      core::DataSegment<float,2> textureCoordinates;

      //GLuint texName;//
      //int lastTextureWidth, lastTextureHeight;//


      public:

      /// List of well known features
      enum FeatureType {
        /// scalar components 4 bytes each!
        Intensity,      //!< single float intensity
        Label,          //!< single int32 label
        Depth,          //!< single float depth value (interpretation suggestion: dist to camera center)

        BGR,            //!< [uchar b,g,r, padding]
        BGRA,           //!< [uchar b,g,r, alpha]
        BGRA32s,        //!< bgra packed as one icl32s

        // 4D vector components
        XYZ,            //!< [float x,y,z, padding]
        XYZH,           //!< [float x,y,z, homogenous part]
        Normal,         //!< [float nx,ny,nz,curvature]
        RGBA32f,        //!< [float r,g,b,a ]

        NUM_FEATURES
      };

      /// Default constructor
      /** Enables locking in the wrapped SceneObject class */
      PointCloudObjectBase():timestamp(utils::Time::null){
        setLockingEnabled(true);
        m_defaultPointColor = GeomColor(0,0.5,1,1);
        useDrawNormalLines=false;
        useMasking=false;
        useTriangulation=false;
        useTexturing=false;
        //glGenTextures(1,&texName);//
        //lastTextureWidth=0;//
        //lastTextureHeight=0;//

      }

      /// sets the current timestamp
      virtual void setTime(const utils::Time &t) { this->timestamp = t; }

      /// returns the current timestamp
      virtual const utils::Time &getTime() const { return this->timestamp; }

      /// interface for supported features
      virtual bool supports(FeatureType t) const = 0;

      /// interface function for dynamic Point cloud types that can dynamically add features
      /** For features that are already supported, the output is undefined */
      virtual bool canAddFeature(FeatureType t) const { return false; }

      /// interface for adding a feature to an existing point cloud instance
      /** if the given feature cannot be added, an exception is thrown. To
          avoid this, call canAddFeature before using this function.
          Implementations of this function are supposed ignore this call
          in cases where the feature is actually already supported */
      virtual void addFeature(FeatureType t) throw (utils::ICLException){
        throw utils::ICLException("unable to add given feature to point cloud");
      }

      /// interface for supported features
      virtual bool isOrganized() const = 0;

      /// returns the 2D size of the pointcloud (throws exception if not ordered)
      virtual utils::Size getSize() const throw (utils::ICLException) = 0;

      /// return the linearily ordered number of point in the point cloud
      virtual int getDim() const = 0;

      /// adapts the 2D size and enables the 'organized mode'
      /** Implementations of this method should ensure, that the function
          behaves lazy, i.e. if the object has already the desired size,
          nothing should be done.\n
          Furthermore, if the given height is 0 or smaller, the point-cloud
          should be set to the un-organized mode
      */
      virtual void setSize(const utils::Size &size) = 0;

      /// sets the number of contained points (and enables the unorganized mode)
      inline void setDim(int dim){
        setSize(utils::Size(dim,-1));
      }

      /// well know features XYZ (three floats, this feature must usually be available)
      virtual core::DataSegment<float,3> selectXYZ(){ return error<float,3>(__FUNCTION__);   }

      /// common way to store XYZ-data (4th float define homogeneous part)
      virtual core::DataSegment<float,4> selectXYZH(){ return  error<float,4>(__FUNCTION__);   }

      /// well known feature Intensity (single float values)
      virtual core::DataSegment<float,1> selectIntensity(){   return error<float,1>(__FUNCTION__);   }

      /// well known feature Depth (single float values)
      virtual core::DataSegment<float,1> selectDepth(){   return error<float,1>(__FUNCTION__);   }

      /// well known feature Intensity (single 32bit int values)
      virtual core::DataSegment<icl32s,1> selectLabel(){      return error<icl32s,1>(__FUNCTION__);  }

      /// well known feature Intensity (three byte vectors ordered BGR)
      virtual core::DataSegment<icl8u,3> selectBGR(){         return error<icl8u,3>(__FUNCTION__);   }

      /// well known feature Intensity (four byte vectors ordered BGRA)
      virtual core::DataSegment<icl8u,4> selectBGRA(){        return error<icl8u,4>(__FUNCTION__);   }

      /// well known feature Intensity (single int value encoding byte-wise BGRA)
      virtual core::DataSegment<icl32s,1> selectBGRA32s(){    return error<icl32s,1>(__FUNCTION__);   }

      /// well known feature Normal (4 float values)
      /** in the PCL, the 4th value is sometimes used to store a local curvature value */
      virtual core::DataSegment<float,4> selectNormal(){      return error<float,4>(__FUNCTION__);   }

      /// well known feature RGBA (4 float values, ordred RGBA)
      virtual core::DataSegment<float,4> selectRGBA32f(){     return error<float,4>(__FUNCTION__);   }

      /// dynamic feature selection function
      /** This can be implemented in subclasses to grant access to less common feature types
          such as feature point descriptors */
      virtual core::DataSegmentBase select(const std::string &featureName) { return error_dyn(featureName); }

      // const select methds

      /// const xyz data
      const core::DataSegment<float,3> selectXYZ() const { return const_cast<PointCloudObjectBase*>(this)->selectXYZ(); }

      /// const xyzh data
      const core::DataSegment<float,4> selectXYZH() const { return const_cast<PointCloudObjectBase*>(this)->selectXYZH(); }

      /// const intensity data
      const core::DataSegment<float,1> selectIntensity() const { return const_cast<PointCloudObjectBase*>(this)->selectIntensity(); }

      /// const intensity data
      const core::DataSegment<float,1> selectDepth() const { return const_cast<PointCloudObjectBase*>(this)->selectDepth(); }

      /// const label data
      const core::DataSegment<icl32s,1> selectLabel() const { return const_cast<PointCloudObjectBase*>(this)->selectLabel(); }

      /// const bgr data
      const core::DataSegment<icl8u,3> selectBGR() const { return const_cast<PointCloudObjectBase*>(this)->selectBGR(); }

      /// const rgba  data
      const core::DataSegment<icl8u,4> selectBGRA() const { return const_cast<PointCloudObjectBase*>(this)->selectBGRA(); }

      /// const bgra32s data
      const core::DataSegment<icl32s,1> selectBGRA32s() const { return const_cast<PointCloudObjectBase*>(this)->selectBGRA32s(); }

      /// const normals data
      const core::DataSegment<float,4> selectNormal() const { return const_cast<PointCloudObjectBase*>(this)->selectNormal(); }

      /// const rgba32f data
      const core::DataSegment<float,4> selectRGBA32f() const { return const_cast<PointCloudObjectBase*>(this)->selectRGBA32f(); }

      /// const dynamic/custom data
      const core::DataSegmentBase select(const std::string &featureName) const {
        return const_cast<PointCloudObjectBase*>(this)->select(featureName);
      }

      /// tints the point cloud pixel from the given image data
      /** The image size must be equal to the point cloud size*/
      void setColorsFromImage(const core::ImgBase &image) throw (utils::ICLException);

      /// extracts the color information and stores it into the given image
      /** The image size and color format is adapted if necessary */
      void extractColorsToImage(core::ImgBase &image, bool withAlpha=false) const throw (utils::ICLException);

      /// sets the color that is used to render points if color information is available
      void setDefaultVertexColor(const GeomColor &color);

      /// implements the SceneObject's virtual getter function for this feature
      virtual GeomColor getDefaultVertexColor() const{ return m_defaultPointColor*255; }

      /// For subclasses that provide Dynamic features, this function must be implemented
      virtual std::vector<std::string> getSupportedDynamicFeatures() const {
        return std::vector<std::string>();
      }

      /// custom rendering method
      /** The basic implementation of this method uses the XYZ data
          segment to render points using an OpenGL vertex array. If any
          color information is available, it is used to tint the drawn
          points using OpenGL color pointer. Miss-ordered BGR types are
          rendered using an OpenGL fragment shaded to swap the
          transferred blue and red channels */
      virtual void customRender();

      /// set use draw normal lines
      void setUseDrawNormalLines(bool use, float lineLength=40, int granularity=4);

      // set use masking (0 render, 1 dont render)
      void setUseMasking(bool use, core::Img8u &mask);

      //set use triangulation
      void setUseTriangulation(bool use, float maxDelta=50);

      //set texturing
      void setUseTexturing(bool use, core::Img8u &tex, core::DataSegment<float,2> texCoords);

      /// deep copy interface (needs to be implemented by subclasses)
      virtual PointCloudObjectBase *copy() const {
        return 0;
      }

      /// deeply copies all well-known features that are shared by this and dst
      virtual void deepCopy(PointCloudObjectBase &dst) const;

      /// returns whether two points clouds are equal
      virtual bool equals(const PointCloudObjectBase &dst,
                          bool compareOnlySharedFeatures=false,
                          bool allowDifferentColorTypes=true,
                          float tollerance=1.0e-5) const;

      /// returns the meta data associated with this point cloud object
      std::map<std::string,std::string> &getMetaData();

      /// returns the meta data associated with this point cloud object (const)
      const std::map<std::string,std::string> &getMetaData() const;

      /// returns the meta data associated with a given key
      const std::string &getMetaData(const std::string &key) const throw (utils::ICLException);

      /// returns whether meta data to the given key is associated
      bool hasMetaData(const std::string &key) const;

      /// returns whether any meta data is available
      bool hasAnyMetaData() const;

      /// sets the meta data entry for given key to value
      void setMetaData(const std::string &key, const std::string &value);

      /// deletes all meta data entries
      void clearAllMetaData();

      /// clears the meta data for a given key
      void clearMetaData(const std::string &key);

      /// returns a list of all available meta data entires
      std::vector<std::string> getAllMetaDataEntries() const;
    };

    /// overloaded ostream operator for PointCloudObjectBase::FeatureType
    ICLGeom_API std::ostream &operator<<(std::ostream &s, const PointCloudObjectBase::FeatureType t);
  } // namespace geom
}

