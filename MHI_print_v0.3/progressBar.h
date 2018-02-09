//#include <stdio.h>
#include <opencv\cv.h> 
#include <opencv\highgui.h>
#include <opencv\ml.h>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <vector>

class progressBar
{
public:
	progressBar();
	~progressBar();
	void increment(int actionGroup);
	void setArray(int *inputArray);
	cv::Mat getMat(cv::Mat dst);

private:
	std::string action[6] = { "Boxing", "Clapping", "Waving", "Jogging", "Running", "Walking" };
	int highestPercentage = 0;
	int sliderValues[6];
};
