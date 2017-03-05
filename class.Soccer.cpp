#include "class.Soccer.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include "util.h"

bool Soccer::teamBAttacking; // static intitialization
Mat Soccer::warpMatrix; // static initialization

Soccer::Soccer() 
{
	// Nacitaj vsetky casti Soccera (Load all parts Soccer)
	log = CREATE_log4CPP();
	if(log != NULL) {
		log->debug("Starting Soccer");
	}

	// Field lines defaults
	fline_bot = INT_MAX;
	fline_top = 0;
}

Soccer::~Soccer() {
	SAFE_DELETE(m_record);
	SAFE_DELETE(m_pMOG2);
	SAFE_DELETE(m_detector);
	SAFE_DELETE(m_drawer);
	SAFE_DELETE(m_grass);
	SAFE_RELEASE(m_actual);
	SAFE_DELETE(m_tracer);
	if(log != NULL) log->debug("Ending Soccer");
}

int Soccer::getLockFPS() {
	return 60;
}

bool Soccer::Run() {
	int key = cv::waitKey(15);
	string str;  
	str =(char) key;
	commandArrive(str);
	if(!m_pause) {
		loadNextFrame();
	}
	if(m_actual != NULL) {
		processFrame(m_actual);
	}
	return true;
}

// TO DO: sometimes you have to hit the key a bunch of times before it responds
void Soccer::commandArrive(string& cmd) {
	// Pauza (Pause)
	if(cmd == "p") {  // this was s for some reason
		m_pause = !m_pause;
		log->debugStream() << "m_pause " << m_pause;
		return;
	}
	if(cmd == "d") {		
		m_drawer->switchDebugDraw();
		return;
	}
	if(cmd == "f") {
		m_drawer->switchDrawType();
		// e.g. silhouette, circle object, label
		return;
	}
	if(cmd == "c") {
		m_drawer->switchTeamColoring();
		return;
	}

	// ROI oblast (area)
	if(cmd == "w") {		
		m_drawer->switchROIDraw();
		return;
	}
	if(cmd == "e") {		
		m_drawer->nextROI();
		return;
	}
	if(cmd == "q") {
		m_drawer->previousROI();
		return;
	}
}

void Soccer::loadNextFrame() {
	// Ziskaj snimok .. (They acquire images .. )
	SAFE_RELEASE(m_actual);
	try {
		m_actual = m_record->readNext();
	} catch(VideoRecord::EndOfStream stream) {
		// Tu nastane 5sec delay ked je koniec streamu, dovod neznamy 
		// (Here 5sec delay occurs when the end of the stream, reason unknown)
		if(log != NULL) {
			log->debugStream() << "Restart stream.";
		}
		m_record->doReset();
		delete m_actual;
		m_actual = m_record->readNext();
	}
	if(log != NULL) {
		//log->debugStream() << "Image: " << m_actual->pos_msec;
	}
}

