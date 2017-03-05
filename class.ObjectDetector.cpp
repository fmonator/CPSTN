#include "class.ObjectDetector.hpp"
#include <array>

void ObjectDetector::computeHistogram(Mat& ROI) {
	for ( auto it = histogram.begin(); it != histogram.end(); ++it ) {
		it->count = it->color->pixels(ROI);
	}
	sort(histogram.begin(), histogram.end());
}

void ObjectDetector::determinePerson(Mat& image, Mat& mask, FrameObject* obj, int top_bound, int bot_bound) {
	// Person out of playground
	if(isRelativeIntersection(obj->m_countour, BANNER_AREA, VOLUME_BANNER)) {
		obj->type = DetectedObjectType::PERSON;
		return;
	}

	// Prepare data for detecting players
	Mat combinedImageMask;
	image.copyTo(combinedImageMask, mask); 
	Mat ROI = obj->getROI(combinedImageMask);		

	// Count pixels for specific colors
	int pixelsInROI = countNonZero( obj->getROI(mask) );
	computeHistogram(ROI);
	BinInfo info = histogram.back();
	BinInfo info2 = histogram[histogram.size()-2];
	if(info.count < (pixelsInROI * MIN_COLOR_VOLUME)) {
		obj->type = DetectedObjectType::PERSON; // reprezentacia pixelov je velmi mala
		// representation of pixels is very small
		return;
	}
	obj->count = info.count;
	if(abs(info.count-info2.count) < 50) {
		if((info.type == DetectedObjectType::PLAYER_A && info2.type == DetectedObjectType::PLAYER_B) || (info.type == DetectedObjectType::PLAYER_B && info2.type == DetectedObjectType::PLAYER_A)) {
			obj->type = DetectedObjectType::CONFUSED_AB;
			return;
		}
	}

	obj->type = info.type; // prirad typ z farby (Assign the type of paint)
	int min_count = 100;
	//double whr_var = 
	int adj_count = info.count / mapRange(top_bound,bot_bound,1,3.5,obj->m_boundary.center.y);
	double area = obj->m_boundary.boundingRect().area();
	obj->count = adj_count;
	double scale_factor = 1.9;
	if(area < 1000 * mapRange(top_bound,bot_bound,0.8,2.5,obj->m_boundary.center.y)) return;
	else {
		if(obj->type == PLAYER_A) obj->type = CONFUSED_A;
		else if(obj->type == PLAYER_B) obj->type = CONFUSED_B;
		else obj->type = CONFUSED;
	}
	return;
	if(obj->type == DetectedObjectType::PLAYER_A) {
		if(teamACount > min_count && (adj_count > (int)(scale_factor*averageTeamA))) {
			obj->type = DetectedObjectType::CONFUSED_A;
		} else {
			averageArea = (averageArea*(teamACount+teamBCount) + 	area) / (teamACount+teamBCount+1);
			averageTeamA = (averageTeamA*teamACount + adj_count) / (teamACount+1);
			++teamACount;
			//cout << "updating for team a average: " << teamACount << endl;
		}
	} else if(obj->type == DetectedObjectType::PLAYER_B) {
		if(teamBCount > min_count && (adj_count > (int)(scale_factor*averageTeamB))) {
			obj->type = DetectedObjectType::CONFUSED_B;
		} else {
			averageArea = (averageArea*(teamACount+teamBCount) + 	area) / (teamACount+teamBCount+1);
			//cout << "updating for team b average: " << averageTeamB << endl;
			averageTeamB = (averageTeamB*teamBCount + adj_count) / (teamBCount+1);
			++teamBCount;
		}
	}
}

