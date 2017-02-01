/** @file class.App.cpp
*
* Implementacia triedy App. (Implementation of App class)
*/
#include "class.App.hpp"
#include "class.Time.hpp"
#include <iostream>

void App::MainCycle() {
	// Toto je hlavny cyklus aplikacie. (This is the main loop applications.)
	// V nom sa udrzuje beh apliakcie v cykle. (In it it keeps running Applications in the cycle.)
	// Po kazdom cykle prebieha preposielanie sprav z oper. systemom. (After each cycle runs forward mail from operas. systems.)
	MSG msg;
	ZeroMemory( &msg, sizeof(msg) );
	while(canRun()) {
		if( PeekMessage( &msg, NULL, 0U, 0U, PM_REMOVE ) ) { // prekladame spravy (Message translation [That's what I'm doing rn])
			TranslateMessage( &msg );  // prekladame spravy (Message translation)
			DispatchMessage( &msg ); // uvolnujeme (release)
		} else {
			double time = Time::getInstance().GetAbsolute();
			bool refreshApp = Run();
			double deltaTime = time - Time::getInstance().GetAbsolute();;
			if(refreshApp) {
				// nemozme priamo volat UpdateWindow ale toto mozme ..je take iste
				//(You can directly call UpdateWindow but this can be the same ..is)
				//Refresh();
			}
			//LimitFPS(deltaTime);
		}
	}
}

void App::LimitFPS(double delta) {
	double sleepTime = ( (1.0 / getLockFPS()) - delta) * 1000.0f;
	/*if(blog != NULL) {
	blog->debugStream() << "delta " << delta << "\n";
	}*/
	if(sleepTime < 0) {
		// Ak vypocet snimku klesol pod nase FPS
		// (If the calculation rate drops below our FPS)
		Sleep(0); // pomahame windowsu sa vyrovnat s hrou (Windows, help to cope with the game)
	} else {
		/*if(blog != NULL) {
			blog->debugStream() << "sleep " << sleepTime << "\n";
		}*/
		Sleep((DWORD) sleepTime);
	}
}

App::App() {
	blog = CREATE_log4CPP();
}

void App::start() {
	Init();
	m_running = true;
	MainCycle();
	M = NULL;
}

bool App::canRun() {
	return m_running;
}
void App::stop() {
	m_running = false;
}