#include "genetics.h"

/* Global data structure that keeps the names for the individual NCBI classifications.
 */
const char *ChromosomeSequence::namesForStrandTypes[3] = 
{
	"UNKOWN",	// 0
	"FORWARD",  // 1
	"REVERSE"	// 2
};

const char *GeneLocusDescriptor::classificationNames[9] = 
{
	"?",		// 0
	"GENOMIC",  // 1
	"?",		// 2
	"MRNA",		// 3
	"?",		// 4
	"?",		// 5
	"?",		// 6
	"?",		// 7
	"PEPTIDE"   // 8
};