void ObjectDetector::determineObject(Mat& image, Mat& mask, FrameObject* obj, int top_bound, int bot_bound) {
	// Artefakt
	if( obj->pixels() < MIN_PIXELS_IN_CONTOUR) { 
		obj->type = ARTEFACT;
		return;
	}
	float takeSpace = obj->getSpace();
	if(takeSpace < MIN_AREA) {
		obj->type = ARTEFACT;
		double wh_ratio = obj->m_boundary.boundingRect().width / obj->m_boundary.boundingRect().height;
		bool has_bounding_square = wh_ratio > 0.9 && wh_ratio < 1.1;
		if(takeSpace > 20 && has_bounding_square) {
			determinePerson(image, mask, obj, top_bound, bot_bound);
			if(obj->type == PLAYER_B) obj->type = BALL;
			else obj->type = ARTEFACT;
		}
		return;
	}

	// Banner
	if(takeSpace > MAX_AREA) {
		obj->type = BANNER;
		return;
	}

	// Is it person?
	determinePerson(image, mask, obj, top_bound, bot_bound);
}

void ObjectDetector::findObjects(Mat& image, Mat& mask, vector<FrameObject*>& objects, vector<FrameObject*>& ta, vector<FrameObject*>& tb, int top_bound, int bot_bound) {		
	vector<vector<Point>> contours;
	findContours(mask, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE); // Vyber kontury (Select contours)

	for( unsigned int i = 0; i < contours.size(); i++ ) { 
		vector<Point> contour = contours[i];
		if(contour.size() < 5) {
			continue;
		}
		FrameObject* obj = new FrameObject(contour, fitEllipse( Mat(contour) ));
		if(obj->m_boundary.center.y < top_bound || obj->m_boundary.center.y > bot_bound || obj->m_boundary.center.x < 50) {
			delete obj;
			continue;
		}

		objects.push_back(obj);
		determineObject(image, mask, obj, top_bound, bot_bound);
		
		if(obj->type == PLAYER_A || obj->type == GOAL_KEEPER_A) {
			ta.push_back(obj);
		} else if(obj->type == PLAYER_B || obj->type == GOAL_KEEPER_B || obj->type == PERSON) {
			tb.push_back(obj);
		} else if(obj->type == CONFUSED_A) {
			int width = obj->m_boundary.boundingRect().width;
			int height = obj->m_boundary.boundingRect().height;
			Point center = obj->m_boundary.center;
			double wh_ratio =  width / height;
			FrameObject *p1, *p2;
			if(wh_ratio > 1) {
				Size psize(width/2, height);
				p1 = new FrameObject(vector<Point>(), RotatedRect(center + Point(width/4,0), psize, obj->m_boundary.angle));
				p2 = new FrameObject(vector<Point>(), RotatedRect(center - Point(width/4,0), psize, obj->m_boundary.angle));
			} else {
				Size psize(width, height/2);
				p1 = new FrameObject(vector<Point>(), RotatedRect(center + Point(0,height/4), psize, obj->m_boundary.angle));
				p2 = new FrameObject(vector<Point>(), RotatedRect(center - Point(0,height/4), psize, obj->m_boundary.angle));
			}
			p1->type = PLAYER_A;
			p2->type = PLAYER_A;
			ta.push_back(p1);
			ta.push_back(p2);
		} else if(obj->type == CONFUSED_B) {
			int width = obj->m_boundary.boundingRect().width;
			int height = obj->m_boundary.boundingRect().height;
			Point center = obj->m_boundary.center;
			double wh_ratio =  width / height;
			FrameObject *p1, *p2;
			if(wh_ratio > 1) {
				Size psize(width/2, height);
				p1 = new FrameObject(vector<Point>(), RotatedRect(center + Point(width/4,0), psize, obj->m_boundary.angle));
				p2 = new FrameObject(vector<Point>(), RotatedRect(center - Point(width/4,0), psize, obj->m_boundary.angle));
			} else {
				Size psize(width, height/2);
				p1 = new FrameObject(vector<Point>(), RotatedRect(center + Point(0,height/4), psize, obj->m_boundary.angle));
				p2 = new FrameObject(vector<Point>(), RotatedRect(center - Point(0,height/4), psize, obj->m_boundary.angle));
			}
			p1->type = PLAYER_B;
			p2->type = PLAYER_B;
			tb.push_back(p1);
			tb.push_back(p2);
		}
	}
}

Size ObjectDetector::WIN_SIZE = Size(640, 480); 
