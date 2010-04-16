/********************************************************************
 **                Image Component Library (ICL)                    **
 **                                                                 **
 ** Copyright (C) 2006-2010 Neuroinformatics, CITEC                 **
 **                         University of Bielefeld                 **
 **                Contact: nivision@techfak.uni-bielefeld.de       **
 **                Website: www.iclcv.org                           **
 **                                                                 **
 ** File   : include/ICLIO/OpenCVVideoGrabber.h                     **
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
#ifndef ICL_OPENCVVIDEOGRABBER_H
#define ICL_OPENCVVIDEOGRABBER_H

#include <ICLOpenCV/OpenCV.h>
#include <ICLIO/Grabber.h>
#include <ICLUtils/FPSLimiter.h>
#include <ICLUtils/Exception.h>
#include <highgui.h>
#include <string>
#include <ICLIO/File.h>
namespace icl{

class OpenCVVideoGrabber : public Grabber{
private:
	/// Device struct
	CvCapture *cvc;
	///Filename of the file
	std::string filename;
	///Ensures the desired framerate
	FPSLimiter *fpslimiter;
	///Buffer for scaling if necessary
	ImgBase *scalebuffer;
	///
	bool use_video_fps;
public:

	/// returns a list of properties, that can be set using setProperty
	/**currently:
	  -pos_msec current position in file in msec
	  -pos_frames current frame
	  -pos_avi_ratio current position in file in %
	  -size size of frame in file
	  -format
	    -RGB
	  -fps frames per second
	  -fourcc the videocodecidentifier of current file
	  -frame_count framecount
	  -use_video_fps forces to use fps from file
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
	        @copydoc icl::Grabber::setProperty(const std::string&, const std::string&)
	        @param property name of the property
	        @param value new property value
	 */
	virtual void setProperty(const std::string &property, const std::string &value);

	/// Returns whether this property may be changed internally
	/** For example a video grabber's current stream position. This can be changed
	    from outside, but it is changed when the stream is played. The isVolatile
	    function should return a msec-value that describes how often the corresponding
	    feature might be updated internally or just 0, if the corresponding
	    feature is not volatile at all. The default implementation of isVolatile
	    returns 0 for all features. So if there is no such feature in your grabber,
	    this function must not be adapted at all. "info"-typed Properties might be
	    volatile as well */
	virtual int isVolatile(const std::string &propertyName);

	/// Constructor creates a new OpenCVVideoGrabber instance
	/** @param filename name of file to use */
	OpenCVVideoGrabber(const std::string &fileName) throw (FileNotFoundException);

	/// Destructor
	~OpenCVVideoGrabber();
};

}
#endif /* ICL_OPENCVVIDEOGRABBER_H */
