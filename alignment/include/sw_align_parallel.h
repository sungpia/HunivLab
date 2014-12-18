#pragma once

#define DUMP_MATRIX (1)

#if _MSC_VER
	#include <windows.h>
#endif

#include <vector>
#include "max_profiler.h"
#include "scoring_matrix.h"

#ifdef __GNUC__
	// SSE2
	#include <emmintrin.h>
	
	// SSE4 
	#include <smmintrin.h>
#endif

void vDump__m128i( __m128i value )
{
	union {
		 __m128i value;    
		 int16_t arr[8];  
	} converter;
	converter.value = value; 

	for (int iterator = 0; iterator < 8; iterator++)
	{
		std::cout << converter.arr[iterator] << "\t";
	} // for

	std::cout << std::endl;
} // function

/* Tricky extraction of the maximum among 8 parallel short-integers using 7 SIMD-statements.
 */
#define __max_8(ret, xx) do { \
	(xx) = _mm_max_epi16((xx), _mm_srli_si128((xx), 8)); \
	(xx) = _mm_max_epi16((xx), _mm_srli_si128((xx), 4)); \
	(xx) = _mm_max_epi16((xx), _mm_srli_si128((xx), 2)); \
	(ret) = _mm_extract_epi16((xx), 0); \
} while (0)

#if 0
const kswr_t g_defr = { 0, -1, -1, -1, -1, -1, -1 };
#endif

template<class T_scoring,			// the type that shall be used for the scoring values
		 class T_size_t>
struct SW_align_par_type 
{
	T_size_t lengthQuerySequence;
	T_size_t lengthOfSegment;

	T_scoring min_Score; 
	T_scoring max_Score, max;
	T_scoring bytesPerScore;

	ScoringLookupTable<T_scoring, T_size_t> scoringTable;

	SmithWatermanParamaterSet<T_scoring> &pSWparameterSetRef;
	/* values per __m128i (in the paper denoted p)
	 * 16 bit scoring -> 8 values in parallel
	 * 8  bit scoring -> 16 values in parallel
	 */
	static const int valuesPer128BitValue = sizeof(__m128i) / sizeof(T_scoring);

	__m128i *startOfReservedMemory;

	__m128i *startOfAllocatedMemory, *H_VectorPreviousRow, *H_VectorCurrentRow, *E, *H_VectorWithGlobalMaximum;
	
#if 0
	SW_align_par_type( )
	{

	} // default constructor
#endif

