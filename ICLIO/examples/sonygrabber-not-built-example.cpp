/********************************************************************
**                Image Component Library (ICL)                    **
**                                                                 **
** Copyright (C) 2006-2010 CITEC, University of Bielefeld          **
**                         Neuroinformatics Group                  **
** Website: www.iclcv.org and                                      **
**          http://opensource.cit-ec.de/projects/icl               **
**                                                                 **
** File   : ICLIO/examples/sonygrabber-not-built-example.cpp       **
** Module : ICLIO                                                  **
** Authors: Christof Elbrechter                                    **
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

#include <ICLIO/FileWriter.h>
#include <ICLIO/SonyFwGrabber.h>

using namespace std;
using namespace icl;

int main(int n, char **ppc) {
#ifdef WIN32
#ifdef WITH_SONYIIDC
	SonyFwGrabber *grabber = new SonyFwGrabber();
	if (!grabber->init()) {
		cout << "init failed!" << endl;
		Sleep(5000);
	}

	FileWriter writer("sony_bayer.pgm");

	const ImgBase* img = grabber->grab();
	writer.write(img);

	ImgBase *left, *right;
	left = 0;
	right = 0;
	
	grabber->grabStereo(&left, &right);
	writer.setFileName("left_0.pgm");
	writer.write(left);
	writer.setFileName("right_0.pgm");
	writer.write(right);

	grabber->grabStereo(&left, &right);
	writer.setFileName("left_1.pgm");
	writer.write(left);
	writer.setFileName("right_1.pgm");
	writer.write(right);

	delete grabber;

	return 0;
#endif
#endif

}

