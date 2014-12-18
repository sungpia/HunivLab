#pragma once

#include <windows.h>
#include <vector>

/* T_scoring is the type used for scoring, this should be a 16bit type here.
 * The row sequence is represented by some sequence of 8-bit numbers.
 */
template<class T_scoring,
		 class T_size_t>
class SmithWatermanSSE2 {
public:
	/* TO DO: Later insert an leadin and leadout section.
	 * !! Variables of type _m128i are automatically aligned on 16-byte boundaries. !!
	 */


	T_scoring align16(	T_size_t	    uxLengthRowSequence, 
						const uint8_t*	pRowSequence, 
						int			   _gapo, 
						int			   _gape,
						// max_profiler<T_scoring, T_size_t> *maxScoreProfiler 
					 ) 
	{
		// The two void-vectors for initialization purposes.
		__m128i const m128_VOID_CODE_ROW_VECTOR = _mm_set1_epi16( (short)DEF_VOID_CODE_ROW );
		__m128i const m128_VOID_CODE_COLUMN_VECTOR = _mm_set1_epi16( (short)DEF_VOID_CODE_COLUMN );
		__m128i const m128_MATCH_WEIGHTS_VECTOR = _mm_set1_epi16( (short)10 );
		__m128i const m128_MISMATCH_WEIGHTS_VECTOR = _mm_set1_epi16( (short)-3 );
		__m128i const m128_ZERO_VECTOR = _mm_setzero_si128();
		
		// START ROW ITERATION
			// Initialize the row-vector with row elements
			__m128i parRowVector;

			/* Initialize the column vector with void codes. (The front elements)
			 * IMPORTANT: The column vector stays always reverted !
			 */
			__m128i parColumnVector = m128_VOID_CODE_COLUMN_VECTOR; 
			
			// START COLUMN ITERATION
			while()
				/* 1. The left right shifting of the column sequence by 2 bytes 
				 * _mm_srli_si128(__m128i a, int imm):
				 * Shifts the 128-bit value in a right by imm bytes while shifting in zeros. imm must be an immediate.
				 */
				parColumnVector = _mm_srli_si128( parColumnVector, 2);

				/* 2. The insertion of a fresh sequence element in the column vector.
				 * __m128i _mm_or_si128 (__m128i a, __m128i b):
				 * Computes the bitwise OR of the 128-bit value in a and the 128-bit value in b.
				 */
				parColumnVector = _mm_or_si128( parColumnVector, parColumnVector /* HERE COMES THE FRESH COLUMN ELEMENT */ );
						
				/* 5 SSE INSTRUCTIONS FOR THE COMPUTATION OF THE WEIGHT VECTOR.
				 *
				 * __m128i _mm_sub_epi16 (__m128i a, __m128i b):
				 * Subtracts the 8 signed or unsigned 16-bit integers of b from the 8 signed or unsigned 16-bit integers of a.
				 * parWeights = parRowElements - parColumnElements;
				 */
				__m128i parDifferenceVector = _mm_sub_epi16( parColumnVector, parRowVector );

				/* __m128i _mm_cmpeq_epi16 (__m128i a, __m128i b):
				 * Compares the 8 signed or unsigned 16-bit integers in a and the 8 signed or unsigned 16-bit integers in b for equality.
				 */
				__m128i parBitMaskVector = _mm_cmpeq_epi16( parDifferenceVector, m128_ZERO_VECTOR );

				/* __m128i _mm_and_si128 (__m128i a, __m128i b):
				 * Computes the bitwise AND of the 128-bit value in a and the 128-bit value in b.
				 */
				__m128i parMatchWeightsVector = _mm_and_si128( parBitMaskVector, m128_MATCH_WEIGHTS_VECTOR );

				/* __m128i _mm_andnot_si128 (__m128i a, __m128i b):
				 * Computes the bitwise AND of the 128-bit value in b and the bitwise NOT of the 128-bit value in a.
				 */
				__m128i parMismatchWeightsVector = _mm_andnot_si128( parBitMaskVector, m128_MISMATCH_WEIGHTS_VECTOR );

				/* __m128i _mm_or_si128 (__m128i a, __m128i b);
				 * Computes the bitwise OR of the 128-bit value in a and the 128-bit value in b.
				 * pasWeightsVector = parMatchWeightsVector | parMismatchWeightsVector;
				 */
				__m128i parWeightsVector = _mm_or_si128( parMatchWeightsVector, parMismatchWeightsVector );

				// COMPUTE THE E_i_j ON THE FOUNDATION OF THE INACTIVE H VECTOR

				// SHIFT THE INACTIVE H VECTOR, STORE LEFTMOST ELEMENT, LOAD THE RIGHTMOST ELEMENT



			// END COLUMN ITERATION
		// END ROW ITERATION



	} // method


