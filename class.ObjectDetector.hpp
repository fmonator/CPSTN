#pragma once
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include "entities.hpp"
#include "class.Drawer.hpp"
#include "class.ThresholdColor.hpp"
#include "util.h"

using namespace cv;
using namespace std;

class ObjectDetector {
private:
	static Size WIN_SIZE;
	class BinInfo {
	public:
		ThresholdColor* color;
		int count;
		DetectedObjectType type;

		BinInfo(DetectedObjectType type, ThresholdColor* color) {
			this->type = type;
			this->color = color;
		}

		bool operator< (const BinInfo &b) const {
			return this->count < b.count;
		}
	};

	// Filtruj kontury (Filter contours)
	int MIN_PIXELS_IN_CONTOUR; 
	int MIN_AREA;
	int MAX_AREA;
	Rect BANNER_AREA;
	float VOLUME_BANNER;
	float MIN_COLOR_VOLUME;
	vector<BinInfo> histogram;
	
	// Running average count of player color count
	double averageTeamA;
	unsigned int teamACount;
	double averageTeamB;
	unsigned int teamBCount;

	void determinePerson(Mat& image, Mat& mask, FrameObject* obj, int top, int bot);
	void determineObject(Mat& image, Mat& mask, FrameObject* obj, int top, int bot);
	void computeHistogram(Mat& ROI);

public:

	ObjectDetector() {
		
		MIN_PIXELS_IN_CONTOUR = 15;// 
		MIN_AREA = 175; // 868 velksot hraca (size hraca)
		MAX_AREA = 5000;
		BANNER_AREA = Rect(0, 0, 640, 18);
		VOLUME_BANNER = 0.8f;
		MIN_COLOR_VOLUME = 0.1;

		// Running average variables
		averageTeamA = 0;
		teamACount = 0;
		averageTeamB = 0;
		teamBCount = 0;

		// where is GOAL_KEEPER_B ?
		// SO it looks like this just takes a range of colors which the object is within
		histogram.push_back(BinInfo(REFEREE, new ThresholdColor(Scalar(0, 127, 81), Scalar(12, 219, 255)))); // green to blue
		histogram.push_back(BinInfo(PLAYER_A, new ThresholdColor(Scalar(110, 67, 48), Scalar(141, 150, 158)))); // brown to gray
		histogram.push_back(BinInfo(PLAYER_B, new ThresholdColor(Scalar(0, 0, 245), Scalar(180, 255, 255)))); // blue to light blue
		histogram.push_back(BinInfo(GOAL_KEEPER_A, new ThresholdColor(Scalar(21, 90, 0), Scalar(52, 190, 255)))); // green to blue
	}

	void findObjects(Mat& image, Mat& mask, vector<FrameObject*>& objects, vector<FrameObject*>& ta, vector<FrameObject*>& tb, FrameObject* ball, int top, int bot);
};