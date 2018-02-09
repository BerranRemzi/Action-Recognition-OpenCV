#include <opencv\cv.h> 
#include <opencv\highgui.h>
#include <opencv\ml.h>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <vector>

using namespace cv;
using namespace std;

char ch[30];

//--------Using SURF as feature extractor and FlannBased for assigning a new point to the nearest one in the dictionary
Ptr<DescriptorMatcher> matcher = DescriptorMatcher::create("FlannBased");
Ptr<DescriptorExtractor> extractor = new SurfDescriptorExtractor();
SurfFeatureDetector detector(500);
//---dictionary size=number of cluster's centroids
int dictionarySize = 1500;
TermCriteria tc(CV_TERMCRIT_ITER, 10, 0.001);
int retries = 1;
int flags = KMEANS_PP_CENTERS;
BOWKMeansTrainer bowTrainer(dictionarySize, tc, retries, flags);
BOWImgDescriptorExtractor bowDE(extractor, matcher);



void collectclasscentroids() {
	IplImage *img;
	int i, j;
	for (j = 1; j <= 4; j++)
		for (i = 1; i <= 60; i++) {
			sprintf(ch, "%s%d%s%d%s", "train/", j, " (", i, ").jpg");
			const char* imageName = ch;
			img = cvLoadImage(imageName, 0);
			vector<KeyPoint> keypoint;
			detector.detect(img, keypoint);
			Mat features;
			extractor->compute(img, keypoint, features);
			bowTrainer.add(features);
		}
	return;
}
int main(int argc, char* argv[])
{

	int i, j;
	IplImage *img2;
	cout << "Vector quantization..." << endl;
	collectclasscentroids();
	vector<Mat> descriptors = bowTrainer.getDescriptors();
	int count = 0;
	for (vector<Mat>::iterator iter = descriptors.begin(); iter != descriptors.end(); iter++)
	{
		count += iter->rows;
	}
	cout << "Clustering " << count << " features" << endl;
	//choosing cluster's centroids as dictionary's words
	Mat dictionary = bowTrainer.cluster();
	bowDE.setVocabulary(dictionary);
	cout << "extracting histograms in the form of BOW for each image " << endl;
	Mat labels(0, 1, CV_32FC1);
	Mat trainingData(0, dictionarySize, CV_32FC1);
	int k = 0;
	vector<KeyPoint> keypoint1;
	Mat bowDescriptor1;
	//extracting histogram in the form of bow for each image 
	for (j = 1; j <= 4; j++)
		for (i = 1; i <= 60; i++)
		{
			sprintf(ch, "%s%d%s%d%s", "train/", j, " (", i, ").jpg");
			const char* imageName = ch;
			img2 = cvLoadImage(imageName, 0);
			detector.detect(img2, keypoint1);
			bowDE.compute(img2, keypoint1, bowDescriptor1);
			trainingData.push_back(bowDescriptor1);
			labels.push_back((float)j);
		}
	//Setting up SVM parameters
	CvSVMParams params;
	params.kernel_type = CvSVM::RBF;
	params.svm_type = CvSVM::C_SVC;
	params.gamma = 0.50625000000000009;
	params.C = 312.50000000000000;
	params.term_crit = cvTermCriteria(CV_TERMCRIT_ITER, 100, 0.000001);
	CvSVM svm;



	printf("%s\n", "Training SVM classifier");

	bool res = svm.train(trainingData, labels, cv::Mat(), cv::Mat(), params);

	cout << "Processing evaluation data..." << endl;


	Mat groundTruth(0, 1, CV_32FC1);
	Mat evalData(0, dictionarySize, CV_32FC1);
	k = 0;
	vector<KeyPoint> keypoint2;
	Mat bowDescriptor2;


	Mat results(0, 1, CV_32FC1);;
	for (j = 1; j <= 4; j++)
		for (i = 1; i <= 60; i++)
		{
			sprintf(ch, "%s%d%s%d%s", "eval/", j, " (", i, ").jpg");
			const char* imageName = ch;
			img2 = cvLoadImage(imageName, 0);
			detector.detect(img2, keypoint2);
			bowDE.compute(img2, keypoint2, bowDescriptor2);
			evalData.push_back(bowDescriptor2);
			groundTruth.push_back((float)j);
			float response = svm.predict(bowDescriptor2);
			results.push_back(response);
		}



	//calculate the number of unmatched classes 
	double errorRate = (double)countNonZero(groundTruth - results) / evalData.rows;
	cout << "Error rate = " << errorRate << endl;

	return 0;
}