		/* The align method for SW-alignments - 16 bit version of the parallel SW implementation.
	 * The first gap costs -( _gapo + _gape) !
	 * The first row and column have both index 0 (important if a maximum is monitored in row 0)
	 * DatabaseSequence should be delivered in the already translated form.
	 * You have to deliver a fresh maxScoreProfiler
	 */
	T_scoring align16SSE2(	T_size_t	    lengthDataBaseSequence, 
							const uint8_t*	dataBaseSequence, 
							int			   _gapo, 
							int			   _gape,
							max_profiler<T_scoring, T_size_t> *maxScoreProfiler 
						) 
	{
		std::cout << "Start SSE alignment (new)" << std::endl;
		
		register __m128i parRowVector  = _mm_set1_epi32(0);
		register __m128i parColumnVector  = _mm_set1_epi32(0); 
		
		const __m128i m128_MATCH_WEIGHTS_VECTOR = _mm_set1_epi16( (short)10 );
		const __m128i m128_MISMATCH_WEIGHTS_VECTOR = _mm_set1_epi16( (short)-3 );
		
		const __m128i m128_ZERO_VECTOR = _mm_set1_epi32(0);
		
		const __m128i parVectorGEXT = _mm_set1_epi16( (short)-7 );
		const __m128i parVectorGINIT =  _mm_set1_epi16( (short)-3 );

		__m128i parGlobalMaxVector  = _mm_set1_epi32(0);		
		
		register __m128i parVectorE = _mm_set1_epi32(0);
		register __m128i parVectorF = _mm_set1_epi32(0);
		register __m128i parVectorHinactive = _mm_set1_epi32(0);
		register __m128i parVectorHactive = _mm_set1_epi32(0);
		register __m128i parVectorRegister = _mm_set1_epi32(0);

		// int F_store ;
		// int *F_storePtr = &F_store;

		// int H_store ;
		// int *H_storePtr = &H_store;

		/* The outer loop iterates over the matrix rows. (The database sequence)
		 * The iteration start with zero not one!.
		 */
		for( T_size_t i = 0; i < (lengthDataBaseSequence / 8) + 1; ++i ) 
		{
			__m128i parRowVector = _mm_load_si128( startOfAllocatedMemory );
			
			/* The inner loop iterates over the matrix columns
			 * Because of the segmentation it considers 8 values in parallel.
			 */
			for( T_size_t j = 0; j < lengthOfSegment + 1; ++j )
			{				
				/* read the next 8 element into the data vector.
				 */
				 __m128i parVectorColumnData = _mm_load_si128( startOfAllocatedMemory + j );
				 __m128i parVectorHData = _mm_load_si128( startOfAllocatedMemory + lengthOfSegment + j );
				
				for( T_size_t k = 0; k < 8; k++ )
				{
					/* Step 1
					 * A) The left shifting of the column sequence by 2 bytes 
					 * _mm_srli_si128(__m128i a, int imm):
					 * Shifts the 128-bit value in a left by imm bytes while shifting in zeros. imm must be an immediate.
					 */
					parColumnVector = _mm_slli_si128( parColumnVector, 2);

					/* Copy the least 16 bit of parVectorColumnData
					 */
#if CONF_NO_BLEND
					parVectorRegister = _mm_and_si128( parVectorColumnData, maskVector ); 
					parColumnVector =  _mm_or_si128( parColumnVector, parVectorRegister);
#else
					parColumnVector = _mm_blend_epi16( parColumnVector, parVectorColumnData, 0x03 );
#endif
					
					/* We shift the column data-vector by two positions
					 */
					parVectorColumnData = _mm_srli_si128( parVectorColumnData, 2);
					
					// parColumnVector =_mm_insert_epi16 ( parColumnVector,(* (int *)(startOfAllocatedMemory + j)), 0 );


					/* Step 2
					 * Compute the weight vector
					 * __m128i _mm_cmpeq_epi16 (__m128i a, __m128i b):
					 * Compares the 8 signed or unsigned 16-bit integers in a and the 8 signed or unsigned 16-bit integers in b for equality.
					 */
					__m128i parBitMaskVector = _mm_cmpeq_epi16( parColumnVector, parRowVector );

#if CONF_NO_BLEND
					/* (BLEND A) __m128i _mm_and_si128 (__m128i a, __m128i b):
					 * Computes the bitwise AND of the 128-bit value in a and the 128-bit value in b.
					 */
					__m128i parMatchWeightsVector = _mm_and_si128( parBitMaskVector, m128_MATCH_WEIGHTS_VECTOR );

					/* (BLEND B) __m128i _mm_andnot_si128 (__m128i a, __m128i b):
					 * Computes the bitwise AND of the 128-bit value in b and the bitwise NOT of the 128-bit value in a.
					 */
					__m128i parMismatchWeightsVector = _mm_andnot_si128( parBitMaskVector, m128_MISMATCH_WEIGHTS_VECTOR );

					/* (BLEND C) __m128i _mm_or_si128 (__m128i a, __m128i b);
					 * Computes the bitwise OR of the 128-bit value in a and the 128-bit value in b.
					 * pasWeightsVector = parMatchWeightsVector | parMismatchWeightsVector;
					 */
					__m128i parWeightsVector = _mm_or_si128( parMatchWeightsVector, parMismatchWeightsVector );
#else
					__m128i parWeightsVector = _mm_blendv_epi8(  m128_MATCH_WEIGHTS_VECTOR, m128_MISMATCH_WEIGHTS_VECTOR, parBitMaskVector );
#endif
	
					/* Step 3
					 * We compute the E-vector in parallel on the foundation of the inactive H vector.
					 *
					 * __m128i _mm_abs_epi16( __m128i a );
					 * SIMD Extensions 3 (SSSE3) instruction 
					 * This instruction returns the absolute value for each packed 16-bit integer of the 128-bit parameter.
					 */
					parVectorRegister = _mm_abs_epi16( parVectorHinactive );
					parVectorE = _mm_sub_epi16( parVectorE, parVectorGEXT );
					parVectorRegister = _mm_sub_epi16( parVectorRegister, parVectorGINIT );
					parVectorE = _mm_max_epi16 ( parVectorE, parVectorRegister );

					/* Step 4
					 * We shift the inactive H vector and insert the H from the previous iteration carried over
					 * In the context of this operation we store one fresh element
					 * 	*H_storePtr =_mm_extract_epi16 ( parVectorHinactive, 7);
					 */
					parVectorHinactive = _mm_blend_epi16( parVectorHinactive, parVectorHData, 0x03 );

					/* We shift the column data-vector by two positions
					 * _mm_insert_epi16 ( parVectorHinactive, *F_storePtr, 0 );
					 */
					parVectorHData = _mm_srli_si128(  parVectorHData, 2);
					parVectorHinactive = _mm_slli_si128  (parVectorHinactive, 2 );

					/* Step 5
					 * We compute the basic form of Hij by subtracting the weights in parallel (the vector contains the required Hi-1,j-1.
					 * Get the provisional Hi,j by 2 parallel-max operations.
					 */
					parVectorHactive = _mm_sub_epi16( parVectorHactive, parWeightsVector );
					parVectorRegister = _mm_max_epi16( parVectorE, m128_ZERO_VECTOR );
					parVectorHactive = _mm_max_epi16( parVectorHactive, parVectorRegister );

					/* Step 6
					 * We compute the Fij
					 *
					 * __m128i _mm_cmplt_epi16 (__m128i a, __m128i b);
					 * Compares the 8 signed 16-bit integers in a and the 8 signed 16-bit integers in b for less than.
					 * r0 := (a0 < b0) ? 0xffff : 0x0
					 */
					parVectorHinactive = _mm_abs_epi16( parVectorHinactive );
					parBitMaskVector = _mm_cmplt_epi16( parVectorHinactive, m128_ZERO_VECTOR );

#if CONF_NO_BLEND					
					/* Blend operation
					 * (BLEND A) 
					 */
					parMatchWeightsVector = _mm_and_si128( parBitMaskVector, parVectorGEXT );

					/* (BLEND B) __m128i _mm_andnot_si128 (__m128i a, __m128i b):
					 * Computes the bitwise AND of the 128-bit value in b and the bitwise NOT of the 128-bit value in a.
					 */
					parMismatchWeightsVector = _mm_andnot_si128( parBitMaskVector, parVectorGINIT );

					/* (BLEND C) __m128i _mm_or_si128 (__m128i a, __m128i b);
					 * Computes the bitwise OR of the 128-bit value in a and the 128-bit value in b.
					 * pasWeightsVector = parMatchWeightsVector | parMismatchWeightsVector;
					 */
					parWeightsVector = _mm_or_si128( parMatchWeightsVector, parMismatchWeightsVector );
#else
					parBitMaskVector = _mm_cmplt_epi16( parVectorHinactive, m128_ZERO_VECTOR );
					parWeightsVector = _mm_blendv_epi8( parVectorGEXT, parVectorGINIT, parBitMaskVector );
#endif

					/* This is now the F-vector
					 */
					parVectorRegister = _mm_sub_epi16( parVectorHinactive, parWeightsVector );

					/* Step 7
					 * We compute the fresh Hij .
					 */
					parBitMaskVector = _mm_cmpgt_epi16( parVectorHactive, parVectorRegister );
					parVectorHactive = _mm_max_epi16( parVectorHactive, parVectorRegister );
					parWeightsVector = _mm_blendv_epi8( parVectorHactive, parVectorRegister, parBitMaskVector );
					
					/*__m128i _mm_sign_epi16( __m128i a, __m128i b);
					 * (SSSE3) This instruction negates or clears 16-bit signed integers of a 128 bit parameter.
					 * r0 := (b0 < 0) ? -a0 : ((b0 == 0) ? 0 : a0)
					 */
					parBitMaskVector = _mm_cmpgt_epi16( parVectorHactive, parVectorRegister );
					parVectorHactive = _mm_sign_epi16( parVectorHactive, parBitMaskVector );
					
					/* SWAP
					 */
					parVectorRegister = parVectorHactive;
					parVectorHactive = parVectorHinactive;
					parVectorHinactive = parVectorRegister;
				} // for k
				
			_mm_store_si128( startOfAllocatedMemory + j, parVectorHactive );	
			} // for j
			
		} // for i	

		std::cout << "End alignment" << std::endl;
		return 0; // globalMaxScore;
	} // end alignment method
	
