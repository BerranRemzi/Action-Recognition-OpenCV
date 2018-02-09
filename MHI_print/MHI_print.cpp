#define _CRT_SECURE_NO_WARNINGS

//#include "stdafx.h"
#include <opencv\cv.h> 
#include <opencv\highgui.h>
#include <opencv\ml.h>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <vector>

using namespace cv;
using namespace std;

char ch[30];
int group = 4;
int images = 1;

Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("FlannBased");
Ptr<DescriptorExtractor> extractor = new SurfDescriptorExtractor(100); //200
SurfFeatureDetector detector(80);

int dictionarySize = 1500;
TermCriteria tc(CV_TERMCRIT_ITER, 10, 0.001);
int retries = 1;
int flags = KMEANS_PP_CENTERS;
BOWKMeansTrainer bowTrainer(dictionarySize, tc, retries, flags);
BOWImgDescriptorExtractor bowDE(extractor, matcher);

CvSVM svm;

IplImage *mhi = 0;
IplImage *orient = 0;
IplImage *mask = 0;
IplImage *segmask = 0;
CvMemStorage* storage = 0;

const double MHI_DURATION = 0.9;
const double MAX_TIME_DELTA = 0.5;
const double MIN_TIME_DELTA = 0.05;
const int N = 2;
IplImage** buf;

int box = 0;
int wave = 0;
int clap = 0;
int run = 0;
int walk = 0;
int jog = 0;

int running = 0;
int clapping = 0;
int last = 0;
int motion_array[5] = { 0, 0, 0, 0, 0 };
int largest = 0;
int hAction = 0;

const char* action[6] = { "Boxing", "Clapping", "Waving", "Jogging", "Running", "Walking" };
void update_mhi(IplImage* img, int diff_threshold)
{
	IplImage* dst = img;
	double timestamp = (double)clock() / CLOCKS_PER_SEC;
	CvSize size = cvSize(img->width, img->height);
	int i, idx1 = last, idx2;
	IplImage* silh;
	CvSeq* seq;
	CvRect comp_rect;
	double count;
	double angle;
	CvPoint center;
	double magnitude;
	CvScalar color;

	if (!mhi || mhi->width != size.width || mhi->height != size.height) {
		if (buf == 0) {
			buf = (IplImage**)malloc(N * sizeof(buf[0]));
			memset(buf, 0, N * sizeof(buf[0]));
		}

		for (i = 0; i < N; i++) {
			cvReleaseImage(&buf[i]);
			buf[i] = cvCreateImage(size, IPL_DEPTH_8U, 1);
			cvZero(buf[i]);
		}
		cvReleaseImage(&mhi);
		cvReleaseImage(&orient);
		cvReleaseImage(&segmask);
		cvReleaseImage(&mask);

		mhi = cvCreateImage(size, IPL_DEPTH_32F, 1);
		cvZero(mhi);
		orient = cvCreateImage(size, IPL_DEPTH_32F, 1);
		segmask = cvCreateImage(size, IPL_DEPTH_32F, 1);
		mask = cvCreateImage(size, IPL_DEPTH_8U, 1);
	}

	cvCvtColor(img, buf[last], CV_BGR2GRAY);

	idx2 = (last + 1) % N;
	last = idx2;

	silh = buf[idx2];
	cvAbsDiff(buf[idx1], buf[idx2], silh);

	cvThreshold(silh, silh, diff_threshold, 1, CV_THRESH_BINARY);
	imshow("idx1", Mat(buf[idx1]));
	imshow("idx2", Mat(buf[idx2]));
	imshow("silh", Mat(silh));
	cvUpdateMotionHistory(silh, mhi, timestamp, MHI_DURATION);
	cvCvtScale(mhi, mask, 255. / MHI_DURATION,
		(MHI_DURATION - timestamp)*255. / MHI_DURATION);
	cvZero(dst);
	cvCvtPlaneToPix(mask, 0, 0, 0, dst);

	cvCalcMotionGradient(mhi, mask, orient, MAX_TIME_DELTA, MIN_TIME_DELTA, 3);

	if (!storage)
		storage = cvCreateMemStorage(0);
	else
		cvClearMemStorage(storage);

	seq = cvSegmentMotion(mhi, segmask, storage, timestamp, MAX_TIME_DELTA);

	for (i = -1; i < seq->total; i++) {

		if (i < 0) {
			comp_rect = cvRect(0, 0, size.width, size.height);
			color = CV_RGB(255, 255, 255);
			magnitude = 100;
		}
		else {
			comp_rect = ((CvConnectedComp*)cvGetSeqElem(seq, i))->rect;
			if (comp_rect.width + comp_rect.height < 100)
				continue;
			color = CV_RGB(255, 0, 0);
			magnitude = 30;
		}

		cvSetImageROI(silh, comp_rect);
		cvSetImageROI(mhi, comp_rect);
		cvSetImageROI(orient, comp_rect);
		cvSetImageROI(mask, comp_rect);

		angle = cvCalcGlobalOrientation(orient, mask, mhi, timestamp, MHI_DURATION);
		angle = 360.0 - angle;
		count = cvNorm(silh, 0, CV_L1, 0);

		cvResetImageROI(mhi);
		cvResetImageROI(orient);
		cvResetImageROI(mask);
		cvResetImageROI(silh);

		if (count < comp_rect.width*comp_rect.height * 0.05)
			continue;

		center = cvPoint((comp_rect.x + comp_rect.width / 2),
			(comp_rect.y + comp_rect.height / 2));

	}
}


