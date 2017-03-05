#include "class.Drawer.hpp"
#include "util.h"

Drawer::Drawer() {
	m_roi_index = 0;
	m_roiDraw = false;
	m_debugDraw = false;
	m_teamColoring = false;
	m_drawType = 2;
	m_roi = new ThresholdColor(Scalar(35, 72, 50), Scalar(51, 142, 144));
	m_roi->createTrackBars("roiMask");

	log = CREATE_log4CPP();
	if(log != NULL) {
		log->debug("Starting drawer");
	}
}
Drawer::~Drawer() {
	SAFE_DELETE(m_roi);
}

void Drawer::switchTeamColoring() {
	m_teamColoring = !m_teamColoring; 
	log->debugStream() << "m_teamColoring " << m_teamColoring;
}
void Drawer::switchDrawType() {
	m_drawType = (m_drawType + 1) % 5; 
	log->debugStream() << "m_drawType " << m_drawType;
}
void Drawer::switchDebugDraw() {
	m_debugDraw = !m_debugDraw; 
	log->debugStream() << "m_debugDraw " << m_debugDraw;
}
void Drawer::switchROIDraw() {
	m_roiDraw = !m_roiDraw;
	log->debugStream() << "m_roiDraw " << m_roiDraw;
}
void Drawer::nextROI() {
	m_roi_index++;
	log->debugStream() << "next roi ";
}
void Drawer::previousROI() {
	m_roi_index--;
	log->debugStream() << "previous roi ";
}

void Drawer::draw(Mat& image, Mat& mask, vector<FrameObject*>& objs, int top, int bot) {
	if(m_roiDraw) {
		drawROI(image, mask, objs);
	}

	// Vykresli detekovane oblasti (Portrays the detection area)
	for( UINT i = 0; i < objs.size(); i++ ) { 
		FrameObject* obj = objs.at(i);
		if(!isVisible(obj)) {
			continue;
		}

		m_drawType = 5;
		Scalar color = determineColor(obj); 
		if(m_drawType == 0) {
			ellipse(image, objs.at(i)->m_boundary, color, 1);
		} else if(m_drawType == 2) {
			ostringstream os;
			os << obj->type;
			putText(image, os.str(), obj->m_boundary.center, 0, 0.2, color, 1, 8);
		} else if(m_drawType == 3) {
			ostringstream os;
			os << obj->m_boundary.boundingRect().area();
			putText(image, os.str(), obj->m_boundary.center, 0, 0.8, Scalar(0, 0, 0), 1, 8);
		} else if(m_drawType == 4) {
			ostringstream os;
			os << obj->distanceCovered();
			putText(image, os.str(), obj->m_boundary.center, 0, 0.8, Scalar(255, 255, 255), 1, 8);
		} else if(m_drawType == 1) {
			drawPoints(image, objs.at(i)->m_countour, color);
		} else if(m_drawType == 5) {
			ostringstream os;
			os << obj->type;
			string label = to_string(obj->m_boundary.boundingRect().area());
			if(obj->type == CONFUSED) putText(image, os.str(), obj->m_boundary.center, 0, 0.8, color, 1, 8);
			//ellipse(image, objs.at(i)->m_boundary, color, 1);
			rectangle(image,obj->m_boundary.boundingRect(),color);
		}
	}

	for(int i = top; i < bot; i += 20) {
		ostringstream os;
		os << 1000 * mapRange(top,bot,0.8,2.5,i);
		putText(image, os.str(), Point(10,i), 0, 0.8, Scalar(0,0,0), 1, 8);
	}
	imshow("Output", image);
}

bool Drawer::isVisible(FrameObject* obj) {
	if((obj->type == ARTEFACT || obj->type == BANNER )) {
		return m_debugDraw;
	}
	return true;
}

Scalar Drawer::determineColor(FrameObject* obj) {
	// bgr
	if(obj->type == ARTEFACT) {
		return Scalar(0, 0, 255);
	}
	if(obj->type == BANNER) {
		return Scalar(0, 0, 255);
	}
	if(obj->type == BALL) {
		return Scalar(255, 0, 255);
	}
	if(obj->type == PERSON) {
		return Scalar(60, 60, 60);
	}
	if(obj->type == REFEREE) {
		return Scalar(0, 0, 255);
	}
	if(m_teamColoring && obj->type == GOAL_KEEPER_A) { // Should this be GOAL_KEEPER_B ?
		return Scalar(255, 255, 255); // same color as PLAYER_B
	}
	if(obj->type == GOAL_KEEPER_A) {
		return Scalar(0, 220, 220);
	} // above and below this line have different colors though
	if(obj->type == PLAYER_A) {
		return Scalar(255, 0, 0);
	}
	if(obj->type == PLAYER_B) {
		return Scalar(255, 255, 255);
	}
	return Scalar(0, 0, 0);
}

void Drawer::drawROI(Mat& image, Mat& mask, vector<FrameObject*>& objs) {
	// Find ROI or select main image
	Mat ROI;
	UINT size = objs.size();
	if(size > 0) {
		Mat combinedImageMask;
		image.copyTo(combinedImageMask, mask); 
		ROI = objs.at( m_roi_index % size )->getROI(combinedImageMask);		

		//Mat roiMask = m_roi->getMask(ROI);
		//resize(roiMask, roiMask, winSize);
		//imshow("roiMask", roiMask);
		//Mat histogram = computeHistogram(labColor);
		//resize(histogram, histogram, m_winSize);
		//imshow("Histogram HSV",  computeHistogram(labColor) );
		//Mat labels = computeClusters(ROI);
		//resize(labels, labels, m_winSize);
		//imshow("result", labels);	
	} else {
		ROI = Mat(image);
	}
	resize(ROI, ROI, WIN_SIZE);
	imshow("Selected section", ROI);
}

Size Drawer::WIN_SIZE = Size(640, 480); 

// Vybrana sekcia == selected section