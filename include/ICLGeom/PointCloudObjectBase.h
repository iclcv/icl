/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2012 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : include/ICLGeom/PointCloudObjectBase.h                 **
** Module : ICLGeom                                                **
** Authors: Christof Elbrechter, Patrick Nobou                     **
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

#ifndef ICL_POINT_CLOUD_OBJECT_BASE_H
#define ICL_POINT_CLOUD_OBJECT_BASE_H

#include <ICLGeom/SceneObject.h>
#include <ICLGeom/DataSegment.h>
#include <map>

namespace icl{

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
      acquired form 3D cameras such as kinect. Point clouds whose height
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
  class PointCloudObjectBase : public SceneObject{
    protected:
    std::map<std::string,std::string> m_metaData;
    
    /// internally used utility method that throws verbose exceptions
    template<class T, int N>
    DataSegment<T,N> &error(const std::string &fname) throw (ICLException){
      throw ICLException("static feature "+fname+" is not supported by this PointCloudObjectBase instance");
      static DataSegment<T,N> dummy; return dummy;
    }

    /// internally used utility method that throws verbose exceptions
    DataSegmentBase &error_dyn(const std::string &featureName) throw (ICLException){
      throw ICLException("dynamic feature "+featureName+" is not supported by this PointCloudObjectBase instance");
      static DataSegmentBase dummy; return dummy;
    }

    public:

    /// List of well known features
    enum FeatureType {
      /// scalar components 4 bytes each!
      Intensity,      //!< single float intensity
      Label,          //!< single int32 label

      BGR,            //!< [uchar b,g,r, padding]
      BGRA,           //!< [uchar b,g,r, alpha]
      BGRA32s,        //!< bgra packed as one icl32s

      // 4D vector components
      XYZ,            //!< [float x,y,z, padding]
      Normal,         //!< [float nx,ny,nz,curvature]
      RGBA32f,        //!< [float r,g,b,a ]

      NUM_FEATURES
    };

    /// Default constructor
    /** Enables locking in the wrapped SceneObject class */
    PointCloudObjectBase(){
      setLockingEnabled(true);
    }

    /// interface for supported features 
    virtual bool supports(FeatureType t) = 0;

    /// interface for supported features 
    virtual bool isOrganized() const = 0;

    /// returns the 2D size of the pointcloud (throws exception if not ordered)
    virtual Size getSize() const throw (ICLException) = 0;
    
    /// return the linearily ordered number of point in the point cloud
    virtual int getDim() const = 0;
    
    /// adapts the 2D size and enables the 'organized mode'
    /** Implementations of this method should ensure, that the function 
        behaves lazy, i.e. if the object has already the desired size,
        nothing should be done.\n
        Furthermore, if the given width is 0 or smaller, the point-cloud
        should be set to the un-organized mode
    */
    virtual void setSize(const Size &size) = 0;

    /// sets the number of contained points (and enables the unorganized mode)
    inline void setDim(int dim){
      setSize(Size(dim,-1));
    }

    /// well know features XYZ (three floats, this feature must usually be available)
    virtual DataSegment<float,3> selectXYZ(){         return error<float,3>(__FUNCTION__);   }
    
    /// well known feature Intensity (single float values)
    virtual DataSegment<float,1> selectIntensity(){   return error<float,1>(__FUNCTION__);   }

    /// well known feature Intensity (single 32bit int values)
    virtual DataSegment<icl32s,1> selectLabel(){      return error<icl32s,1>(__FUNCTION__);  }

    /// well known feature Intensity (three byte vectors ordered BGR)
    virtual DataSegment<icl8u,3> selectBGR(){         return error<icl8u,3>(__FUNCTION__);   }

    /// well known feature Intensity (four byte vectors ordered BGRA)
    virtual DataSegment<icl8u,4> selectBGRA(){        return error<icl8u,4>(__FUNCTION__);   }

    /// well known feature Intensity (single int value encoding byte-wise BGRA)
    virtual DataSegment<icl32s,1> selectBGRA32s(){    return error<icl32s,1>(__FUNCTION__);   }

    /// well known feature Normal (4 float values)
    /** in the PCL, the 4th value is sometimes used to store a local curvature value */
    virtual DataSegment<float,4> selectNormal(){      return error<float,4>(__FUNCTION__);   }

    /// well known feature RGBA (4 float values, ordred RGBA)
    virtual DataSegment<float,4> selectRGBA32f(){     return error<float,4>(__FUNCTION__);   }

    /// dynamic feature selection function
    /** This can be implemented in subclasses to grant access to less common feature types
        such as feature point descriptors */
    virtual DataSegmentBase select(const std::string &featureName) { return error_dyn(featureName); }

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
    
    /// deep copy interface (needs to be implemented by subclasses)
    virtual PointCloudObjectBase *copy() const {
      return 0;    
    }

    /// returns the meta data associated with this point cloud object
    std::map<std::string,std::string> &getMetaData();
    
    /// returns the meta data associated with this point cloud object (const)
    const std::map<std::string,std::string> &getMetaData() const;
    
    /// returns the meta data associated with a given key
    const std::string &getMetaData(const std::string &key) const throw (ICLException);

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
  std::ostream &operator<<(std::ostream &s, const PointCloudObjectBase::FeatureType t);
}

#endif