	/* The align method for SW-alignments - 16 bit version of the parallel SW implementation.
	 * The first gap costs -( _gapo + _gape) !
	 * The first row and column have both index 0 (important if a maximum is monitored in row 0)
	 * DatabaseSequence should be delivered in the already translated form.
	 * You have to deliver a fresh maxScoreProfiler
	 */
	T_scoring align16SSE2B(	T_size_t	    lengthDataBaseSequence, 
							const uint8_t*	dataBaseSequence, 
							int			   _gapo, 
							int			   _gape,
							max_profiler<T_scoring, T_size_t> *maxScoreProfiler 
						) 
	{
		std::cout << "Start SSE alignment (new)" << std::endl;
		
		__m128i parRowVector  = _mm_set1_epi32(0);
		__m128i parColumnVector  = _mm_set1_epi32(0); 
		
	
		__m128i m128_ZERO_VECTOR = _mm_set1_epi32(0);
		// __m128i m128_MATCH_WEIGHTS_VECTOR = _mm_set1_epi16( (short)10 );
		// __m128i m128_MISMATCH_WEIGHTS_VECTOR = _mm_set1_epi16( (short)-3 );
		__m128i parVectorGEXT = _mm_set1_epi16( (short)-7 );
		__m128i parVectorGINIT =  _mm_set1_epi16( (short)-3 );

		// __m128i parWeightsVector = _mm_set1_epi32(0);
		// __m128i maskVector = _mm_setzero_si128();

		__m128i parGlobalMaxVector  = _mm_set1_epi32(0);		
		
		__m128i parVectorE = _mm_set1_epi32(0);
		__m128i parVectorF = _mm_set1_epi32(0);
		__m128i parVectorHinactive = _mm_set1_epi32(0);
		__m128i parVectorHactive = _mm_set1_epi32(0);
		__m128i parVectorTmp = _mm_set1_epi32(0);

		int F_store ;
		int *F_storePtr = &F_store;

		int H_store ;
		int *H_storePtr = &H_store;

		
		__m128i *storePtr128 = &store128;
		__m128i parVectorColumnData= m128_ZERO_VECTOR;

		/* The outer loop iterates over the matrix rows. (The database sequence)
		 * The iteration start with zero not one!.
		 */
		for( T_size_t i = 0; i < (lengthDataBaseSequence / 8) + 1; ++i ) 
		{
			/* The inner loop iterates over the matrix columns
			 * Because of the segmentation it considers 8 values in parallel.
			 */
			for( T_size_t j = 0; j < (lengthOfSegment + 1); ++j )
			{				
				parVectorColumnData = _mm_load_si128( storePtr128 );
				
				for( T_size_t k = 0; k < 8; k++ )
				{
					/* Step 1
					 * A) The left shifting of the column sequence by 2 bytes 
					 * _mm_srli_si128(__m128i a, int imm):
					 * Shifts the 128-bit value in a left by imm bytes while shifting in zeros. imm must be an immediate.
					 */
					// parColumnVector = _mm_slli_si128( parColumnVector, 2);

					/* B) The insertion of a fresh sequence element in the column vector.
					 */
#if 0
					parColumnVector = _mm_insert_epi16( parColumnVector, *F_storePtr, 0 );
#endif
					/* We get the least 16 bit
					 */
#if CONF_NO_BLEND
					parVectorTmp = _mm_and_si128( parVectorColumnData, maskVector ); 
					parColumnVector =  _mm_or_si128( parColumnVector, parVectorTmp);
#else
					parColumnVector = _mm_slli_si128( parColumnVector, 2);
					parColumnVector = _mm_blend_epi16( parColumnVector, parVectorColumnData, 0x03 );
#endif
					/* We shift the column data-vector by two positions
					 */
					parVectorColumnData = _mm_srli_si128( parVectorColumnData, 2);

#if CONF_NO_BLEND
					/* Step 2
					 * Compute the weight vector
					 * __m128i _mm_cmpeq_epi16 (__m128i a, __m128i b):
					 * Compares the 8 signed or unsigned 16-bit integers in a and the 8 signed or unsigned 16-bit integers in b for equality.
					 */
					__m128i parBitMaskVector = _mm_cmpeq_epi16( parColumnVector, parRowVector );


					/* (BLEND A) __m128i _mm_and_si128 (__m128i a, __m128i b):
					 * Computes the bitwise AND of the 128-bit value in a and the 128-bit value in b.
					 */
					__m128i parMatchWeightsVector = _mm_and_si128( parBitMaskVector, m128_MATCH_WEIGHTS_VECTOR );

					/* (BLEND B) __m128i _mm_andnot_si128 (__m128i a, __m128i b):
					 * Computes the bitwise AND of the 128-bit value in b and the bitwise NOT of the 128-bit value in a.
					 */
					__m128i parMismatchWeightsVector = _mm_andnot_si128( parBitMaskVector, m128_MISMATCH_WEIGHTS_VECTOR );

					/* (BLEND C) __m128i _mm_or_si128 (__m128i a, __m128i b);
					 * Computes the bitwise OR of the 128-bit value in a and the 128-bit value in b.
					 * pasWeightsVector = parMatchWeightsVector | parMismatchWeightsVector;
					 */
					__m128i parWeightsVector = _mm_or_si128( parMatchWeightsVector, parMismatchWeightsVector );
#else
					__m128i parWeightsVector = _mm_blendv_epi8(  _mm_set1_epi16( (short)10 ),
																 _mm_set1_epi16( (short)-3 ), 
																 _mm_cmpeq_epi16( parColumnVector, parRowVector ) // parBitMaskVector
															  );
#endif
	
					/* Step 3
					 * We compute the E-vector in parallel on the foundation of the inactive H vector.
					 *
					 * __m128i _mm_abs_epi16( __m128i a );
					 * SIMD Extensions 3 (SSSE3) instruction 
					 * This instruction returns the absolute value for each packed 16-bit integer of the 128-bit parameter.
					 */
					parVectorTmp = _mm_abs_epi16(  parVectorHinactive );
				
					parVectorE = _mm_sub_epi16( parVectorE, parVectorGEXT );
					parVectorTmp = _mm_sub_epi16( parVectorTmp, parVectorGINIT );
					parVectorE = _mm_max_epi16 ( parVectorE, parVectorTmp );

					/* Step 4
					 * We shift the inactive H vector and insert the H from the previous iteration carried over
					 * In the context of this operation we store one fresh element
					 */
					if (k == 0)
					{
						*H_storePtr =_mm_extract_epi16 ( parVectorHinactive, 7);
					}
					parVectorHinactive = _mm_slli_si128  (parVectorHinactive, 2 );
					if (k == 0)
					{
						parVectorHinactive =_mm_insert_epi16 ( parVectorHinactive, *F_storePtr, 0 );
					}

					/* Step 5
					 * We compute the basic form of Hij by subtracting the weights in parallel (the vector contains the required Hi-1,j-1.
					 * Get the provisional Hi,j by 2 parallel-max operations.
					 */
					
					// parVectorHactive = _mm_sub_epi16( parVectorHactive, parWeightsVector );
					// parVectorTmp = _mm_max_epi16( parVectorE, m128_ZERO_VECTOR );
					parVectorHactive = _mm_max_epi16( _mm_sub_epi16( parVectorHactive, parWeightsVector ), _mm_max_epi16( parVectorE, m128_ZERO_VECTOR ) );

					/* Step 6
					 * We compute the Fij
					 *
					 * __m128i _mm_cmplt_epi16 (__m128i a, __m128i b);
					 * Compares the 8 signed 16-bit integers in a and the 8 signed 16-bit integers in b for less than.
					 * r0 := (a0 < b0) ? 0xffff : 0x0
					 */
					
					parVectorHinactive = _mm_abs_epi16(  parVectorHinactive );

#if CONF_NO_BLEND
					parBitMaskVector = _mm_cmplt_epi16( parVectorHinactive, m128_ZERO_VECTOR );
					/* Blend operation
					 * (BLEND A) 
					 */
					parMatchWeightsVector = _mm_and_si128( parBitMaskVector, parVectorGEXT );

					/* (BLEND B) __m128i _mm_andnot_si128 (__m128i a, __m128i b):
					 * Computes the bitwise AND of the 128-bit value in b and the bitwise NOT of the 128-bit value in a.
					 */
					parMismatchWeightsVector = _mm_andnot_si128( parBitMaskVector, parVectorGINIT );

					/* (BLEND C) __m128i _mm_or_si128 (__m128i a, __m128i b);
					 * Computes the bitwise OR of the 128-bit value in a and the 128-bit value in b.
					 * pasWeightsVector = parMatchWeightsVector | parMismatchWeightsVector;
					 */
					parWeightsVector = _mm_or_si128( parMatchWeightsVector, parMismatchWeightsVector );
#else
					parWeightsVector = _mm_blendv_epi8( parVectorGEXT, parVectorGINIT, _mm_cmplt_epi16( parVectorHinactive, m128_ZERO_VECTOR ) );
#endif

					/* This is now the F-vector
					 */
					parVectorTmp = _mm_sub_epi16( parVectorHinactive, parWeightsVector );

					/* Step 7
					 * We compute the fresh Hij .
					 */
					// parBitMaskVector = _mm_cmpgt_epi16( parVectorHactive, parVectorTmp );
#if 1
					parVectorHactive = _mm_max_epi16( parVectorHactive, parVectorTmp );
#else
					parWeightsVector = _mm_blendv_epi8( parVectorHactive, parVectorTmp, parBitMaskVector );
#endif
					/* 
					 * __m128i _mm_sign_epi16( __m128i a, __m128i b);
					 * (SSSE3) This instruction negates or clears 16-bit signed integers of a 128 bit parameter.
					 * r0 := (b0 < 0) ? -a0 : ((b0 == 0) ? 0 : a0)
					 */
					parVectorHactive = _mm_sign_epi16( parVectorHactive, 
													   _mm_cmpgt_epi16( parVectorHactive, parVectorTmp )  //    parBitMaskVector 
													 );

					/* SWAP
					 */
	#if 1
					parVectorTmp = parVectorHactive;
					parVectorHactive = parVectorHinactive;
					parVectorHinactive = parVectorTmp;
	#endif

				} // for k
			} // for j
		} // for i	

		std::cout << "End alignment" << std::endl;
		return 0; // globalMaxScore;
	} // end alignment method
};