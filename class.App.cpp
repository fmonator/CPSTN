/** @file class.App.cpp
*
* Implementacia triedy App. (Implementation of App class)
*/
#include "class.App.hpp"
#include "class.Time.hpp"
#include <iostream>
#include <chrono>
#include <thread>

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

			// this next little bit is also related to <windows.h> but I'm not sure if this ever happens
			// UPDATE: it happens constantly

			// UPDATE: (Solution?)
			// Seksy himself commented out the only use of deltaTime so I'm going to comment out the lines that deal with that
			// I will also comment out the useless if block since the only line in it is commented out again by Seksy

			
			//double time = Time::getInstance().GetAbsolute();
			bool refreshApp = Run(); // this line is actually crucial
			//double deltaTime = time - Time::getInstance().GetAbsolute();

			/*
			if(refreshApp) {
				// nemozme priamo volat UpdateWindow ale toto mozme ..je take iste
				//(You can directly call UpdateWindow but this can be the same ..is)
				//Refresh(); // commented out by Seksy
			}
			*/
			
			//LimitFPS(deltaTime); // commented out by Seksy
		}
	}
}

void App::LimitFPS(double delta) {
	double sleepTime = ( (1.0 / getLockFPS()) - delta) * 1000.0f;
	/*if(blog != NULL) {
	blog->debugStream() << "delta " << delta << "\n";
	}*/
	if(sleepTime < 0) { // This is unlikely to happen: if our app is running too fast hahaha
		// Ak vypocet snimku klesol pod nase FPS
		// (If the calculation rate drops below our FPS)

		// We could comment out the next line and it would still work?

		//sleep(0); // pomahame windowsu sa vyrovnat s hrou (Windows, help to cope with the game)
		std::this_thread::sleep_for(std::chrono::milliseconds(0));
		std::cout<<"WINDOWS SLEEP 0 \n"; // I never saw this get logged
	} else {
		/*if(blog != NULL) {
			blog->debugStream() << "sleep " << sleepTime << "\n";
		}*/

		// is this windows-only?
		//sleep((DWORD) sleepTime);
		std::this_thread::sleep_for(std::chrono::milliseconds((DWORD) sleepTime));
		std::cout<<"WINDOWS SLEEP DWORD \n"; // I never saw this get logged either
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