#pragma once

#include "support.h"
#include <iostream>
#include <scoring_matrix.h>
//  #include "fasta.h"

#if _MSC_VER
	#include <windows.h>
#endif


#if 0
template <class T>
void reverse(T word[], size_t length)
{
	char temp;
	for ( size_t i = 0; i < length / 2; i++ )
	{
		temp = word[i];
		word[i] = word[length - i - 1 ];
		word[length - i - 1] = temp;
	}
} // reverse


/* Wrapper class for a c-driven Fasta file reader.
 */
class UnmanagedFastaReader
{
private:
	
	
public:
	fasta_record_t *pFastaRecord;
	/* The default constructor.
	 */
	UnmanagedFastaReader() : pFastaRecord(NULL)
	{ }

	void inverse()
	{
		inverseSequence( pFastaRecord->sequence.text, pFastaRecord->sequence.length );
		reverse( pFastaRecord->sequence.text, pFastaRecord->sequence.length );
	}

	/* The central load function
	 */
#if _MSC_VER
	errno_t load( char *pFileName )
#else
	void load( char *pFileName )
#endif
	{
		FILE *pFileHandle;
		std::cout << "In C++ library: open File:" << pFileName << ":" << std::endl;

#if _MSC_VER
		errno_t io_error = fopen_s( &pFileHandle, pFileName, "r" );
#else
		pFileHandle = fopen( pFileName, "r" );
#endif

#if _MSC_VER		
		if ( !io_error )
#else
	if ( pFileHandle != NULL )
#endif			
		{
			/* Allocate basic memory for the fasta record.
			 */

			pFastaRecord = kseq_init( pFileHandle );

			/* read the fasta record, here it can come to automatic memory reallocation.
			 * After reading translate the characters into a sequence of numbers.
			 */
			kseq_read( pFastaRecord );
			// translateSequence( pFastaRecord->sequence.text );

			/* We are done with all file reading, so we close the file immediately.
			 */
			fclose( pFileHandle );
		} // if
		
		std::cout << "length:" << pFastaRecord->sequence.length << std::endl;

#if _MSC_VER	
		std::cout << "io_error is:" << io_error << std::endl;
		
		return io_error;
#endif
	} // method

#if 0
	/* WARNING: Here you can get a NULL pointer if something went wrong.
	 */
	uint8_t* getSequenceReference() 
	{
		return ( pFastaRecord != NULL ? (uint8_t *)pFastaRecord->sequence.text
								      : NULL );
	} // method
#endif		

	~UnmanagedFastaReader()
	{
		/* We free all the resources of the reader
		 */
		if ( pFastaRecord != NULL )
		{
			std::cout << "Release all fasta resources" << std::endl;
			kseq_destroy( pFastaRecord );
		}
	} // destructor
}; // class
#endif

template<class T_scoring>
struct generic_eh_type {
	/* 2 times 4 bytes = altogether 8 bytes
	 */
	T_scoring h, e;
};

/* queryOutcomeMatrix must have the size (numberOfRows + 1) x (numberOfColumns + 1)
 * CONF_FILL_OUTCOME_MATRIX if you deliver true here, then the code for filling the outcome matrix is included
 */
template<bool CONF_FILL_OUTCOME_MATRIX, // Compile switch, that decides about filling outcome matrix
         class T_scoring,				// the type that shall be used for the scoring
		 class T_size_t
        >
struct SW_align_type
{
	T_size_t numberOfColumns; const uint8_t *columnSequence;
	T_size_t numberOfRows; const uint8_t *rowSequence;
	
	/* number of symbols in your alphabet (don't forget +1 -> A,C,G,T => 5)
	 */
	static const int alphabetSize = 5; 

	/* Local sub-object that is initialized in the context of the default constructor.
	 */
	scoring_matrix<T_scoring, T_size_t> scoringMatrix;

	ScoringLookupTable<T_scoring, T_size_t> scoringTable;

	SmithWatermanParamaterSet<T_scoring> &pSWparameterSetRef;