Mat Soccer::getWarpMatrix() {
	// this assumes the camera is static and that gameplay boundary lines are visible. 
			// In the future we would have to redo this whenever the camera moved
			log->debugStream() << "Looking at first frame for camera angle and field analysis.";
			
			Mat dst, cdst;

			Canny(m_actual->data, dst, 50, 200, 3);
			cvtColor(dst, cdst, CV_GRAY2BGR);
			vector<Vec2f> allLines;

			HoughLines(dst, allLines, 2, CV_PI/180, 175, 0, 0 );

			Vec2f angledLine, topLine, bottomLine;
			
			for( size_t i = 0; i < allLines.size(); i++ ) {
			  float rho = allLines[i][0], theta = allLines[i][1];

			  double deg_theta = (theta*180/CV_PI);
			  while (deg_theta >= 360) deg_theta -=360;

			  // Find vertical lines: (not really useful)
			  // if ( (deg_theta >= 179) && (deg_theta <= 181) ) lines.push_back(allLines.at(i));

			  // Find horizontal lines:
			  if ((deg_theta >= 89.5) && (deg_theta <= 90.5) ) lines.push_back(allLines.at(i));

			  // Find the goal line: (should be about 150 or 30 for our data)
			  if ((deg_theta >= 111) && (deg_theta <= 158) ) {
				  angledLine = allLines.at(i);
				  teamBAttacking = false;
				  std::cout << "TEAM A ATTACKING\n";
				  // assume 'PERSON' is goalie
			  } else if ((deg_theta >= 12) && (deg_theta <= 45) ) {
				  angledLine = allLines.at(i);
				  teamBAttacking = true;
				  // 'GOAL_KEEPER_A' is goalie
				  std::cout << "TEAM B ATTACKING\n";
			  }
			} 

			// Note: (0,0) is top left of screen

			// Based on how I've done line detection and weeded out what I didn't need, 
			// From middle of screen the first horizontal line above it is the touchline
			// Going down, the second one is the touchline (the first is the edge of the penalty area)

			// So step 1: find the centre of the screen
			Point centre;
			centre.y = WIN_SIZE.height/2;
			centre.x = WIN_SIZE.width/2;

			// find the relevant lines:
			// Find bottom line
			int min = WIN_SIZE.height;
			for( size_t i = 0; i < lines.size(); i++ ) {
			  float rho = lines[i][0];

			  if (rho <= centre.x) continue;

			  if (rho < min) {
				  min = rho;
				  bottomLine = lines.at(i);
			  }
			}

			// Find top line
			int max = 0;
			for( size_t i = 0; i < lines.size(); i++ ) {
			  float rho = lines[i][0];

			  if (rho >= (centre.x/2)) continue;

			  if (rho > max) {
				  max = rho;
				  topLine = lines.at(i);
			  }
			}
			
			// Draw bottom line:
			float rho = bottomLine[0], theta = bottomLine[1];
			Point pt1, pt2;
			double a = cos(theta), b = sin(theta);
			double x0 = a*rho, y0 = b*rho;
			pt1.x = cvRound(x0 + 1000*(-b));
			pt1.y = cvRound(y0 + 1000*(a));
			pt2.x = cvRound(x0 - 1000*(-b));
			pt2.y = cvRound(y0 - 1000*(a));
			fline_bot = pt1.y;
			line( cdst, pt1, pt2, Scalar(0,0,255), 1, CV_AA);

			// Don't comment out these draw blocks, variables are needed for later.
			// Draw top line:
			rho = topLine[0], theta = topLine[1];
			a = cos(theta), b = sin(theta);
			x0 = a*rho, y0 = b*rho;
			pt1.x = cvRound(x0 + 1000*(-b));
			pt1.y = cvRound(y0 + 1000*(a));
			pt2.x = cvRound(x0 - 1000*(-b));
			pt2.y = cvRound(y0 - 1000*(a));
			fline_top = pt1.y;
			line( cdst, pt1, pt2, Scalar(0,0,255), 1, CV_AA);

			// Draw angled line:
			rho = angledLine[0], theta = angledLine[1];
			a = cos(theta), b = sin(theta);
			x0 = a*rho, y0 = b*rho;
			pt1.x = cvRound(x0 + 1000*(-b));
			pt1.y = cvRound(y0 + 1000*(a));
			pt2.x = cvRound(x0 - 1000*(-b));
			pt2.y = cvRound(y0 - 1000*(a));
			line( cdst, pt1, pt2, Scalar(0,0,255), 1, CV_AA);

			// Next step: find where the angled line meets the two horizontal lines.
			// the horizontal lines are just y = rho while the angled one is y = m*x + b
			// I know the y value will be rho of the other line because that one is horizontal
			// y - b = m*x => x = (rho - b)/m

			// The slope and intercept will be used to define goalLine struct

			// Figure out the slope based on points defined above just to avoid any angle trouble
			double delta_x, delta_y;

			if (pt2.x > pt1.x) {
				delta_x = pt2.x - pt1.x; 
				delta_y = pt2.y - pt1.y; 
			} else {
				delta_x = pt1.x - pt2.x; 
				delta_y = pt1.y - pt2.y; 
			}

			goalLine.slope = delta_y / delta_x;

			// Now get y-intercept:
			// y = mx + b => b = y - mx

			goalLine.intercept = pt2.y - (goalLine.slope*pt2.x); 
			Point2f c1(topLine[0],(topLine[0] - goalLine.intercept) / goalLine.slope), 
				c2(bottomLine[0],(bottomLine[0] - goalLine.intercept) / goalLine.slope);

			c1.y = topLine[0];
			c1.x = (c1.y - goalLine.intercept) / goalLine.slope;
				
			c2.y = bottomLine[0];
			c2.x = (c2.y - goalLine.intercept) / goalLine.slope;

			// Based on these two corners and the fact that the vertical centre of the screen is 
			// the middle of the trapezoid created by the perspective of the rectangle, find where the other corners would be
			// Note: these are not the real field corners but the pretend corners of the relevant area captured by the camera

			/*
			         c1--------------c4				c1--------------c4
					/				   \			|				 |
				   /					\	  =>	|				 |
			      c2--------------------c3			c2--------------c3
			*/
			Point2f c3, c4;
			// This assumes that the camera is orthogonal to the field
			c3.y = c2.y;
			c4.y = c1.y;

			double topdiff, bottomdiff;
			topdiff = centre.x - c1.x;
			bottomdiff = centre.x - c2.x;

			c4.x = centre.x + topdiff;
			c3.x = centre.x + bottomdiff;

			// define the destination corners
			Point2f d1,d2,d3,d4;
			// these are left in the same  position
			d1 = c1; 
			d4 = c4;
			// warp these
			d2.x = c1.x; 
			d2.y = c2.y;

			d3.x = c4.x;
			d3.y = c3.y;

			// time to get the perspective transformation matrix
			const Point2f sourcePts[4] = {c1,c4,c3,c2}; // top left then clockwise
			const Point2f destPts[4] = {d1,d4,d3,d2};

			Mat m = getPerspectiveTransform(sourcePts, destPts); // transformation matrix 

			// uncomment these 5 lines to see the original frame, the detected corner, and the application of the warp matrix
			/*
			Mat theNewOne = m_actual->data.clone();
			warpPerspective(m_actual->data, theNewOne, warpMatrix, WIN_SIZE, INTER_LINEAR, BORDER_CONSTANT, Scalar());
			imshow("Source", m_actual->data);
			imshow("FIELD CORNER", cdst);
			imshow("Warped", theNewOne);
			*/

			return m;
}

