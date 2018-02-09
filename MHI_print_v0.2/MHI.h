#pragma once

#include <opencv\cv.h> 
#include <opencv\highgui.h>
#include <opencv\ml.h>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <vector>

class MHI
{
public:
	MHI();
	MHI(double MHI_DURATION_input, double MAX_TIME_DELTA_input, double MIN_TIME_DELTA_input, int N_input);
	void update_mhi(IplImage* img, int diff_threshold);
	cv::Mat frameMat();
	cv::Mat blur();
	~MHI();
private:
	IplImage * *buf;
	IplImage *dst = 0;
	int last = 0;

	IplImage *mhi = 0;
	IplImage *orient = 0;
	IplImage *mask = 0;
	IplImage *segmask = 0;
	CvMemStorage *storage = 0;

	double MHI_DURATION = 0.9;
	double MAX_TIME_DELTA = 0.5;
	double MIN_TIME_DELTA = 0.05;
	int N = 2;

};