	/* Constructor
	 * querySequence is the "column sequence" at should be delivered as sequence of numbers that can be used for lookup operations in the scoringTable
	 */
	SW_align_par_type( T_size_t lengthQuerySequence, 
					   uint8_t *querySequence, 
					   SmithWatermanParamaterSet<T_scoring> &SWparameterSet, 
					   int sizeOfAlphabet 
					 ) 
		: scoringTable( SWparameterSet.xWeightMatch, SWparameterSet.xWeightMismatch ), 
		  lengthQuerySequence( lengthQuerySequence ),
		  pSWparameterSetRef( SWparameterSet )
	{	
		/* First we compute the size of all segments.
		 * ( We can have 16 or 8 or 4 segments! )
		 */
		lengthOfSegment = ( lengthQuerySequence + valuesPer128BitValue - 1 ) / valuesPer128BitValue;
		
		
		/* Allocation memory for the query profile and the 4 auxiliary vectors.
		 * (We allocate this memory as a single block!)
		 */
		startOfReservedMemory = (__m128i*)malloc(   256														 // space for alignment purposes. TO DO: Check me.
											      + sizeof(__m128i) * lengthOfSegment * (sizeOfAlphabet + 4) // +4 for H0, H1, E, MAX
												); 
		
		if ( startOfReservedMemory == NULL )
		{
			std::cout << "memory allocation failed" << lengthQuerySequence << std::endl;
		}
		
		/* Align with respect to the reserved memory for efficiency improvements.
		 */
		startOfAllocatedMemory = (__m128i*)(((T_size_t)startOfReservedMemory + 15) >> 4 << 4); 

		H_VectorPreviousRow = startOfAllocatedMemory + ( lengthOfSegment * sizeOfAlphabet );
		H_VectorCurrentRow = H_VectorPreviousRow + lengthOfSegment;
		E  = H_VectorCurrentRow + lengthOfSegment;
		H_VectorWithGlobalMaximum = E + lengthOfSegment;

		/* Initialize the Scoring Profile as described in the paper. 
		 * The Scoring Profile starts at the beginning of the allocated memory.
		 */
		T_scoring *profileReference = (T_scoring*)startOfAllocatedMemory;
		
		for( int alphabetIterator = 0; alphabetIterator < sizeOfAlphabet; ++alphabetIterator ) 
		{
			const T_scoring *scroringTableRow = scoringTable.pScoringTable + (alphabetIterator * sizeOfAlphabet);
			for( T_size_t i = 0; i < lengthOfSegment; ++i )
			{
				for( T_size_t k = i; k < lengthOfSegment * valuesPer128BitValue; k += lengthOfSegment ) // p iterations
				{
					/* The -100 for "non existing columns" has serious impact and isn't that neutral as someone could think.
					 * Don't take 0 here, because in this case we get faulty values.
					 * TO DO: -100 should be correct computed !!.
					 */
					*profileReference++ = ( k >= lengthQuerySequence ? -10000 // non existing column
													 			     : scroringTableRow[ querySequence[k] ]
										  );
				} // for k - iteration over p, where p = 16 or 8 or 4
			} // for i - iteration over segment size
		} // for alphabetIterator
	} // constructor

	/* The destructor deallocates all reserved memory.
	 */
	~SW_align_par_type()
	{
		std::cout << "release all memory" << std::endl;
		free( startOfReservedMemory );
	} // destructor
	
