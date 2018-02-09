#define _CRT_SECURE_NO_WARNINGS

//#include "stdafx.h"
#include <opencv\cv.h> 
#include <opencv\highgui.h>
#include <opencv\ml.h>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <vector>
#include "MHI.h"
#include "progressBar.h"

using namespace cv;
using namespace std;

void saveImage(Mat* inputImage) {
	static int imageNumber;
	imageNumber++; 
	char filename[30];
	sprintf(filename, "%s%d%s", "(", imageNumber, ").jpg");

	//IplImage image = IplImage(inputImage);
	const char* imageName = filename;

	cvSaveImage(imageName, &inputImage);

	//imwrite(imageName, inputImage);
}

//const char* action[6] = { "Boxing", "Clapping", "Waving", "Jogging", "Running", "Walking" };

int main(int argc, char* argv[])
{
	MHI motionImage;
	progressBar percent;

	CvCapture* capture;
	if (argc == 1) {
		capture = cvCreateCameraCapture(0);
	}
	else {
		capture = cvCreateFileCapture(argv[1]);
	}

	IplImage *inputFrame;
	Mat inputMat;
	Mat gray;
	while (1) {
		inputFrame = cvQueryFrame(capture);
		if (!inputFrame) break;
		
		inputMat = Mat(inputFrame, true);
		

		motionImage.update_mhi(inputFrame, 30);
		resize(inputMat, inputMat, Size(640, 480), 0, 0);

		inputMat = percent.getMat(inputMat);

		cvtColor(motionImage.blur(), gray, CV_RGB2GRAY);

		namedWindow("Display window", CV_WINDOW_AUTOSIZE);// Create a window for display.
		imshow("Display window", inputMat);
		waitKey(30);
		imshow("Gray", gray);

		int zero = countNonZero(gray);

		//char key = (char)WaitKey(30);

		switch (waitKey(30)) {
		case 'q':	return 0; break;
		case 'ESC': return 0; break;
		case 'p':	saveImage(&gray); break;
		case '1':	percent.increment(0); break;
		case '2':	percent.increment(1); break;
		case '3':	percent.increment(2); break;
		case '4':	percent.increment(3); break;
		case '5':	percent.increment(4); break;
		case '6':	percent.increment(5); break;
		default:	break;
		}
	}

	return 0;

}
