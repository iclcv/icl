/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
 **                         University of Bielefeld                 **
 **                Contact: nivision@techfak.uni-bielefeld.de       **
 **                Website: www.iclcv.org                           **
 **                                                                 **
 ** File   : include/ICLIO/OpenCVCamGrabber.h                       **
 ** Module : ICLIO                                                  **
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
 *********************************************************************/
#ifndef ICL_OPENCVCAMGRABBER_H
#define ICL_OPENCVCAMGRABBER_H
#include <ICLOpenCV/OpenCV.h>
#include <ICLIO/Grabber.h>
#include <ICLUtils/Exception.h>
#include <highgui.h>
#include <cxcore.h>
namespace icl{

class OpenCVCamGrabber : public Grabber{
private:
	/// Wrapped Device struct
	CvCapture *cvc;
	///number of device
	int device;
	///
	Mutex m_Mutex;
	///Buffer for imagescaling
	ImgBase *scalebuffer;
public:

	/// returns a list of properties, that can be set using setProperty
	/** currently:
	    -size this value needs to be supported from the camera  else
				the next best size is choosen automatically
	    -brightness
	    -contrast
	    -saturation
	    -hue
	    -format
	       -RGB
	   @return list of supported property names **/
	virtual std::vector<std::string> getPropertyList();

	/// get type of property
	/** \copydoc icl::Grabber::getType(const std::string &)*/
	virtual std::string getType(const std::string &name);

	/// get information of a properties valid values values
	/** \copydoc icl::Grabber::getInfo(const std::string &)*/
	virtual std::string getInfo(const std::string &name);

	/// returns the current value of a given property
	/** \copydoc icl::Grabber::getValue(const std::string &)*/
	virtual std::string getValue(const std::string &name);

	/// grab function grabs an image (destination image is adapted on demand)
	/** @copydoc icl::Grabber::grab(ImgBase**) **/
	virtual const ImgBase *grabUD (ImgBase **ppoDst=0);

	/// Sets a property to a new value
	/** call getPropertyList() to see which properties are supported
	    make sure that m__bIgnoreDesiredParams is set to true
	        @copydoc icl::Grabber::setProperty(const std::string&, const std::string&)
	        @param property name of the property
	        @param value new property value
	 */
	virtual void setProperty(const std::string &property, const std::string &value);

	/// Constructor creates a new OpenCVCamGrabber instance from a given device
	/** @param device device to use
	 */
	OpenCVCamGrabber(int dev=0);

	/// Destructor
	~OpenCVCamGrabber();
};

}

#endif /* ICL_OPENCVCAMGRABBER_H */
