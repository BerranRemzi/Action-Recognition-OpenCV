#include "progressBar.h"


progressBar::progressBar()
{
	for (int i = 0; i < 6; i++) {
		sliderValues[i] = 0;
	}
}


progressBar::~progressBar()
{
}

void progressBar::increment(int actionGroup)
{
	if (actionGroup < 6) {
		sliderValues[actionGroup]++;
	}
}

void progressBar::setArray(int * inputArray)
{
	for (int i = 0; i < 6; i++) {
		this->sliderValues[i] = inputArray[i];
	}
}



cv::Mat progressBar::getMat(cv::Mat dst)
{

	cv::Scalar color[4] = { cv::Scalar(0, 0, 200), cv::Scalar(0, 200, 250), cv::Scalar(0, 0, 0), cv::Scalar(255, 255, 255) };
	double value[6];
	bool boolColor;  //color boolean
	double sumOfPredictions = 0;
	int actionPercent[6];
	int textpoint = 12;
	int maxPercentValue = 0;
	
	//load slider values from int array
	for (int i = 0; i < 6; i++) value[i] = (double)this->sliderValues[i];
	
	//Sum of all itterations
	for (int i = 0; i < 6; i++) sumOfPredictions += value[i];
	
	//Calculate percent for every action
	for (int i = 0; i < 6; i++) 
	{ 
		actionPercent[i] = (int)(value[i] / sumOfPredictions * 100);
	}
	
	//find action with highest possibility
	maxPercentValue = actionPercent[0];
	for (int i = 1; i < 6; i++) {
		if (maxPercentValue < actionPercent[i]) {
			maxPercentValue = actionPercent[i];
			highestPercentage = i;
		}
	}

	for (int i = 0; i < 6; i++) {
		if (i == highestPercentage) { boolColor = 1; }
		else { boolColor = 0; };
		putText(dst, action[i], cv::Point(20, textpoint + 8), cv::FONT_HERSHEY_SIMPLEX, 0.5, color[boolColor + 2], boolColor + 1);
		rectangle(dst, cv::Point(100, textpoint), cv::Point(200, textpoint + 10), color[boolColor], 0, 8);
		rectangle(dst, cv::Point(100, textpoint), cv::Point(actionPercent[i] + 100, textpoint + 10), color[boolColor], -1, 8);
		textpoint += 15;
	}

	return dst;

	//return Mat();
}