Mat percent(Mat dst, int box_slider, int clap_slider, int wave_slider, int jog_slider, int run_slider, int walk_slider) {

	Scalar color[4] = { Scalar(0, 0, 200), Scalar(0, 200, 250), Scalar(0, 0, 0), Scalar(255, 255, 255) };
	double value[6];
	bool c;  //color boolean
	double max = 0;
	int act_per[6];
	int textpoint = 12;
	value[0] = (double)box_slider;
	value[1] = (double)clap_slider;
	value[2] = (double)wave_slider;
	value[3] = (double)jog_slider;
	value[4] = (double)run_slider;
	value[5] = (double)walk_slider;

	for (int i = 0; i < 6; i++) max += value[i];
	for (int i = 0; i < 6; i++) act_per[i] = (int)(value[i] / max * 100);
	for (int i = 0; i < 6; i++) {
		if (i == hAction) { c = 1; }
		else { c = 0; };
		putText(dst, action[i], Point(20, textpoint + 8), FONT_HERSHEY_SIMPLEX, 0.5, color[c + 2], c + 1);
		rectangle(dst, Point(100, textpoint), Point(200, textpoint + 10), color[c], 0, 8);
		rectangle(dst, Point(100, textpoint), Point(act_per[i] + 100, textpoint + 10), color[c], -1, 8);
		textpoint += 15;
	}

	return dst;
}


void collectclasscentroids() {
	IplImage *img;
	int i, j;
	for (j = 1; j <= group; j++)
		for (i = 1; i <= images; i++) {
			sprintf(ch, "%s%d%s%d%s", "train/", j, " (", i, ").jpg");
			printf("%s\n", ch);
			const char* imageName = ch;
			img = cvLoadImage(imageName, 0);
			vector<KeyPoint> keypoint;
			detector.detect(img, keypoint);
			Mat features;
			extractor->compute(img, keypoint, features);
			bowTrainer.add(features);
		}
}



int main(int argc, char* argv[])
{
	CvCapture* capture;
	if (argc == 1) {
		capture = cvCreateCameraCapture(0);
	}
	else {
		capture = cvCreateFileCapture(argv[1]);
	}

	IplImage* motion = 0;
	Mat im_f;

	int check = 1;

	int i, j;
	IplImage *img2;
	//collectclasscentroids();
	vector<Mat> descriptors = bowTrainer.getDescriptors();
	int count = 0;
	for (vector<Mat>::iterator iter = descriptors.begin(); iter != descriptors.end(); iter++)
	{
		count += iter->rows;
	}

	Mat dictionary;

	FileStorage fs("vocab.xml", FileStorage::READ);
	fs["vocabulary"] >> dictionary;
	fs.release();

	bowDE.setVocabulary(dictionary);

	Mat labels(0, 1, CV_32FC1);

	int k = 0;
	vector<KeyPoint> keypoint1;
	Mat bowDescriptor1;

	CvSVMParams params;
	params.kernel_type = CvSVM::RBF;
	params.svm_type = CvSVM::C_SVC;
	params.gamma = 0.50625000000000009;
	params.C = 312.50000000000000;
	params.term_crit = cvTermCriteria(CV_TERMCRIT_ITER, 100, 0.000001);

	svm.load("BoF1.svm");
	Mat groundTruth(0, 1, CV_32FC1);
	Mat evalData(0, dictionarySize, CV_32FC1);
	k = 0;
	vector<KeyPoint> keypoint2;
	Mat bowDescriptor2;

	Mat results(0, 1, CV_32FC1);
	//subtract each row of predicted and real value of label, count errors and subtract with total row count
	double errorRate = (double)countNonZero(groundTruth - results) / evalData.rows;


	IplImage* frame;
	Mat gray;
	while (1) {
		frame = cvQueryFrame(capture);
		if (!frame) break;
		im_f = Mat(frame);
		resize(im_f, im_f, Size(640, 480), 0, 0);
		im_f = percent(im_f, box, clap, wave, jog, run, walk);
		IplImage *dest = cvCreateImage(cvSize(640, 480), frame->depth, frame->nChannels);
		cvResize(frame, dest);
		update_mhi(frame, 30);
		Mat mhifilter = Mat(frame);

		erode(mhifilter, mhifilter, getStructuringElement(MORPH_ELLIPSE, Size(2, 2)));
		dilate(mhifilter, mhifilter, getStructuringElement(MORPH_ELLIPSE, Size(2, 2)));

		//imshow("VideoMHI", mhifilter);
		//imshow("Action Recognition Software", im_f);

		img2 = &((IplImage)mhifilter);

		cvtColor(mhifilter, gray, CV_RGB2GRAY);
		imshow("Gray", gray);

		int zero = countNonZero(gray);

		char key = (char)cv::waitKey(30);
		if (key == 'p') {
			static int imageNumber;
			imageNumber++;
			char filename[30];
			sprintf(filename, "%s%d%s","(", imageNumber, ").jpg");
			
			IplImage image = IplImage(gray);
			const char* imageName = filename;

			cvSaveImage(imageName, &image);
			
			imwrite(imageName, gray);
			//cout << imageNumber << " saved" << endl;
		}
		if (key == 'q') return 0;

		//if (k == 'ESC') return 0;
		/*if (k == 'p') {
			static char filename[30];
			static int j, i;
			i++;
			sprintf(filename, "%s%d%s%d%s", "train/", j, " (", i, ").jpg");
			imwrite(filename, gray);

		}*/

	}
	printf("Box = %d, Clap = %d, Wave = %d, Jog = %d, Run = %d, Walk=%d\n", box, clapping, wave, jog, running, walk);
	return hAction;

}