	/* The align method for SW-alignments - 16 bit version of the parallel SW implementation.
	 * The first gap costs -( _gapo + _gape) !
	 * The first row and column have both index 0 (important if a maximum is monitored in row 0)
	 * DatabaseSequence should be delivered in the already translated form.
	 * You have to deliver a fresh maxScoreProfiler
	 */
	T_scoring align16(	T_size_t	    lengthDataBaseSequence, 
						const uint8_t*	dataBaseSequence, 
						max_profiler<T_scoring, T_size_t> *maxScoreProfiler 
					 ) 
	{
		std::cout << "Start alignment" << std::endl;
		
		T_size_t rowOfGlobalMaxScore = 0;
		T_scoring globalMaxScore = 0;

		/* 128 bit variables
		 */
		__m128i zero = _mm_set1_epi32(0);
	
		__m128i gapoePar8; 
		__m128i gapePar8;
	
		/* Initialization of other stuff
		 */
		// kswr_t r = g_defr;
		
		T_scoring endsc = 0; // (xtra&KSW_XSTOP)? xtra&0xffff : 0x10000;
		T_scoring minsc = 0; // (xtra&KSW_XSUBO)? xtra&0xffff : 0x10000;

		gapoePar8 = _mm_set1_epi16( (short)( pSWparameterSetRef.gapo + pSWparameterSetRef.gape ) );
		gapePar8 = _mm_set1_epi16( (short)pSWparameterSetRef.gape );
		
		/* We initialize the E, H0 and Hmax vectors with zero.
		 * Idea: here we could take something build in function.
		 */
		for( T_size_t uxIterator = 0; uxIterator < lengthOfSegment; ++uxIterator ) 
		{
			_mm_store_si128(E + uxIterator, zero);
			
			/* In the first row we need 0 form the "previous row" !
			 */
			_mm_store_si128(H_VectorPreviousRow + uxIterator, zero);
			
			_mm_store_si128(H_VectorWithGlobalMaximum + uxIterator, zero);
		}

		/* The outer loop iterates over the matrix rows. (The database sequence)
		 * ??? The iteration start with zero, but we need finally one more, so <= lengthDataBaseSequence. ???
		 * Check the statement above.
		 */
		for( T_size_t uxIterator = 0; uxIterator < lengthDataBaseSequence; ++uxIterator ) 
		{
			__m128i e, h;
			__m128i f = zero; 
			__m128i vectorOfMaximaForCurrentRow = zero;

			/* We pick the row in the profile, which belongs to the current symbol previous
			 */
			__m128i *profileReferenceForRow = startOfAllocatedMemory + ( dataBaseSequence[uxIterator] * lengthOfSegment ); 

			h = _mm_load_si128(H_VectorPreviousRow + lengthOfSegment - 1); // h={2,5,8,11,14,17,-1,-1} in the above example
			h = _mm_slli_si128(h, 2);
		
			/* The inner loop iterates over the matrix columns
			 * Because of the segmentation it considers 8 values in parallel.
			 */
			for( T_size_t j = 0; j < lengthOfSegment; ++j )
			{
				/* SW cells are computed in the following order:
				 *   
				 *   F(i,j+1) = max{H(i,j)-q, F(i,j) - r}
				 */

				/* Compute H(i,j) = max{H(i-1,j-1)+S(i,j), E(i,j), F(i,j)} (parallel for 16/8/4 elements)
				 * Note that at the beginning, h = H(i - 1,j - 1)
				 */
				h = _mm_adds_epi16( h, *(profileReferenceForRow++) );
				e = _mm_load_si128( E + j );
				h = _mm_max_epi16( h, e );
				h = _mm_max_epi16( h, f );
				
				/* At this point we have the correct h-values in h, reagrding finding the maximum.
				 * However, f is not integrated here.
				 */
				vectorOfMaximaForCurrentRow = _mm_max_epi16( vectorOfMaximaForCurrentRow, h );
				_mm_store_si128(H_VectorCurrentRow + j, h);
				
				/* We compute and store the <e>-values for the next row.
				 * E(i+1, j) = max{H(i, j) - g_init, E(i, j) - g_ext}
				 */
				h = _mm_subs_epu16(h, gapoePar8);
				e = _mm_subs_epu16(e, gapePar8);
				e = _mm_max_epi16(e, h);
				_mm_store_si128(E + j, e);
				
				/* We compute the <f>-values (left cells). These values can be wrong, we correct this in the lazy-F loop. 
				 * F(i, j+1) = max{H(i, j) - g_init, F(i, j) - g_ext}
				 */
				f = _mm_subs_epu16(f, gapePar8);
				f = _mm_max_epi16(f, h);
				
				/* We load h for next iteration. Important is that we do this here and not at the beginning of the loop.
				 */
				h = _mm_load_si128(H_VectorPreviousRow + j);
			} // for j

			/* The lazy F-loop as described in the paper. This loop is responsible for fixing wrong f values.
			 * TO DO: Check, whether the 16 is ok. I think it should be replace by 8, because p is 8 here.
			 * This loop can become quite expensive in specific situations.
			 */
			for( int k = 0; k < 16; ++k ) 
			{
				f = _mm_slli_si128(f, 2);
				for( T_size_t j = 0; j < lengthOfSegment; ++j ) 
				{
					h = _mm_load_si128( H_VectorCurrentRow + j );
					h = _mm_max_epi16( h, f );
					_mm_store_si128( H_VectorCurrentRow + j, h );
					
					h = _mm_subs_epu16( h, gapoePar8 );
					f = _mm_subs_epu16( f, gapePar8 );
					if( !_mm_movemask_epi8( _mm_cmpgt_epi16( f, h ) ) ) 
					{
						goto exit_loop;
					}
				} // for j
			} // for k

			
		exit_loop:

#if 0
			/* For examples of up to 8 columns, we can see the correct h-vetor here
			 */
			vDump__m128i( *(H_VectorCurrentRow) );
#endif
			/* Extract the maximal score of the current row as a single element.
			 */
			T_scoring maxScoreInCurrentRow;

			/* The computed maximum here may be wrong, because it can belong to some non-existing column.
			 * TO DO: Fix this by writing 0 to the positions of non existing columns.
			 */
			__max_8( maxScoreInCurrentRow, vectorOfMaximaForCurrentRow );

#if 0
			/* We put our row-maximum into the max-score profile
			 * (i keeps the current row)
			 */
			std::cout << "push into profile:: score: " <<  maxScoreInCurrentRow << " row: " << uxIterator <<std::endl;
#endif

			maxScoreProfiler->push( maxScoreInCurrentRow, uxIterator ); 
			
			/* Management of the maxima
			 */
			if ( maxScoreInCurrentRow > globalMaxScore ) {
				

				globalMaxScore = maxScoreInCurrentRow; 
				rowOfGlobalMaxScore = uxIterator;
				
				/* This should be done by some memcpy!
				 * It copies the complete H-vector into the Hmax-vector
				 */
				for( T_size_t j = 0; j < lengthOfSegment; ++j )
				{
					_mm_store_si128( H_VectorWithGlobalMaximum + j, _mm_load_si128(H_VectorCurrentRow + j) );
				} // for j
				
				/* Here we could check whether the global maximum reached some critical threshold and stop.
				 */
			} // if
			
			/* Swap the current and the previous H-vector.
			 * TO DO: Use a tmp vector instead of this construct.
			 */
			profileReferenceForRow = H_VectorCurrentRow; 
			H_VectorCurrentRow = H_VectorPreviousRow; 
			H_VectorPreviousRow = profileReferenceForRow;
		} // for i
		
#if 0
		r.score = globalMaxScore; 
		r.te = rowOfGlobalMaxScore;

		{
			int max = -1, low, high, qlen = lengthOfSegment * 8;
			uint16_t *t = (uint16_t*)H_VectorWithGlobalMaximum;
			for (int i = 0, r.qe = -1; i < qlen; ++i, ++t)
				if ((int)*t > max) max = *t, r.qe = i / 8 + i % 8 * lengthOfSegment;
			if (b) {
				i = (r.score + this->max - 1) / vectorOfMaximaForCurrentRow;
				low = rowOfGlobalMaxScore - i; high = rowOfGlobalMaxScore + i;
				for (i = 0; i < lengthOf_b; ++i) {
					int e = (int32_t)b[i];
					if ((e < low || e > high) && (int)(b[i]>>32) > r.score2)
						r.score2 = b[i]>>32, r.te2 = e;
				}
			}
		}
		
#endif
		
		// return r;
		std::cout << "End alignment" << std::endl;

		return globalMaxScore;
	} // end alignment method
}; // end struct












