#pragma once
//#include <windows.h>

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

/* 
// this is the definition for the problem type:
typedef union _LARGE_INTEGER {
  struct {
    DWORD LowPart;
    LONG  HighPart;
  };
  struct {
    DWORD LowPart;
    LONG  HighPart;
  } u;
  LONGLONG QuadPart;
} LARGE_INTEGER, *PLARGE_INTEGER;
*/

// ok so it looks like the issue here is that some of these data types are windows-specific
class Time
{
private: 
	//LONGLONG m_llQPFTicksPerSec;
	long long m_llQPFTicksPerSec;

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