	/* The parameter
	 * columnSequence : already translated sequence of numbers, representing the query sequence
	 * rowSequence	  : already translated sequence of numbers, representing the database sequence
	 * gapo			  : gap penalty for introducing a gap (together with gape)
	 * gape			  : gap penalty for extending a gap
	 */
	SW_align_type( int numberOfColumns, const uint8_t *columnSequence, 
				   int numberOfRows, const uint8_t *rowSequence, 
				   SmithWatermanParamaterSet<T_scoring> &SWparameterSet
				   ) : pSWparameterSetRef( SWparameterSet ),
					   
					   /* Initialize the scoring matrix
					    */
					   scoringMatrix(numberOfColumns, columnSequence, numberOfRows, rowSequence ),
				       
					   numberOfColumns(numberOfColumns),
					   numberOfRows(numberOfRows),
					   columnSequence(columnSequence),
					   rowSequence(rowSequence),
					   scoringTable( SWparameterSet.xWeightMatch, SWparameterSet.xWeightMismatch )
	{
		std::cout << "SW_serial_align - numberOfRows: " << numberOfRows << std::endl;
	}

	T_scoring swAlign( 
	#if (CONF_BAND_LIMITATION == 1)
					int w, 
	#endif
				   
	#if (CONF_SET_INITIAL_SCORE_VALUE == 1)
					int initalScoreValue,
	#endif
				T_size_t *pColumnIndexOfMaxScore, 
				T_size_t *pRowIndexOfMaxScore,
				T_size_t &indexOfMaxElementInScoringTable
			  )
	{
		T_size_t i, j, start, end;
		T_scoring maxScoreValue;
		int rowIndexOfMaxScore, columnIndexOfMaxScore; 
		
		T_scoring gapoe = pSWparameterSetRef.gapo + pSWparameterSetRef.gape;
		
		int scoreTableRowIterator;
		int queryProfileIterator;

#if 0
		std::cout << "One element of scoring values has " << sizeof(T_scoring) << " bytes" << std::endl;
#endif

	#if (CONF_BAND_LIMITATION == 1)
		int sizeOfMatrix, upperLimitOfGapValue;
	#endif

	#if (CONF_SET_INITIAL_SCORE_VALUE == 1)
		if (initalScoreValue < 0)
			initalScoreValue = 0;
	#endif
		
		/* allocate memory
		 * 1. Query Profile (see ppt-file)
		 * 2. h_and_e_Columns : Two one dimensional array for intermediate data storage (see ppt-file)
		 */
		T_scoring* queryProfile = (T_scoring *)malloc( numberOfColumns * alphabetSize * sizeof(T_scoring));
		generic_eh_type<T_scoring> *h_and_e_Columns = (generic_eh_type<T_scoring> *)calloc( numberOfColumns + 1, sizeof(generic_eh_type<T_scoring>) ); // memory for the score array
		
		/* Generate the query profile by using the scoring table
		 * Effect: See PPT-file
		 */
		queryProfileIterator = 0;
		for (scoreTableRowIterator = 0; scoreTableRowIterator < alphabetSize; ++scoreTableRowIterator) {
			T_scoring *referenceToStartOfRow = &scoringTable.pScoringTable[scoreTableRowIterator * alphabetSize];
			
			for (j = 0; j < numberOfColumns; ++j) {
				queryProfile[queryProfileIterator] = referenceToStartOfRow[ columnSequence[j] ];
				queryProfileIterator++;
			}
		}

		/* Initialize the h-elements of the scoreArray.
		 * The e-elements are set 0 by the initialization
		 */
	#if (CONF_SET_INITIAL_SCORE_VALUE == 1)
		h_and_e_Columns[0].h = initalScoreValue; 
		h_and_e_Columns[1].h = initalScoreValue > gapoe ? initalScoreValue - gapoe 
												   : 0;
	#else
		h_and_e_Columns[0].h = 0;
		h_and_e_Columns[1].h = 0;
	#endif

		for (j = 2; j <= numberOfColumns && h_and_e_Columns[j-1].h > pSWparameterSetRef.gape; ++j)
			h_and_e_Columns[j].h = h_and_e_Columns[j-1].h - pSWparameterSetRef.gape;

	#if (CONF_BAND_LIMITATION == 1)
		/* adjust $w if it is too large
		 * gape = 1; gapo = 3; gapoe = 4;
		 */
		sizeOfMatrix = alphabetSize * alphabetSize;
		maxScoreValue = 0;
		for (i = 0;  i < sizeOfMatrix; ++i) // get the max score
			maxScoreValue = maxScoreValue > scoringTable[i] ? maxScoreValue 
															 : scoringTable[i];
		
		upperLimitOfGapValue = (int)((double)(numberOfColumns * maxScoreValue - gapo) / gape + 1.);
		
		upperLimitOfGapValue = upperLimitOfGapValue > 1 ? upperLimitOfGapValue 
														: 1;
		
		w = w < upperLimitOfGapValue ? w 
									 : upperLimitOfGapValue;
	#endif

		/* computation of H matrix
		 */
	#if (CONF_SET_INITIAL_SCORE_VALUE == 1)
		maxScoreValue = initalScoreValue;
	#else
		maxScoreValue = 0;
	#endif
		
		/* rowIndexOfMaxScore and columnIndexOfMaxScore were originally initialized to -1 !
		 * they should be of type int instead of size_t
		 */
		rowIndexOfMaxScore = 0; 
		columnIndexOfMaxScore = 0;
		start = 0;
		end = numberOfColumns;

		for (i = 0; i < numberOfRows; ++i) {
			T_scoring f = 0;
			T_scoring hInNextRound;
			int indexOfMaxScoreInCurrentRow = -1;
			T_scoring maxScoreInCurrentRow = 0;
			
			/* We select the row in our query profile according to the row-sequence symbol that belongs to the current row
			 */
			T_scoring *referenceToQueryProfileRow = &queryProfile[ rowSequence[i] * numberOfColumns ];
			
			/* compute the first column
			 */
	#if (CONF_SET_INITIAL_SCORE_VALUE == 1)
			hInNextRound = initalScoreValue - (gapo + gape * (i + 1));
			if (hInNextRound < 0) 
				hInNextRound = 0;
	#else
			hInNextRound = 0;
	#endif

	#if (CONF_BAND_LIMITATION == 1)
			/* apply the band and the constraint (if provided)
			 * w seem to a 'band' that limits the computational space to +- w elements
			 */
			if (start < i - w) 
				start = i - w;
			
			if (end > i + w + 1) 
				end = i + w + 1;

			if (end > numberOfColumns) 
				end = numberOfColumns;
	#endif
			
			/* The outcome Matrix has size (numberOfRows + 1) x (numberOfColumns + 1)
			 */
			int uxMatrixStartShift = CONF_FILL_OUTCOME_MATRIX ? ( ( (i + 1) * (numberOfColumns + 1) ) + (start + 1) )
															  : 0;
			T_scoring *queryOutcomeMatrixIterator = CONF_FILL_OUTCOME_MATRIX ? scoringMatrix.scoringOutcomeMatrix + uxMatrixStartShift
																					: NULL;
			sw_direction_t *backtrackingMatrixIterator = CONF_FILL_OUTCOME_MATRIX ? scoringMatrix.backtrackMatrix + uxMatrixStartShift
																				  : NULL;
			
			for ( j = start; j < end; ++j ) {
				/* At the beginning of the loop: scoreArray[j] = { H(i - 1, j - 1), E(i, j) }, 
				 *								 f = F(i, j) 
				 *                               hInNextRound = new value for H(i, j - 1)
				 */
				generic_eh_type<T_scoring> *refIntoScoreArrayForColumn = &h_and_e_Columns[j];
				T_scoring h = refIntoScoreArrayForColumn->h; // get H(i-1,j-1)
				T_scoring e = refIntoScoreArrayForColumn->e; // get E(i,j)
				
				/* 1. add match or mismatch distance
				 * 2. h = H(i,j)= max{H(i-1, j-1) + Score(i, j), E(i, j), F(i, j)}
				 * Here we can imply the entry in the H matrix as well as the backtracking direction !
				 */
				sw_direction_t eDirection;
				if ( CONF_FILL_OUTCOME_MATRIX )
				{
					 eDirection = LEFT_UP;
				}
				
				h += referenceToQueryProfileRow[j]; // H(i-1, j-1) + Score(i, j)

				if (e > h)
				{
					h = e;
					if ( CONF_FILL_OUTCOME_MATRIX )
					{
						eDirection = UP;
					}
				}
				
				if (f > h)
				{
					h = f;
					if ( CONF_FILL_OUTCOME_MATRIX )
					{
						eDirection = LEFT;
					}
				}

				/* We store h in the Matrix for later analysis
				 */
				if ( CONF_FILL_OUTCOME_MATRIX )
				{
					*(queryOutcomeMatrixIterator++) = h;
					*(backtrackingMatrixIterator++) = eDirection;
				}
				
				refIntoScoreArrayForColumn->h = hInNextRound; // set H(i,j-1) for the next row
				hInNextRound = h; // save H(i,j) to hInNextRound for storage in the next iteration.
				
				indexOfMaxScoreInCurrentRow = maxScoreInCurrentRow > h ? indexOfMaxScoreInCurrentRow 
																	   : j;
				
				maxScoreInCurrentRow = maxScoreInCurrentRow > h ? maxScoreInCurrentRow 
																: h;   // m is stored at eh[mj+1]
				
				/* Compute new e value and store this value for the next round
				 * E(i + 1,j) = max{H(i, j) - gapo, E(i,j)} - gape
				 */
				h -= gapoe;
				h = h > 0 ? h 
						  : 0;
				e -= pSWparameterSetRef.gape;
				e = e > h ? e 
						  : h;   
				refIntoScoreArrayForColumn->e = e; // save E(i + 1, j) for the next row
				
				/* Compute new f value and store this for the next iteration
				 * F(i,j+1) = max{H(i, j) - gapo, F(i,j)} - gape
				 */
				f -= pSWparameterSetRef.gape;
				f = f > h ? f 
						  : h;  
			} // inner for
			
			/* Save the final hInNextRound, because there are no more iterations.
			 * Seems to be necessary for begin and end management
			 */
	#if (CONF_FOLLOW_MAX_PATH_OPTIMIZATION == 1)
			h_and_e_Columns[end].h = hInNextRound; 
			h_and_e_Columns[end].e = 0;
	#endif
	#if 0
			if (maxScoreInCurrentRow == 0) 
			{	/* There is no reason to continue, because all scores were zero
				 */
				break;
			}
	#endif

			if( maxScoreInCurrentRow > maxScoreValue )
			{
				maxScoreValue = maxScoreInCurrentRow;
				rowIndexOfMaxScore = i;
				columnIndexOfMaxScore = indexOfMaxScoreInCurrentRow;
			}

	#if (CONF_FOLLOW_MAX_PATH_OPTIMIZATION == 1)
			/* We count down from the maximum position until we find score 0 or we reach the first column
			 */
			for (j = indexOfMaxScoreInCurrentRow; j >= start && h_and_e_Columns[j].h; --j)
				;
			start = j + 1;

			/* We do something similar for the end
			 */
			for (j = indexOfMaxScoreInCurrentRow + 2; j <= end && h_and_e_Columns[j].h; ++j)
				;
			end = j;
	#endif
			
			//beg = 0; end = qlen; // uncomment this line for debugging
		} // outer for
		
		/* Free all allocated memory
		 */
		free(h_and_e_Columns); 
		free(queryProfile);
		
		/* We return the position (row and column index) of the maximum score value.
		 * The values are relative to the matrix. (+1 applied)
		 */
		if ( pColumnIndexOfMaxScore )
		{
			*pColumnIndexOfMaxScore = columnIndexOfMaxScore + 1;
		}
		if ( pRowIndexOfMaxScore )
		{
			*pRowIndexOfMaxScore = rowIndexOfMaxScore + 1;
		}
		
		if( pColumnIndexOfMaxScore && pRowIndexOfMaxScore )
		{
			std::cout << "Max score is: " << maxScoreValue << std::endl;
			std::cout << "Score at column " << *pColumnIndexOfMaxScore << " row " << *pRowIndexOfMaxScore << " is "
					   << scoringMatrix.scoringOutcomeMatrix[*pRowIndexOfMaxScore * scoringMatrix.numberOfColumns + *pColumnIndexOfMaxScore ]
					   << std::endl;
		}
		
		/* The index of the maximum within the table.
		 */
		indexOfMaxElementInScoringTable = (rowIndexOfMaxScore + 1) * scoringMatrix.numberOfColumns + (columnIndexOfMaxScore + 1);
		return maxScoreValue;
	}
}; // struct

