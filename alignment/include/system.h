#include <string>

#if _MSC_VER
	#include <windows.h>
	#include <psapi.h>
	#pragma comment(lib, "psapi.lib") 
#endif

#if _MSC_VER
	void PrintMemoryInfo( DWORD processID );
#endif

#if __GNUC__
	#include <time.h>
#endif

#if _MSC_VER
	template <class TP_FUNC_APPLY>
	__int64 time_call_(TP_FUNC_APPLY &&f)
	{
		__int64 begin = GetTickCount();
		f();
		return GetTickCount() - begin;
	}
	
#elif __GNUC__
	template <class TP_FUNC_APPLY>
	timespec time_call_(TP_FUNC_APPLY &&f)
	{
		timespec startTime, endTime, differenceTime;
		
		/* the function call embedded by two time measurements
		 */
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &startTime);
		f();
		clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &endTime);
		
		/* compute the time difference
		 */
		if ( (endTime.tv_nsec - startTime.tv_nsec) < 0 ) 
		{
			/* overflow occurred
			 */
			differenceTime.tv_sec = endTime.tv_sec - startTime.tv_sec - 1;
			differenceTime.tv_nsec = 1000000000 + endTime.tv_nsec - startTime.tv_nsec;
		} 
		else 
		{
			differenceTime.tv_sec = endTime.tv_sec - startTime.tv_sec;
			differenceTime.tv_nsec = endTime.tv_nsec - startTime.tv_nsec;
		}
		
		return differenceTime;
	}
#endif

#ifdef __GNUC__	
	std::string timeAsString( const timespec &time );
#endif

#ifdef _MSC_VER
	std::string timeAsString( const __int64 &time );
#endif