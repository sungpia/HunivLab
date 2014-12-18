#pragma once

#include <iostream>
#include <memory>

#include "system.h"
#include "sw_align_parallel.h"
#include "sw_align_serial.h"
#include "max_profiler.h"
#include "exception.h"
#include "scoring_matrix.h"

#if 1
	#define DEBUG_MSG_WORKFRAME(str) do { std::cout << str << std::endl; } while( false )
#else
	#define DEBUG_MSG_WORKFRAME(str) do { } while ( false )
#endif

/* This is the core library function for parallel local alignments.
 * The parallel Aligner requires aligned memory for its scoring profile.
 */
template<class T_scoring>
void doParallelAlignment(	GeneticSequenceSlice *pColumnSequenceRef,
							GeneticSequenceSlice *pRowSequenceRef,
							SmithWatermanParamaterSet<T_scoring> &SWparameterSet,
							max_profiler<T_scoring, size_t> *maxProfilerRef 
						 ) 
{
#if _MSC_VER
	PrintMemoryInfo( GetCurrentProcessId() );
#endif

	try 
	{
		/* TO DO: Throw an exception if something goes wrong in the context of memory allocation
		 */
		SW_align_par_type<T_scoring, size_t> parAlignerObject( pColumnSequenceRef->uxGetSequenceSize(),
															   (uint8_t *) pColumnSequenceRef->uxGetSequence(),
															   SWparameterSet, // call by reference
															   5 // alphabet size is fix with 5 elements
														     );
	
		T_scoring maxScore;
		
		/* Here we do the core parallel alignment.
		 * Let the system infer the time type... It is different for MSVC and GCC
		 */
		auto time = time_call_
			( [&]()
				{
					maxScore = parAlignerObject.align16( pRowSequenceRef->uxGetSequenceSize(),
														 (uint8_t *) pRowSequenceRef->uxGetSequence(),
														 maxProfilerRef
													   );
				} // lambda
			);
		
		DEBUG_MSG_WORKFRAME( "MAXIMUM :"  << maxScore );
		DEBUG_MSG_WORKFRAME( "Execution time was: " << timeAsString( time ) );	
	} 
	catch ( std::exception e )
	{
		DEBUG_MSG_WORKFRAME( "catched exception: " << e.what() );
	}
	
} // generic function

/* Serial alignment with generation of a scoring matrix for backtracking.
 * We align on the foundation of fastaReaderReaderColumn and fastaReaderReaderRow
 * endIndex indicates normally the position of a local maximum, where counting start with zero.
 * range is the number of elements that we move back.
 * Rteurn value is maxium score found in the context of teh alignment.
 * WARNING:
 * endIndex has to be understood as INCLUSIVE not "pointer to first character after"
 */
template<class T_scoring>
T_scoring doSerialAlignment( GeneticSequenceSlice *pColumnSequenceRef,
							 GeneticSequenceSlice *pRowSequenceRef,
							 SmithWatermanParamaterSet<T_scoring> &SWparameterSet,
							 alignment_description<char> &alignmentOutcomeVector,
							 size_t uxEndIndexRegardingRow, // input
							 size_t uxRangeRegardingRow,	 // input
							 size_t &ruxAbsoluteStartInRow,	 // output
							 size_t &ruxAbsoluteEndInRow	 // output
					       ) 
{
	DEBUG_MSG_WORKFRAME( "Backtracking for range: " << uxRangeRegardingRow << " endIndex " << uxEndIndexRegardingRow );

	/* Safety check.
	 */
	if ( uxEndIndexRegardingRow > pRowSequenceRef->uxGetSequenceSize() )
	{
		throw aligner_exception("wrong offset");
	}

	/* We have to be careful! E.g. endIndex == 0 means until (inclusive) the first element!
	 * We increment here, because endIndexRegardingRow shall refer the first element after.
	 */
	uxEndIndexRegardingRow++; 
	
	/* If we have get range 0, the serial algorithm will throw some exception!
	 */
	if ( uxEndIndexRegardingRow < uxRangeRegardingRow )
	{
		uxRangeRegardingRow = uxEndIndexRegardingRow;
	}

	/* TO DO: Implement range 0 check and check once again, whether (offset - range) is OK.
	 * Creation of an alignment object.
	 */
	SW_align_type<true, T_scoring, size_t>aligner(			  pColumnSequenceRef->uxGetSequenceSize(), 
												  (uint8_t *) pColumnSequenceRef->uxGetSequence(), 
														 	  uxRangeRegardingRow, 
												  (uint8_t *) pRowSequenceRef->uxGetSequence() + (uxEndIndexRegardingRow - uxRangeRegardingRow),
															  SWparameterSet
											     );
	
	size_t indexOfMaxElementInScoringTable;

	/* The core alignment operation. Serial and with matrix generation.
	 */
	T_scoring maxScore = aligner.swAlign( NULL, NULL, indexOfMaxElementInScoringTable );
	
	size_t startPositionInColumn, endPositionInColumn, startPositionInRow, endPositionInRow;
	
	/* We apply backtracking for getting the alignment.
	 */
	aligner.scoringMatrix.backtrackFromIndex( indexOfMaxElementInScoringTable,
											  alignmentOutcomeVector,
											  startPositionInColumn, // pass by reference
											  endPositionInColumn,	 // pass by reference
											  startPositionInRow,    // pass by reference 
											  endPositionInRow		 // pass by reference
											);

	/* The following two values assume that the first element(nucleotide) has index 0.
	 * Please consider this in the context of later use.
	 */
	ruxAbsoluteStartInRow = startPositionInRow + (uxEndIndexRegardingRow - uxRangeRegardingRow);
	ruxAbsoluteEndInRow = endPositionInRow + (uxEndIndexRegardingRow - uxRangeRegardingRow);
	
#if 0
	/* DEBUGGING: We dump the matrix!
	 */
	aligner.scoringMatrix.dump();
#endif
	
	/* Some debugging messages
	 */
	DEBUG_MSG_WORKFRAME( "MAXIMUM :"  << maxScore );
	DEBUG_MSG_WORKFRAME( "Value at "  << indexOfMaxElementInScoringTable <<  " : "
	     << aligner.scoringMatrix.scoringOutcomeMatrix[indexOfMaxElementInScoringTable]
	     << " relative start : " << startPositionInRow
	     << " relative end : " << endPositionInRow
		 << "   absolute start : " << ruxAbsoluteStartInRow // THIS REPESENTS A CORRECT POSITION! (However the first character has index 0!)
	     << " absolute end : " << ruxAbsoluteEndInRow		// THIS REPESENTS A CORRECT POSITION! (However the first character has index 0!)
		 );

	/* We deliver the maximum score back to the caller.
	 */
	return maxScore;
} // function

