/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
 **                         Neuroinformatics Group                  **
 ** Website: www.iclcv.org and                                      **
 **          http://opensource.cit-ec.de/projects/icl               **
 **                                                                 **
 ** File   : ICLFilter/src/GenericSurfDetector.h                    **
 ** Module : ICLFilter                                              **
 ** Authors: Christian Groszewski                                   **
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
#ifndef ICL_GENERICSURFDETECTOR_H_
#define ICL_GENERICSURFDETECTOR_H_

#include <ICLCore/ImgBase.h>

#include <ICLAlgorithms/OpenCVSurfDetector.h>
#include <ICLAlgorithms/OpenSurfDetector.h>
#ifdef HAVE_QT
#include <ICLQt/DrawWidget.h>
#endif
#include <cv.h>
#include <opensurf/surflib.h>
namespace icl{

class GenericSurfDetector{
private :

	///Forwarddeklaration.
	class Data;
	///Class for internal params and buffers.
	Data *m_data;
public :

	enum IMPLEMENTATION{
		OPENSURF,
		OPENCV
	};

	GenericSurfDetector(IMPLEMENTATION impl);

	GenericSurfDetector(const ImgBase *obj=0,double threshold=500, int extended=1,
			int octaves=3, int octavelayer=4);

	GenericSurfDetector(const ImgBase *obj=0,  bool upright=false, int octaves=4,
			int intervals=4, int init_samples=2, float thresh=0.004f);

	~GenericSurfDetector();

	struct GenericPointImpl{

		virtual ~GenericPointImpl(){};

		virtual Point32f getCenter() const = 0;

		virtual int getRadius() const = 0;

		virtual int getLaplacian() const = 0;

		virtual float getDir() const = 0;
	};

	struct GenericPoint{
		SmartPtr<GenericPointImpl> impl;
		GenericPoint(){};
		GenericPoint(GenericPointImpl* i):impl(SmartPtr<GenericPointImpl>(i)){}
		virtual ~GenericPoint(){};

		Point32f getCenter() const { return impl->getCenter(); }

		int getRadius() const { return impl->getRadius(); }

		int getLaplacian() const {return impl->getLaplacian();}

		float getDir() const {return impl->getDir();}

		//OpenSurf only
		float getScale() const throw (ICLException){
			const OpenSurfGenericPointImpl *point=dynamic_cast<const OpenSurfGenericPointImpl*> (impl.get());
			if(point != 0){
				return point->getScale();
			}else{
				throw ICLException("Error");
			}
		}

		const float* getDescriptor() const throw (ICLException){
			const OpenSurfGenericPointImpl *point = dynamic_cast<const OpenSurfGenericPointImpl*> (impl.get());
			if(point != 0){
				return point->getDescriptor();
			}else{
				throw ICLException("Error");
			}
		}

		float getDx() const throw (ICLException){
			const OpenSurfGenericPointImpl *point=dynamic_cast<const OpenSurfGenericPointImpl*> (impl.get());
			if(point != 0){
				return point->getDx();
			}else{
				throw ICLException("Error");
			}
		}

		float getDy() const throw (ICLException){
			const OpenSurfGenericPointImpl *point=dynamic_cast<const OpenSurfGenericPointImpl*> (impl.get());
			if(point != 0){
				return point->getDy();
			}else{
				throw ICLException("Error");
			}
		}

		int getClusterIndex() const throw (ICLException){
			const OpenSurfGenericPointImpl *point=dynamic_cast<const OpenSurfGenericPointImpl*> (impl.get());
			if(point != 0){
				return point->getClusterIndex();
			}else{
				throw ICLException("Error");
			}
		}

		//OpenCV only
		int getSize() const throw (ICLException){
			const OpenCVGenericPointImpl *point=dynamic_cast<const OpenCVGenericPointImpl*> (impl.get());
			if(point != 0){
				return point->getSize();
			}else{
				throw ICLException("Error");
			}
		}

		float getHessian() const throw (ICLException){
			const OpenCVGenericPointImpl *point=dynamic_cast<const OpenCVGenericPointImpl*> (impl.get());
			if(point != 0){
				return point->getHessian();
			}else{
				throw ICLException("Error");
			}
		}
	};

	struct OpenCVGenericPointImpl : public GenericPointImpl{
		const CvSURFPoint *p;

		OpenCVGenericPointImpl(const CvSURFPoint *point):p(point){}

		~OpenCVGenericPointImpl(){}

		virtual Point32f getCenter() const {
			return Point32f(p->pt.x,p->pt.y);
		}

		virtual int getRadius() const {
			return int(p->size*1.2/9.*2);
		}

		virtual int getLaplacian() const {return p->laplacian;}

		virtual float getDir() const {return p->dir;}

		int getSize() const {return p->size;}

		float getHessian() const {return p->hessian;}
	};

	struct OpenSurfGenericPointImpl : public GenericPointImpl{

		const Ipoint *p;

		OpenSurfGenericPointImpl(const Ipoint *point):p(point){}

		~OpenSurfGenericPointImpl(){}

		virtual Point32f getCenter() const {
			return Point32f(p->x,p->y);
		}

		virtual int getRadius() const {
			return int(2.5f * p->scale);
		}

		virtual float getDir() const {return p->orientation;}

		virtual int getLaplacian() const {return p->laplacian;}

		float getScale() const {return p->scale;}

		const float* getDescriptor() const {return p->descriptor;}

		float getDx() const {return p->dx;}

		float getDy() const {return p->dy;}

		int getClusterIndex() const {return p->clusterIndex;}
	};

	void setObjectImg(const ImgBase *objectImg) throw (ICLException);

	///returns back converted image
	const ImgBase *getObjectImg() throw (ICLException);

	const std::vector<GenericPoint> &getObjectImgFeatures();

	void setRotationInvariant(bool upright) throw (ICLException);

	void setOctaves(int octaves);

	void setOctavelayer(int octavelayer);

	void setInitSamples(int init_samplex) throw (ICLException);

	void setThreshold(double threshold);

	void setExtended(int extended) throw (ICLException);

	//getter
	bool getRotationInvariant() throw (ICLException);

	int getOctaves();

	int getOctavelayer();

	int getInitSamples() throw (ICLException);

	double getThreshold();

	int getExtended() throw (ICLException);

	void setParams(double threshold, int extended,
			int octaves, int octavelayer);

	void setParams(bool upright, int octaves, int intervals,
			int init_samples,float thresh);

	const std::vector<GenericPoint> &extractFeatures(const ImgBase *src) throw (ICLException);

	const std::vector<std::pair<GenericPoint, GenericPoint> > &match(const ImgBase *image) throw (ICLException);

	const std::vector<std::pair<GenericPoint, GenericPoint> > &match(const ImgBase *currentImage, const ImgBase *newObjImage){
		setObjectImg(newObjImage);
		return match(currentImage);
	}

#ifdef HAVE_QT
	static void visualizeFeature(ICLDrawWidget &w,const GenericPoint &p);
	static void visualizeFeatures(ICLDrawWidget &w, const std::vector<GenericPoint> &features);
	static void visualizeMatches(ICLDrawWidget &w_object,ICLDrawWidget &w_result,
			const std::vector<std::pair<GenericPoint, GenericPoint> > &matches);
#endif
};
}

#endif /* ICL_GENERICSURFDETECTOR_H_ */
