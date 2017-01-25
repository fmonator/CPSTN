#include "class.Soccer.hpp"
#include <opencv2/imgproc/imgproc.hpp>
#include "util.h"

Soccer::Soccer() 
{
	// Nacitaj vsetky casti Soccera (Load all parts Soccer)
	log = CREATE_log4CPP();
	if(log != NULL) {
		log->debug("Starting Soccer");
	}
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
	int key = cv::waitKey(30);
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
		log->debugStream() << "Image: " << m_actual->pos_msec;
	}
}

void Soccer::processFrame(Frame* in) {
	// Predspracuj vstup podla potreby, vypocitaj fgMasku
	// (Preprocess input as appropriate, calculate mask)
	resize(m_actual->data, m_actual->data, WIN_SIZE);

	// Learning MOG algorithm (background subtraction)
	if(m_learning) {
		// look at first frame and check lines
		if (in->pos_msec == 1) { // starts at 1 not at 0.
			// this assumes the camera is static and that gameplay boundary lines are visible. 
			// In the future we would have to redo this whenever the camera moved
			log->debugStream() << "Looking at first frame for camera angle and field analysis.";
			
			Mat dst, cdst;
			/*
			// I'm thinking of preprocessing the image: lower contrast may allow differences in green to be detected
			// higher contrast may get rid of some of the noise
			double gain = 0.5; // contrast [1.0,3.0] (as it goes up, you get more random lines but not the ones I want)
			int bias = 0; // brightness [0,100]

			for( int y = 0; y < m_actual->data.rows; y++ ){ 
				for( int x = 0; x < m_actual->data.cols; x++ ){ 
					for( int c = 0; c < 3; c++ ) { 
						m_actual->data.at<Vec3b>(y,x)[c] = saturate_cast<uchar>( gain*( m_actual->data.at<Vec3b>(y,x)[c] ) + bias ); 
					}
				}
			}
			*/

			Canny(m_actual->data, dst, 50, 200, 3);
			cvtColor(dst, cdst, CV_GRAY2BGR);
			//HoughLines(dst, lines, 1, CV_PI/180, 150, 0, 0 ); // original
			HoughLines(dst, lines, 2, CV_PI/180, 175, 0, 0 );

			for( size_t i = 0; i < lines.size(); i++ ) {
			  float rho = lines[i][0], theta = lines[i][1];
			  Point pt1, pt2;
			  double a = cos(theta), b = sin(theta);
			  double x0 = a*rho, y0 = b*rho;
			  pt1.x = cvRound(x0 + 1000*(-b));
			  pt1.y = cvRound(y0 + 1000*(a));
			  pt2.x = cvRound(x0 - 1000*(-b));
			  pt2.y = cvRound(y0 - 1000*(a));
			  line( cdst, pt1, pt2, Scalar(0,0,255), 1, CV_AA);
			} 

			log->debugStream() << lines.size();
			imshow("source", m_actual->data);
			imshow(to_string(lines.size()), cdst);
			
		}

		if(in->pos_msec < m_mogLearnFrames) { // here's where it trains
			Mat mask;
			m_pMOG2->operator()(in->data, mask, 1.0 / m_mogLearnFrames);
			return;
		}

		if(in->pos_msec == m_mogLearnFrames) {
			learningEnd();
			return;
		}
		return;
	} 
	
	/* // No idea why Seksy had this
	// Pauza pre konkretny snimok (Pause for specific frames)
	if(in->pos_msec == 355) {
		m_pause = true;
	}
	*/

	processImage(m_actual->data.clone());
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
	//imshow("grassMask",grassMask); 
	
	// Vypracuj spolocnu masku (Elaborate common mask)
	Mat finalMask;
	bitwise_and(grassMask, mogMask, finalMask);
	//imshow("finalMask", finalMask); 
	
	// Najdi objekty (Find objects)
	vector<FrameObject*> objects;
	m_detector->findObjects(input, finalMask, objects);
	m_tracer->process(input, objects);
	m_drawer->draw(input, finalMask, objects);
}


void Soccer::Init() {
	m_record = new VideoRecord("data/filmrole6.avi"); // note: 3 and 4 are of centre field, focus on 2,5,6
	m_pMOG2 = new BackgroundSubtractorMOG2(200, 16.0, false);
	m_grass = new ThresholdColor(Scalar(35, 72, 50), Scalar(51, 142, 144));
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

Size FrameObject::WIN_SIZE = Size(640, 480);  //Size(1920, 1080);
Size Soccer::WIN_SIZE = Size(640, 480); 
Size Drawer::WIN_SIZE = Size(640, 480); 

// TODO: - skupina, ked sa dotykaju rukou tak pouzijem opening a zistim, ci tam nebude torso hraca 2x, 3x
// TODO: - torso zistim extra eroziou, kde ruky a hlavu odstranim
// TODO: - pomocou trajektorie hraca zistim, kolko hracov tam je
// TODO: - lopta sa riesi druhym tokom, potom hladam cez hugh circle o urcitej velkosti a biele pozadie motion gradient

// TODO: - group when hands touch so we'll use opening and see if there is not a player torso 2x, 3x
// TODO: - torso zistim extra erosion, where the hands and head odstranim
// TODO: - using the path a player how many players are there
// TODO: - the ball is solved Druhym flow, then look through Hough circle of a certain size and white background gradient motion

