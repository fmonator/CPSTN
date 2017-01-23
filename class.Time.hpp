#pragma once
#include <windows.h>

/**
* Trieda pre urcenie velmy presneho aktualneho casu
* Za pomoci nej dokazeme urcit cas medzi 2 bodmy.
* Je thread safe.
*/

/**
* Class for determining highly accurate current time
* With the help of another we can determine the time between two points.
* It is thread safe.
*/
class Time
{
private: 
	LONGLONG m_llQPFTicksPerSec;

	Time(const Time& a);  
	Time& operator=(const Time& a); 
	Time() {
		LARGE_INTEGER qwTicksPerSec = { 0 };
		QueryPerformanceFrequency( &qwTicksPerSec );
		m_llQPFTicksPerSec = qwTicksPerSec.QuadPart;
	}

public:
	// Trieda je typu singleton (The class is a singleton type)
	static Time& getInstance() {
		static Time singleton; 
		return singleton; 
	}; 

	// Ziskaj akutalny cas v ms (You will get the current time in ms)
	inline double GetAbsolute()	{
		LARGE_INTEGER qwTime = { 0 };
		QueryPerformanceCounter( &qwTime );
		double fTime = qwTime.QuadPart / ( double )m_llQPFTicksPerSec;
		return fTime;
	}
};
