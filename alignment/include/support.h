#pragma once

#include <string>
#include <sstream>
#include <stdlib.h>

#ifdef __GNUC__
	#include <stdint.h>
#elif _MSC_VER
	/* Several data-type definitions of stdint.h not conatined in the MSC compiler
	 */
	typedef unsigned __int8 uint8_t;
	typedef signed __int8 int8_t;

	typedef unsigned __int32 uint32_t;
	typedef signed __int32 int32_t;

	typedef unsigned __int16 uint16_t;
	typedef signed __int16 int16_t;

	typedef unsigned __int64 uint64_t;
#endif

/* This can be replaced by std::to_string(...) with C++11
	*/
template <typename TP>
std::string numberToString( TP number )
{
	std::ostringstream ss;
	ss << number;
	return ss.str();
}

#if 0
/* Void codes indicate missing or non A,C,G,T characters.
 * For rows and columns we work with different void codes.
 */
//#define DEF_VOID_CODE_COLUMN ( 4 )
//#define DEF_VOID_CODE_ROW ( 5 )


#ifdef __cplusplus
extern "C" {
#endif
	extern const unsigned char alphabetTranslationTableColumn[256];
	extern const unsigned char alphabetTranslationTableRow[256];
	
	void translateSequence( char *pInputSequence, const char *alphabetTranslationTable );

	void inverseSequence(char *pInputSequence, size_t length);

	char translateACGTCodeToCharacter(uint8_t uxACGTCode);

#ifdef __cplusplus
}
#endif

#endif
