#pragma once
#include <unordered_map>
#include "class.App.hpp"
#include "log4cpp.h"
#include "entities.hpp"
#include "class.VideoRecord.hpp"
#include <opencv2/video/background_segm.hpp>
#include "class.ThresholdColor.hpp"
#include "class.ObjectDetector.hpp"
//#include "class.Drawer.hpp"
#include "class.ObjectTracer.hpp"

#include "Poco/Net/HTTPClientSession.h"
#include "Poco/Net/HTTPRequest.h"
#include "Poco/Net/HTTPResponse.h"
#include <Poco/Net/HTTPCredentials.h>
#include "Poco/StreamCopier.h"
#include "Poco/NullStream.h"
#include "Poco/Path.h"
#include "Poco/URI.h"
#include "Poco/Exception.h"
#include "Poco/JSON/JSON.h"
#include "Poco/JSON/Object.h"
#include "Poco/JSON/Stringifier.h"

class Soccer : public App
{
private:
	static bool teamBAttacking;
	static bool pastTheIssue;
	static int consecutiveOffsides;
	static FrameObject *lastBall;
	static Mat warpMatrix;
	struct SlopeLine { // Given in y = slope*x + intercept form, rearranged for x = (y - intercept)/slope
						// put in a y-value to find the field limit in x
	public: 
		double slope, intercept;

		bool isThisTooFar(Point p) { // takes in a point (up to you to decide which point on object)
			int x_limit = (p.y - intercept)/slope;

			if (teamBAttacking == false) // i.e. goalLine is on the right side of the image
				return (p.x > x_limit);
			else						// i.e. goalLine is on the left side of the image
				return (p.x < x_limit);
		}

		bool isThisTooFar(Point2f p) { // takes in a point (up to you to decide which point on object) 
			int x_limit = (p.y - intercept)/slope;

			if (teamBAttacking == false) // i.e. goalLine is on the right side of the image
				return (p.x > x_limit);
			else						// i.e. goalLine is on the left side of the image
				return (p.x < x_limit);
		}
	};

	// Atributy aplikacie (application attributes)
	SlopeLine goalLine;
	log4cpp::Category* log;
	static Size WIN_SIZE;
	void commandArrive(string& str);
	vector<Vec2f> lines; // for determining gameplay area and camera angle

	// Networking/offside
	bool notifyRef();
	bool checkOffside(vector<FrameObject*>teamA,vector<FrameObject*>teamB,FrameObject* ball);
	void vecSort(vector<Point2f> &a, vector<Point2f> &b);

	// Spracovanie videa (video processing)
	bool m_pause;
	VideoRecord* m_record;
	Frame* m_actual;
	void processFrame(Frame* image); 
	void loadNextFrame(); 

	// Mog attributes
	Ptr<BackgroundSubtractor> m_pMOG2; //MOG2 Background subtractor
	int m_mogLearnFrames;
	bool m_learning;
	void learningEnd();

	// Determine objects
	ObjectDetector* m_detector;
	Drawer* m_drawer;
	void processImage(Mat& input);
	Mat getWarpMatrix();
	ThresholdColor* m_grass;

	ObjectTracer* m_tracer;

	// Field lines
	int fline_top;
	int fline_bot;

protected:
	// Inicializacia programu (program initialization)
	virtual void Init();

	// Run() metoda je vola v cykle dokedy bezi apliakcia
	// (Run() method is called in the cycle Until application runs)
	virtual bool Run();
	virtual int getLockFPS();

public:
	Soccer();
	~Soccer();
};