#if 0
/* The align method for SW-alignments - 16 bit version of the parallel SW implementation.
	 * The first gap costs -( _gapo + _gape) !
	 * The first row and column have both index 0 (important if a maximum is monitored in row 0)
	 * DatabaseSequence should be delivered in the already translated form.
	 * You have to deliver a fresh maxScoreProfiler
	 */
	T_scoring align16(	T_size_t	    	lengthDataBaseSequence, 
						const uint8_t*		dataBaseSequence, 
						int			   		_gapo, 
						int			   		_gape,
						max_profiler<T_scoring, T_size_t> *maxScoreProfiler 
					) 
	{
		std::cout << "Start SSE alignment (original):" << lengthDataBaseSequence << std::endl;

		T_size_t rowOfGlobalMaxScore = 0;
		T_scoring globalMaxScore = 0;

		/* 128 bit variables
		 */
		__m128i zero = _mm_set1_epi32(0);
	
		__m128i gapoePar8; 
		__m128i gapePar8;
	
		/* Initialization of other stuff
		 */
		// kswr_t r = g_defr;
		
		//T_scoring endsc = 0; // (xtra&KSW_XSTOP)? xtra&0xffff : 0x10000;
		//T_scoring minsc = 0; // (xtra&KSW_XSUBO)? xtra&0xffff : 0x10000;

		gapoePar8 = _mm_set1_epi16( (short)(_gapo + _gape) );
		gapePar8 = _mm_set1_epi16( (short)_gape );
		
		/* We initialize the E, H0 and Hmax vectors with zero.
		 * Idea: here we could take something build in function.
		 */
		for( T_size_t i = 0; i < lengthOfSegment; ++i ) 
		{
			_mm_store_si128(E + i, zero);
			
			/* In the first row we need 0 form the "previous row" !
			 */
			_mm_store_si128(H_VectorPreviousRow + i, zero);
			
			_mm_store_si128(H_VectorWithGlobalMaximum + i, zero);
		}

		/* The outer loop iterates over the matrix rows. (The database sequence)
		 * The iteration start with zero not one!.
		 */
		for( T_size_t i = 0; i < lengthDataBaseSequence; ++i ) 
		{
			cout << "line:" << i << endl;
			__m128i e, h;
			__m128i f = zero; 
			__m128i vectorOfMaximaForCurrentRow = zero;

			/* We pick the row in the profile, which belongs to the current symbol previous
			 */
			__m128i *profileReferenceForRow = startOfAllocatedMemory + ( dataBaseSequence[i] * lengthOfSegment ); 

			h = _mm_load_si128(H_VectorPreviousRow + lengthOfSegment - 1); // h={2,5,8,11,14,17,-1,-1} in the above example
			h = _mm_slli_si128(h, 2);
		
			/* The inner loop iterates over the matrix columns
			 * Because of the segmentation it considers 8 values in parallel.
			 */
			for( T_size_t j = 0; j < lengthOfSegment; ++j )
			{
				/* SW cells are computed in the following order:
				 *   
				 *   F(i,j+1) = max{H(i,j)-q, F(i,j) - r}
				 */

				/* Compute H(i,j) = max{H(i-1,j-1)+S(i,j), E(i,j), F(i,j)} (parallel for 16/8/4 elements)
				 * Note that at the beginning, h = H(i - 1,j - 1)
				 */
				h = _mm_adds_epi16( h, *(profileReferenceForRow++) );
				e = _mm_load_si128( E + j );
				h = _mm_max_epi16( h, e );
				h = _mm_max_epi16( h, f );
				
				/* At this point we have the correct h-values in h.
				 */
				vectorOfMaximaForCurrentRow = _mm_max_epi16( vectorOfMaximaForCurrentRow, h );
				_mm_store_si128(H_VectorCurrentRow + j, h);
				
				/* We compute and store the <e>-values for the next row.
				 * E(i+1, j) = max{H(i, j) - g_init, E(i, j) - g_ext}
				 */
				h = _mm_subs_epu16(h, gapoePar8);
				e = _mm_subs_epu16(e, gapePar8);
				e = _mm_max_epi16(e, h);
				_mm_store_si128(E + j, e);
				
				/* We compute the <f>-values (left cells). These values can be wrong, we correct this in the lazy-F loop. 
				 * F(i, j+1) = max{H(i, j) - g_init, F(i, j) - g_ext}
				 */
				f = _mm_subs_epu16(f, gapePar8);
				f = _mm_max_epi16(f, h);
				
				/* We load h for next iteration. Important is that we do this here and not at the beginning of the loop.
				 */
				h = _mm_load_si128(H_VectorPreviousRow + j);
			} // for j
			/* The lazy F-loop as described in the paper. This loop is responsible for fixing wrong f values.
			 * TO DO: Check, whether the 16 is ok. I think it should be replace by 8, because p is 8 here.
			 * This loop can become quite expensive in specific situations.
			 */
			for( int k = 0; k < 16; ++k ) 
			{
				f = _mm_slli_si128(f, 2);
				for( T_size_t j = 0; j < lengthOfSegment; ++j ) 
				{
					h = _mm_load_si128( H_VectorCurrentRow + j );
					h = _mm_max_epi16( h, f );
					_mm_store_si128( H_VectorCurrentRow + j, h );
					
					h = _mm_subs_epu16( h, gapoePar8 );
					f = _mm_subs_epu16( f, gapePar8 );
					if( !_mm_movemask_epi8( _mm_cmpgt_epi16( f, h ) ) ) 
					{
						goto exit_loop;
					}
				} // for j
			} // for k
	exit_loop:
			/* Extract the maximal score of the current row as a single element.
			 */
			T_scoring maxScoreInCurrentRow;

#if ( DUMP_MATRIX == 1 )
			std::cout << "dump matrix" << std::endl;
			union {
				__m128i value;    
				int16_t arr[8];  
			} converter;
			converter.value = vectorOfMaximaForCurrentRow;

			for (int iterator = 0; iterator < 8; iterator++)
			{
				std::cout << converter.arr[iterator] << " ";
			}
			std::cout << std::endl;
#endif

			/* The computed maximum here may be wrong, because it can belong to some non-existing column.
			 * TO DO: Fix this by writing 0 to the positions of non existing columns.
			 */
			__max_8( maxScoreInCurrentRow, vectorOfMaximaForCurrentRow );

			/* We put our row-maximum into the max-score profile
			 * (i keeps the current row)
			 */
			std::cout << "push into profile:: score: " <<  maxScoreInCurrentRow << " row: " << i <<std::endl;

			maxScoreProfiler->push( maxScoreInCurrentRow, i ); 
			
			/* Management of the maxima
			 */
			if ( maxScoreInCurrentRow > globalMaxScore ) {
				

				globalMaxScore = maxScoreInCurrentRow; 
				rowOfGlobalMaxScore = i;
				
				/* This should be done by some memcpy!
				 * It copies the complete H-vector into the Hmax-vector
				 */
				for( T_size_t j = 0; j < lengthOfSegment; ++j )
				{
					_mm_store_si128( H_VectorWithGlobalMaximum + j, _mm_load_si128(H_VectorCurrentRow + j) );
				} // for j
				
				/* Here we could check whether the global maximum reached some critical threshold and stop.
				 */
			} // if
	
			/* Swap the current and the previous H-vector.
			 * TO DO: Use a tmp vector instead of this construct.
			 */
			profileReferenceForRow = H_VectorCurrentRow; 
			H_VectorCurrentRow = H_VectorPreviousRow; 
			H_VectorPreviousRow = profileReferenceForRow;
		} // for i
		
#if 0
		r.score = globalMaxScore; 
		r.te = rowOfGlobalMaxScore;

		{
			int max = -1, low, high, qlen = lengthOfSegment * 8;
			uint16_t *t = (uint16_t*)H_VectorWithGlobalMaximum;
			for (int i = 0, r.qe = -1; i < qlen; ++i, ++t)
				if ((int)*t > max) max = *t, r.qe = i / 8 + i % 8 * lengthOfSegment;
			if (b) {
				i = (r.score + this->max - 1) / vectorOfMaximaForCurrentRow;
				low = rowOfGlobalMaxScore - i; high = rowOfGlobalMaxScore + i;
				for (i = 0; i < lengthOf_b; ++i) {
					int e = (int32_t)b[i];
					if ((e < low || e > high) && (int)(b[i]>>32) > r.score2)
						r.score2 = b[i]>>32, r.te2 = e;
				}
			}
		}
		
#endif
		// return r;
		std::cout << "End alignment" << std::endl;

		return globalMaxScore;
	} // end alignment method
#endif