void Soccer::processFrame(Frame* in) {
	// Predspracuj vstup podla potreby, vypocitaj fgMasku
	// (Preprocess input as appropriate, calculate mask)
	resize(m_actual->data, m_actual->data, WIN_SIZE);

	// Learning MOG algorithm (background subtraction)
	if(m_learning) {
		// look at first frame and check lines
		if (in->pos_msec == 1) { // starts at 1 not at 0.
			
			warpMatrix = getWarpMatrix();
		}

		//Mat newOne = m_actual->data.clone();
		//warpPerspective(m_actual->data, newOne, M, WIN_SIZE, INTER_LINEAR, BORDER_CONSTANT, Scalar());

		if(in->pos_msec < m_mogLearnFrames) { // here's where it trains
			Mat mask;
			m_pMOG2->operator()(in->data, mask, 1.0 / m_mogLearnFrames); 
			//m_pMOG2->operator()(newOne, mask, 1.0 / m_mogLearnFrames);
			return;
		}

		if(in->pos_msec == m_mogLearnFrames) {
			learningEnd();
			return;
		}
		return;
	} 
	
	 // No idea why Seksy had this but I used it to test the post 
	// Pauza pre konkretny snimok (Pause for specific frames)
	/*
	if(in->pos_msec == 10) {
		
		while (true) {
			bool pass = notifyRef();
			if (pass) {
				std::cout<<"SUCCESS\n";
				break;
			} else
				std::cout<<"FAILURE\n";
		}
		
	}
	*/
	

	//Mat theNewOne = m_actual->data.clone();
	//warpPerspective(m_actual->data, theNewOne, M, WIN_SIZE, INTER_LINEAR, BORDER_CONSTANT, Scalar());
	processImage(m_actual->data.clone());
	//processImage(theNewOne);
}

void Soccer::learningEnd() {
	log->debugStream() << "Restart stream.";
	m_record->doReset();
	m_learning = false;
	log->debugStream() << "End of learning.";
	m_detector = new ObjectDetector();
	m_drawer = new Drawer();
	m_tracer = new ObjectTracer();
}

