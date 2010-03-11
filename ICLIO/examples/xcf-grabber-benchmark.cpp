/*************************************************************************** 
**                                                                        **
** Copyright (C) 2006-2010 neuroinformatics group (vision)                **
**                         University of Bielefeld                        **
**                         nivision@techfak.uni-bielefeld.de              **
**                                                                        **
** This file is part of the ICLIO module of ICL                           **
**                                                                        **
** Commercial License                                                     **
** Commercial usage of ICL is possible and must be negotiated with us.    **
** See our website www.iclcv.org for more details                         **
**                                                                        **
** GNU General Public License Usage                                       **
** Alternatively, this file may be used under the terms of the GNU        **
** General Public License version 3.0 as published by the Free Software   **
** Foundation and appearing in the file LICENSE.GPL included in the       **
** packaging of this file.  Please review the following information to    **
** ensure the GNU General Public License version 3.0 requirements will be **
** met: http://www.gnu.org/copyleft/gpl.html.                             **
**                                                                        **
***************************************************************************/ 

#ifdef HAVE_XCF
#include <vector>
#include <ICLIO/XCFServerGrabber.h>
#ifdef WIN32
#include <log4cxx/PropertyConfigurator.h>
#endif
using namespace icl;
#endif

int main() {
#ifdef HAVE_XCF
#ifdef WIN32
	log4cxx::PropertyConfigurator::configure(std::string("log4cxx.props"));
#endif
	XCFServerGrabber *grabber = new XCFServerGrabber("XCFSonyFwImageServer");
	grabber->setRequest("<IMAGEREQUEST>"
                    	"<GRAB stereo=\"true\" timestamp=\"\"/>"
                    	"</IMAGEREQUEST>");

	std::vector<ImgBase*> imgs;
	imgs.resize(2);
	imgs[0] = 0;
	imgs[1] = 0;

	Time before = Time::now();
	for (int i=0; i<200; i++) {
		grabber->grab(imgs);
	}
	Time dur = before.age();
	std::cout << "Grabbed 200 images in " << dur.toSeconds() << " seconds!" << std::endl;
	std::cout << 200.0 / dur.toSeconds() << " hz" << std::endl;

#ifdef WIN32
	Sleep(1000);
#endif

	delete imgs[0];
	delete imgs[1];
	delete grabber;
#endif
	return 0;
}
