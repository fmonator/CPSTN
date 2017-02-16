#pragma once
//#include <windows.h>
#define NOMINMAX
#include <limits>
#include <opencv2/imgproc/imgproc.hpp>
#include "log4cpp.h"

/**
* Trieda reprezentuju aplikaciu v najvyssej podobe.
* teda tato trieda sa spusta v maine ako prva a predstavuje
* urcitu komunikaciu s prostredim, teda operacnym systemom.
*/

/*
* Class represent an application of the highest form.
* Thus, this class will start in Maine and is the first
* Certain communication with the environment, that operating system.
*/

class App {
protected:
	log4cpp::Category* blog;
	bool m_running;
	cv::Mat M; // perspective warp matrix
	bool teamBAttacking; // is teamBAttacking or not? Used for offside comparisons

	virtual int getLockFPS() = 0;
	virtual void Init() = 0;
	virtual bool Run()  = 0;
	virtual void LimitFPS(double delta);
	virtual void Refresh()  {}
	virtual void MainCycle();
public:
	// Prvotna inicializacia (The primary initialization)
	App();

	// Spusti aplikaciu po celkovej inicializacii (Start the application after a total initialization)
	virtual void start();
	// Moze bezat nadalej aplikacia, nenastal problem ?
	/*
	Can run further applications, there is a problem? || REMAiN can run applications, problems arose?
	*/
	bool canRun(); 

	// Prikaz na zastavenie behu aplikacie (Stop command runtime)
	void stop();
};