void Soccer::processImage(Mat& input) {
	// Ziskaj masku pohybu cez MOG algoritmus
	// (Earn mask movement through MOG algorithm)
	Mat mogMask;
	m_pMOG2->operator()(input, mogMask);

	// Opening
	erode(mogMask, mogMask, Mat(), Point(-1,-1), 1);
	dilate(mogMask, mogMask, Mat(), Point(-1,-1), 3);
	//imshow("mogMask", mogMask); 

	// Ziskaj masku travy cez farbu
	// (Earn mask grass over color)
	Mat grassMask = m_grass->getMask(input);
	bitwise_not(grassMask, grassMask);
	m_grass->createTrackBars("grassMask");
	//imshow("grassMask",grassMask); 
	
	// Vypracuj spolocnu masku (Elaborate common mask)
	Mat finalMask;
	bitwise_and(grassMask, mogMask, finalMask);
	//imshow("finalMask", finalMask); 
	
	// Najdi objekty (Find objects)
	vector<FrameObject*> objects, teama, teamb;
	FrameObject* ball = NULL;
	m_detector->findObjects(input, finalMask, objects, teama, teamb, ball, fline_top,fline_bot);
	m_tracer->process(input, objects);
	m_drawer->draw(input, finalMask, objects);
}

void Soccer::Init() {
	m_record = new VideoRecord("data/filmrole6.avi"); // note: 3 and 4 are of centre field, focus on 1,2,5,6
	m_pMOG2 = new BackgroundSubtractorMOG2(200, 16.0, false);
	m_grass = new ThresholdColor(Scalar(26, 18, 8), Scalar(75, 168, 200)); // 35,72,50 to 51, 142, 144 is what Seksy had
	m_learning = true;
	m_mogLearnFrames = 200;
	m_pause = false;
	m_actual = NULL;

	const char* windows[] = { 
		//"mogMask", 
		//"grassMask",
		//"Color mask", 
		//"Roi", 
		"Output" 
	};
	//createWindows(windows);
	m_grass->createTrackBars("grassMask");
}

bool Soccer::notifyRef() {
	Poco::URI uri("http://ouda-server.herokuapp.com/notifyreferee");
    std::string path(uri.getPathAndQuery());
    if (path.empty()) path = "/";

	Poco::Net::HTTPClientSession session(uri.getHost(), uri.getPort());
    Poco::Net::HTTPRequest request(Poco::Net::HTTPRequest::HTTP_POST, path, Poco::Net::HTTPMessage::HTTP_1_1);
    Poco::Net::HTTPResponse response;

	session.setKeepAlive(true);
	response.setKeepAlive(true);

	Poco::JSON::Object obj;
    obj.set("team", "A");
	std::stringstream ss;
	obj.stringify(ss);
	request.setKeepAlive(true);
	request.setContentLength(ss.str().size());
	request.setContentType("application/json"); 

	response.setStatus(Poco::Net::HTTPResponse::HTTP_FORBIDDEN); // just doing this because OK is the default.

	std::ostream& o = session.sendRequest(request);
	obj.stringify(o);
	std::istream& i = session.receiveResponse(response);

    std::cout << response.getStatus() << " " << response.getReason() << std::endl;
    if (response.getStatus() == Poco::Net::HTTPResponse::HTTP_OK)
    {
		std::cout<<response.getContentType()<<std::endl;
		std::cout << i.rdbuf();
        return true;
    }
    else
    {
        return false;
    }
}

Size FrameObject::WIN_SIZE = Size(640, 480);  //Size(1920, 1080);
Size Soccer::WIN_SIZE = Size(640, 480); 

// TODO: - skupina, ked sa dotykaju rukou tak pouzijem opening a zistim, ci tam nebude torso hraca 2x, 3x
// TODO: - torso zistim extra eroziou, kde ruky a hlavu odstranim
// TODO: - pomocou trajektorie hraca zistim, kolko hracov tam je
// TODO: - lopta sa riesi druhym tokom, potom hladam cez hugh circle o urcitej velkosti a biele pozadie motion gradient

// TODO: - group when hands touch so we'll use opening and see if there is not a player torso 2x, 3x
// TODO: - torso zistim extra erosion, where the hands and head odstranim
// TODO: - using the path a player how many players are there
// TODO: - the ball is solved Druhym flow, then look through Hough circle of a certain size and white background gradient motion

