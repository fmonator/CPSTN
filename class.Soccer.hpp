#pragma once
#include <unordered_map>
#include "class.App.hpp"
#include "log4cpp.h"
#include "entities.hpp"
#include "class.VideoRecord.hpp"
#include <opencv2/video/background_segm.hpp>
#include "class.ThresholdColor.hpp"
#include "class.ObjectDetector.hpp"
#include "class.Drawer.hpp"
#include "class.ObjectTracer.hpp"

class Soccer : public App
{
private:
	// Atributy aplikacie (application attributes)
	log4cpp::Category* log;
	static Size WIN_SIZE;
	void commandArrive(string& str);
	vector<Vec2f> lines; // for determining gameplay area and camera angle

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
	ThresholdColor* m_grass;

	ObjectTracer* m_tracer;

	// Field lines
	int fline_top;
	int fline_bot;

	vector<FrameObject*> last_frame;
	vector<FrameObject*> filter(vector<FrameObject*>&);
	void mapToLast(vector<FrameObject*>&);

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
