#include <vector>
#include "scoring_matrix.h"
#include "sw_align_serial.h"
#include "sw_align_parallel.h"
#include "sequence_provider.h"
#include "fasta_reader.h"

char *deliverText();
#if 0
#if _MSC_VER
	void PrintMemoryInfo( DWORD processID );
#endif

void forwardSequenceProviderLoad( SequenceProvider &sequenceProvider, char* pFileName );

void forwardSequenceProviderInverse( SequenceProvider &sequenceProvider );

void applyAlignmentUsingStringsTest( std::vector< alignment_description_element<char>> &alignmentOutcomeVector );

void doParallelAlignment(  SequenceProvider *fastaReaderReaderColumn,
							SequenceProvider *fastaReaderReaderRow,
							max_profiler<int16_t, size_t> *maxProfilerRef 
							);

void doSerialAlignment(  SequenceProvider *fastaReaderReaderColumn,
							SequenceProvider *fastaReaderReaderRow,
							std::vector< alignment_description_element<char>> &alignmentOutcomeVector,
							size_t endIndexRegardingRow, 
							size_t rangeRegardingRow,
							size_t &absoluteStartInRow,
							size_t &absoluteEndInRow 
						 );
#endif