template<class T_scoring>
void alignment( GeneticSequenceSlice &rColumnSequence, GeneticSequenceSlice &rRowSequence ) 
{
	try {
		/* We fill the parameter set with requested Smith-Waterman parameter
		 */
		SmithWatermanParamaterSet<T_scoring> SWparameterSet( 10, -3, 10, 5 );
		max_profiler<T_scoring, size_t> maxProfiler;
		
		/* First we do a parallel alignment to find all matches.
		 */
		doParallelAlignment( &rColumnSequence, &rRowSequence, SWparameterSet, &maxProfiler ); 

		/* The max-profiler is a recursive data structure, where the interesting parts are on bottom level
		 * We have to perform the following steps:
		 *	1. deep flush to get everything to the deepest level
		 *	2. Shrinking to the requested size
		 *	3. Sorting the data
		 */ 
		maxProfiler.deep_flush();
		maxProfiler.getDeepestChild()->filterUntilRequestedSize( 20 );
		maxProfiler.getDeepestChild()->vSort();

		/* We have now the required profile ...
		 */
#if 0
		maxProfiler.getDeepestChild()->dump();
#endif
		DEBUG_MSG_WORKFRAME( "Size of profile: " << maxProfiler.getDeepestChild()->sizeOfProfile );
		
		/* We do a serial alignment incl. backtracking for all entries in the maxima-profile.
		 * uxRow is the index within rRowSequence, so its is a value within [0..size(rRowSequence) - 1]
		 * (WARNING: Counting does not start with 1 here.)
		 */
		maxProfiler.getDeepestChild()->doForAllProfileEntries
			( [&](size_t uxRow, T_scoring xScore) 
				{ 
					DEBUG_MSG_WORKFRAME(    "In row: " << uxRow  
										 << " the parallel aligner found max: " << xScore 
										 << " backtrack distance: " << SWparameterSet.xGetApproximatedBackTrackDistance( rColumnSequence.uxGetSequenceSize(), xScore )
									   );
					size_t absoluteStartInRow;
					size_t absoluteEndInRow;

					{	/* We need a fresh alignmentOutcome object with each iteration.
						 */
						alignment_description<char> alignmentOutcome;

						/* Calculate the approximated backtrack distance.
						 */
						size_t uxBacktrackDistance = SWparameterSet.xGetApproximatedBackTrackDistance( rColumnSequence.uxGetSequenceSize(), xScore );

						T_scoring xMaxScore = doSerialAlignment
						(	
							&rColumnSequence,
							&rRowSequence,
							SWparameterSet,
							alignmentOutcome,
							uxRow,													   // The Endpoint regarding the rows. size_t endIndexRegardingRow, 
							uxBacktrackDistance > uxRow + 1 ? uxRow + 1 : uxBacktrackDistance, // The Number of Elements that we want to go back. 
							absoluteStartInRow,
							absoluteEndInRow 
						); 
						
						/* We check whether the serial aligner comes to the same result like his parallel brother.
						 * However, if we see a difference this does not mean real trouble. uxBacktrackDistance was then simply to large and we catch a previous match.
						 * !! These endpoints are inclusive and computed on a counting system starting with 0 !!
						 */
						if ( absoluteEndInRow != uxRow )
						{
							throw aligner_exception("serial and parallel aligner come to different end-points");
						} // if

						if ( xMaxScore != xScore )
						{
							throw aligner_exception("serial and parallel aligner computed different maximum scores");
						} // if

						/* We print the outcome to the console
						 */
						std::string string;
						std::cout << alignmentOutcome.rsAppendStringRepesentation( string );

						/* We create a HTML representation for the alignment
						 */
						tinyxml2::XMLDocument HTMLDocument;
						tinyxml2::XMLNode* pRootHTMLElementRef = HTMLDocument.InsertEndChild( HTMLDocument.NewElement( "html") );
						tinyxml2::XMLNode* pHEADElementRef = pRootHTMLElementRef->InsertEndChild( HTMLDocument.NewElement( "head") );
						tinyxml2::XMLNode* pBODYElementRef = pRootHTMLElementRef->InsertEndChild( HTMLDocument.NewElement( "body") );
					
						alignmentOutcome.getHTMLRepresentation( HTMLDocument, *pBODYElementRef );

						HTMLDocument.SaveFile("output.html");
					} // destruction of local alignmentOutcome object
				} // lambda 
			);
	} // try
	catch (std::exception &e)
	{
		/* TO DO log the message somehow
		 */
		std::cout << "Alignment failed due to the following exception: " << e.what() << std::endl;
	} // catch
} // main function