#include "system.h"
#include <stdio.h>
#include <sstream>

/* Some general system related functions
 */

using namespace std;
#if _MSC_VER
	void PrintMemoryInfo( DWORD processID )
	{
		HANDLE hProcess;
		PROCESS_MEMORY_COUNTERS pmc;

		// Print the process identifier.

		printf( "\nProcess ID: %u\n", processID );

		// Print information about the memory usage of the process.

		hProcess = OpenProcess(  PROCESS_QUERY_INFORMATION |
			PROCESS_VM_READ,
			FALSE, 
			processID );
		if (NULL == hProcess)
			return;

		if ( GetProcessMemoryInfo( hProcess, &pmc, sizeof(pmc)) )
		{
			printf( "\tPageFaultCount: 0x%08X\n", pmc.PageFaultCount );
			printf( "\tYour app's PEAK MEMORY CONSUMPTION: %d, 0x%08X\n", 
				pmc.PeakWorkingSetSize );
			printf( "\tYour app's CURRENT MEMORY CONSUMPTION: %d, 0x%08X\n", pmc.WorkingSetSize );
			printf( "\tQuotaPeakPagedPoolUsage: %d, 0x%08X\n", 
				pmc.QuotaPeakPagedPoolUsage );
			printf( "\tQuotaPagedPoolUsage: %d, 0x%08X\n", 
				pmc.QuotaPagedPoolUsage );
			printf( "\tQuotaPeakNonPagedPoolUsage: %d, 0x%08X\n", 
				pmc.QuotaPeakNonPagedPoolUsage );
			printf( "\tQuotaNonPagedPoolUsage: %d, 0x%08X\n", 
				pmc.QuotaNonPagedPoolUsage );
			printf( "\tPagefileUsage: %d, 0x%08X\n", pmc.PagefileUsage ); 
			printf( "\tPeakPagefileUsage: %d, 0x%08X\n", 
				pmc.PeakPagefileUsage );
		}

		CloseHandle( hProcess );
	} // function
#endif

#ifdef _MSC_VER
	/* not that efficient... but working...
	 */
	std::string timeAsString( const __int64 &time )
	{
		std::stringstream stringStream;
		stringStream << "(" << time << ")";
		return stringStream.str();
	} //function
#elif __GNUC__
	/* not that efficient... but working...
	 */
	std::string timeAsString( const timespec &time )
	{
		std::stringstream stringStream;
		stringStream << "(" << time.tv_sec << " : " << time.tv_nsec << ")";
		return stringStream.str();
	} // function
#endif