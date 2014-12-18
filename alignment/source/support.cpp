// #include "support.h"
#include "fasta_reader.h"
#include <vector>


/* taken from http://stackoverflow.com/questions/236129/how-to-split-a-string-in-c
 * There are efficient BOOST solutions for splitting as well.
 */
void split(std::vector<std::string> &result, std::string str, char delim ) {
	std::string bufferString;
	std::string::iterator iterator;
	result.clear();

	for ( iterator = str.begin(); iterator < str.end(); iterator++ ) 
	{
		if ( (const char)*iterator != delim ) 
		{
			bufferString += *iterator;
		} // if 
		else 
		{
			result.push_back(bufferString);
			bufferString.clear();
		} // else
	} // for

	if (! bufferString.empty() )
	{
		result.push_back(bufferString);
	} // if
} // function

/* The translation table for columns.
 */
const unsigned char GeneticSequence::xNucleotideTranslationTable[256] = 
{
	4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
	4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
	4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,
	4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
	4, 0, 4, 1,  4, 4, 4, 2,  4, 4, 4, 4,  4, 4, 4, 4,  // A == 0; C == 1; G == 2;
	4, 4, 4, 4,  3, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  // T == 3;
	4, 0, 4, 1,  4, 4, 4, 2,  4, 4, 4, 4,  4, 4, 4, 4,  // a == 0; c == 1; g == 2;
	4, 4, 4, 4,  3, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  // t == 3;
	4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
	4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
	4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
	4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
	4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
	4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
	4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4, 
	4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4,  4, 4, 4, 4
};

#if 0

/* Important: The input sequence must be Null-terminated.
 * The text sequence is translated into a sequence of numbers 
 */
void translateSequence( char *pInputSequence, const char *alphabetTranslationTable )
{
	while ( *pInputSequence != '\0' )
	{
		*pInputSequence = alphabetTranslationTable[ (uint8_t)*pInputSequence ];
		pInputSequence++;
	}
}

void inverseSequence(char *pInputSequence, size_t length)
{
	int i = 0;
	while ( i < length )
	{
		switch ( *pInputSequence )
		{
		case 0 :  *pInputSequence = 3;
				  break;
		case 1 :  *pInputSequence = 2;
				  break;
		case 2 :  *pInputSequence = 1;
				  break;
		case 3 :  *pInputSequence = 0;
				  break;
		default :;
		}
		pInputSequence++;
		i++;
	} // while
} // function

/* Translates a number back to its corresponding ASCII-character
 */
char translateACGTCodeToCharacter(uint8_t uxACGTCode)
{
	const char chars[4] = {'A', 'C', 'G', 'T'};
	if (uxACGTCode < 4)
	{
		return chars[uxACGTCode];
	}
	else
	{
		return 'N';
	}
}
#endif