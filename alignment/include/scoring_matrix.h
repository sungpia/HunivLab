#pragma once

#include <iostream>
#include <vector>
#include <algorithm>
#include "support.h"
#include "fasta_reader.h"
#include "tinyxml2.h"

/* TO DO: Put the coring table into the parameter-set.
 */
template<class T_scoring>
class SmithWatermanParamaterSet 
{
private:
	/* We avoid the copy of SmithWatermanParamaterSet Objects, for efficiency reasons.
	 */
	SmithWatermanParamaterSet(const SmithWatermanParamaterSet&);

public :
	const T_scoring xWeightMatch;
	const T_scoring xWeightMismatch;
	const T_scoring gapo; 
	const T_scoring gape;

	SmithWatermanParamaterSet ( T_scoring xWeightMatch, T_scoring xWeightMismatch, T_scoring gapo, T_scoring gape )
		: xWeightMatch( xWeightMatch ), xWeightMismatch( xWeightMismatch ), gapo( gapo ), gape( gape )
	{ } // constructor

	T_scoring xGetApproximatedBackTrackDistance( size_t uxSequenceSize, T_scoring xScore )
	{
		T_scoring xMaximallyPossibleScore = uxSequenceSize * xWeightMatch;

		T_scoring xScoreDifference = xMaximallyPossibleScore - xScore;

		return uxSequenceSize + (xScoreDifference / gape);
	} // method
}; // class

typedef enum {
	STOP	= 0,
	LEFT_UP = 1,
	UP		= 2,
	LEFT	= 3
} sw_direction_t;

typedef enum {
	EQUAL_PAIR				 = 0,
	INSERTION_AT_ROW_SIDE	 = 1,
	INSERTION_AT_COLUMN_SIDE = 2,
	UNEQUAL_PAIR			 = 3
} e_alignment_element_kind_t;

typedef enum 
{
	ROW_SIDE	= 0,
	COLUMN_SIDE	= 1,
	BAR			= 2
} eSelectionType;

/* class for the description of alignments
 */
template<class T>
class alignment_description_element
{
private:
	char cMarkedElement( const char c ) const
	{
		return (eElementKind == UNEQUAL_PAIR) ? tolower(c) : c;
	} // private method

	/* We create the XML-structure for a single colored element
	 */
	tinyxml2::XMLNode* createHTMLForSingleCharacter( tinyxml2::XMLDocument &rXMLDocument, const char c ) const
	{
		/* Each single alignment element is placed in a <td> .... </td> complex
		 */
		tinyxml2::XMLNode* pFontElementRef = rXMLDocument.NewElement( "td");
		
		/* The text content for the single character.
		 */
		char cOutputCharacter[] = { eElementKind == UNEQUAL_PAIR ? tolower(c) : c, '\0'};
		
		/* We set the text content ...
		 * The function NewText creates a copy, so we can deliver a locally stored string.
		 */
		pFontElementRef->InsertEndChild( rXMLDocument.NewText( cOutputCharacter ) );

		/* ...and the color attribute
		 */
		pFontElementRef->ToElement()->SetAttribute("style", (eElementKind == UNEQUAL_PAIR) ? "color:red" : "color:green" );

		return pFontElementRef;
	} // private method

public :
	e_alignment_element_kind_t eElementKind;
	T entryOnRowSide;
	T entryOnColumnSide;

	alignment_description_element( e_alignment_element_kind_t eElementKind, 
								   T entryOnRowSide,
								   T entryOnColumnSide ) : eElementKind( eElementKind ), 
														   entryOnRowSide( entryOnRowSide ), 
														   entryOnColumnSide( entryOnColumnSide )
	{ } // constructor

	std::string sHtmlText( eSelectionType selectedSide ) const 
	{
		return std::string( 1,	  selectedSide == ROW_SIDE    ? cMarkedElement( entryOnRowSide )
								: selectedSide == COLUMN_SIDE ? cMarkedElement( entryOnColumnSide )
								: '|'	 				 
			              );
	} //method

	tinyxml2::XMLNode* createHTMLForElement( tinyxml2::XMLDocument &rXMLDocument, eSelectionType selectedSide ) 
	{
		return createHTMLForSingleCharacter
		( 
			rXMLDocument,	  
			  selectedSide == ROW_SIDE    ? cMarkedElement( entryOnRowSide )
			: selectedSide == COLUMN_SIDE ? cMarkedElement( entryOnColumnSide )
			: '|'	 				 
		);
	} //method
}; // class

template<class T>
class alignment_description : public std::vector< alignment_description_element<T>>
{
private:
	void appendSingleRow( iterator begin, 
						  iterator end, 
						  std::string &text, 
						  eSelectionType selectedSide )
	{
		std::for_each( begin, end, [&]( alignment_description_element<T> &element )
										{
											text.append( element.sHtmlText( selectedSide ) );
										} // lambda
					 ); // for_each
	} // method

	tinyxml2::XMLNode* createHTMLForSingleRow( iterator begin, 
											   iterator end, 
											   tinyxml2::XMLDocument &rXMLDocument, 
											   eSelectionType selectedSide )
	{
		/* All elements of one row are placed in a <tr> ..... </tr> complex
		 */
		tinyxml2::XMLNode* pNoBrElementRef = rXMLDocument.NewElement( "tr");
		
		std::for_each( begin, end, [&]( alignment_description_element<T> &element )
		{
			pNoBrElementRef->InsertEndChild( element.createHTMLForElement( rXMLDocument, selectedSide ) );
		} // lambda
		); // for_each

		return pNoBrElementRef;
	} // method

public:
	void getHTMLRepresentation( tinyxml2::XMLDocument &rXMLDocument, tinyxml2::XMLNode &rHTMLBodyElement )
	{
		tinyxml2::XMLNode* pTTElementRef( rXMLDocument.NewElement( "table") );
		rHTMLBodyElement.InsertEndChild( pTTElementRef );


		int stride = 100;
		iterator frontIterator = this->begin();

		while ( frontIterator < this->end() ) 
		{
			iterator tailIterator = ( this->end() - frontIterator ) >= stride ? frontIterator + stride
				: this->end();

			pTTElementRef->InsertEndChild( createHTMLForSingleRow( frontIterator, tailIterator, rXMLDocument, COLUMN_SIDE ) );
		
			pTTElementRef->InsertEndChild( createHTMLForSingleRow( frontIterator, tailIterator, rXMLDocument, BAR ) );
			
			pTTElementRef->InsertEndChild( createHTMLForSingleRow( frontIterator, tailIterator, rXMLDocument, ROW_SIDE ) );
			
			pTTElementRef->InsertEndChild( rXMLDocument.NewElement( "tr") );
			
			frontIterator = tailIterator;
		} // while
	} // method

	std::string & rsAppendStringRepesentation( std::string &string ) 
	{
		int stride = 100;
		iterator frontIterator = this->begin();

		while ( frontIterator < this->end() ) 
		{
			iterator tailIterator = ( this->end() - frontIterator ) >= stride ? frontIterator + stride
																			  : this->end();
			appendSingleRow( frontIterator, tailIterator, string, COLUMN_SIDE );
			string.append( "\n" );
			appendSingleRow( frontIterator, tailIterator, string, BAR );
			string.append( "\n" );
			appendSingleRow( frontIterator, tailIterator, string, ROW_SIDE );
			string.append( "\n" );

			frontIterator = tailIterator;
		} // while

		return string;
	} // method
	 
}; // class


template<class T_scoring, class T_size_t>
class ScoringLookupTable 
{
private:
	/* We avoid the copy of scoring_lookup_table Objects, because it could create trouble in the context of the realloc.
	 * In C++11: FastaString(const FastaString& ) = delete;
	 */
	ScoringLookupTable(const ScoringLookupTable&);

	/* the scoring table must have size 5 X 5 of type T_scoring
	 */
	void initialzeScoringTable( )
	{
		/* initialize scoring matrix
		 */
		T_size_t matrixIterator = 0;
		for (uint8_t i = 0; i < 4; ++i) 
		{
			for (uint8_t j = 0; j < 4; ++j)
				pScoringTable[matrixIterator++] = i == j ? this->xWeightMatch
														 : this->xWeightMismatch;
		
			pScoringTable[matrixIterator++] = 0; // ambiguous base
		}
		/* Fill the final row of the scoring matrix with zeros.
		 */
		for (int j = 0; j < 5; ++j)
		{
			pScoringTable[matrixIterator++] = 0;
		}
	} // method

public:
	const T_scoring xWeightMatch;
	const T_scoring xWeightMismatch;

	/* Quick lookup table for scoring. \
	 * At the moment we allow direct access because of performance.
	 * Here we should integrate a friend declaration.
	 */
	T_scoring* pScoringTable;

	ScoringLookupTable( T_scoring xWeightMatch, T_scoring xWeightMismatch ) : xWeightMatch( xWeightMatch ), xWeightMismatch( xWeightMismatch )
	{
		/* We reserve memory for the scoring table and initialize the table.
		 */
		pScoringTable = new T_scoring[5 * 5];
		initialzeScoringTable( );
	} // constructor

	/* The destructor frees the allocated memory automatically 
	 */
	~ScoringLookupTable()
	{
		delete[] pScoringTable;
	} // destructor

	T_scoring xCalculateMaxScore( T_size_t xSequenceLength )
	{
		return xSequenceLength * xWeightMatch;
	} // method
}; // class

template<class T_scoring>
struct sw_alignment_result_t
{
	/* The absolute index.
	 * It has to interpreted as if the matrix were a plain data structure
	 */
	size_t uxIndex;
	T_scoring score;
};

/* The compare function used in the below qsort
 */
template<class T_scoring>
int compare_sw_alignment_result_t(const void* p1, const void* p2) 
{
	const sw_alignment_result_t<T_scoring> f1 = *(const sw_alignment_result_t<T_scoring>*) p1;
	const sw_alignment_result_t<T_scoring> f2 = *(const sw_alignment_result_t<T_scoring>*) p2;
	return f2.score - f1.score;
} // compare_sw_alignment_result_t

#if 0
/* Generic implementation of the reverse function.
 */
template <class T>
void reverse(T word[])
{
	size_t len = strlen(word);
	char temp;
	for ( size_t i = 0; i < len / 2; i++ )
	{
		temp = word[i];
		word[i] = word[len - i - 1 ];
		word[len - i - 1] = temp;
	}
} // reverse
#endif
/* Class for the description of scoring matrices.
 * Scoring matrices can be used by the serial aligner for backtracking purposes.
 */
template<class T_scoring, class T_size_t>
struct scoring_matrix
{
	T_scoring* scoringOutcomeMatrix;
	sw_direction_t*	backtrackMatrix;

#if 0
	SequenceString &rRowSequenceString;
	SequenceString &rColumnSequenceString;
#endif
	
	const uint8_t* puxRowSequenceRef;
	const T_size_t numberOfRows;
	const uint8_t* puxColumnSequenceRef;
	const T_size_t numberOfColumns;


	/* Initializes the first column and the first row.
	 */
	void initializeFristColumnAndFirstRow() 
	{
		
		for(T_size_t uxIterator = 0; uxIterator < numberOfColumns; uxIterator++ )
		{
			scoringOutcomeMatrix[ uxIterator ] = 0;
			backtrackMatrix[ uxIterator ] = STOP;
		}

		for(T_size_t uxIterator = 0; uxIterator < numberOfRows; uxIterator++ )
		{
			scoringOutcomeMatrix[ uxIterator * numberOfColumns ] = 0;
			backtrackMatrix[ uxIterator * numberOfColumns ] = STOP;
		}
	}

	/* Default constructor
	 * The row and the column sequence should be already translated!
	 */
	scoring_matrix( T_size_t numberOfColumns, const uint8_t*columnSequence, 
		            T_size_t numberOfRows, const uint8_t*rowSequence ) 
				   : numberOfColumns( numberOfColumns + 1 ), // + 1 because the matrix gets a virtual first column 
				     puxColumnSequenceRef( columnSequence ),
				     numberOfRows( numberOfRows + 1 ), // + 1 because the matrix gets a virtual first row
				     puxRowSequenceRef( rowSequence )
	{
		const T_size_t sizesOfOutcomeAndBacktrackingMatrix = (this->numberOfColumns) * (this->numberOfRows);
		
		/* Reserve memory for the matrices with scoring and backtracking information
		 */
		scoringOutcomeMatrix = new T_scoring[sizesOfOutcomeAndBacktrackingMatrix];
		backtrackMatrix = new sw_direction_t[sizesOfOutcomeAndBacktrackingMatrix];
		
		initializeFristColumnAndFirstRow();
	} // default constructor
	
	/* Destructor.
	 */
	~scoring_matrix()
	{
		delete[] scoringOutcomeMatrix;
		delete[] backtrackMatrix;
	}

	/* Dumps the matrix to cout for debugging purposes
	 */
	void dump()
	{
		char directionSymbols[4] = {'x', '\\', '^', '<'};

		std::cout << "**********************************************" << std::endl;
		std::cout << "The scoring matrix is given by  " << std::endl << std::endl;

		std::cout << "    - \t";

		for( size_t uxIteratorColum = 1; uxIteratorColum < numberOfColumns; uxIteratorColum++ )
		{
			std::cout << translateACGTCodeToCharacter(puxColumnSequenceRef[ uxIteratorColum - 1 ]) << "\t";
		}

		std::cout << std::endl;

		for( size_t uxIteratorRow = 0; uxIteratorRow < numberOfRows; uxIteratorRow++ )
		{
			if (uxIteratorRow > 0)
				std::cout << translateACGTCodeToCharacter(puxRowSequenceRef [uxIteratorRow - 1]) << " : ";
			else
				std::cout << "- : ";

			for( size_t uxIteratorColum = 0; uxIteratorColum < numberOfColumns; uxIteratorColum++ )
			{
				std::cout << directionSymbols[ backtrackMatrix [uxIteratorRow * numberOfColumns + uxIteratorColum ] ] 
					 << scoringOutcomeMatrix[ (uxIteratorRow * numberOfColumns + uxIteratorColum ) ]
					 << "\t";
			} // for uxIteratorColum
			std::cout << std::endl;
		} // for uxIteratorRow
	}

	/* Performs a backtrack within the scoring matrix.
	 * The caller has to deliver enough space in resultingSequenceForRow as well as resultingSequenceForColumn.
	 */
	void backtrackFromIndexText( size_t startIndex,
						     char *resultingSequenceForRow,
						     char *resultingSequenceForColumn,
						     size_t &startColumn, 
						     size_t &endColumn,
						     size_t &startRow, 
						     size_t &endRow
							)
	{
		size_t index = startIndex;
		size_t endIndex = startIndex;

		/* We store the initial references for later reversing
		 */
		char *rowSequenceIterator = resultingSequenceForRow;
		char *columnSequenceIterator = resultingSequenceForColumn;

		while( backtrackMatrix[index] != STOP && (scoringOutcomeMatrix[index] > 0) )
		{
			/* size_t should be an integer type !
			 */
			size_t uxColumn = index % numberOfColumns;
			size_t uxRow = index / numberOfColumns;

			

			/* We check whether the two symbols match
			 * WARNING: The scoring matrix is currently shifted into X as well Y direction by one position
			 */
			if ( puxRowSequenceRef[uxRow - 1] == puxColumnSequenceRef[uxColumn - 1] )
			{
				*(rowSequenceIterator++) = translateACGTCodeToCharacter(puxRowSequenceRef[uxRow - 1]);
				*(columnSequenceIterator++) = translateACGTCodeToCharacter(puxRowSequenceRef[uxRow - 1]);
			}
			else
			{	
				switch ( backtrackMatrix[index] )
				{
				case LEFT_UP :
					*(rowSequenceIterator++) = 'x';
					break;
				case LEFT :
					*(rowSequenceIterator++) = '+';
					break;
				case UP :
					*(rowSequenceIterator++) = translateACGTCodeToCharacter(puxRowSequenceRef[uxRow - 1]);
				} // switch

				switch ( backtrackMatrix[index] )
				{
				case LEFT_UP :
					*(columnSequenceIterator++) = 'x';
					break;
				case LEFT :
					*(columnSequenceIterator++) = translateACGTCodeToCharacter(puxColumnSequenceRef[uxColumn - 1]);
					break;
				case UP :
					*(columnSequenceIterator++) = '+';
				} // switch
			}

			/* Now we do the real backtracking.
			 */
			endIndex= index;
			switch ( backtrackMatrix[index] )
			{
			case LEFT_UP :
				index = ((uxRow - 1) * numberOfColumns) + (uxColumn - 1);
				break;
			case LEFT :
				index--;
				break;
			case UP :
				index = ((uxRow - 1) * numberOfColumns) + uxColumn;
			}
		}
	}

	/* Performs a backtrack within the scoring matrix. The outcome is here an STL vector. 
	 * The caller has to deliver an empty vector and is responsible for the memory allocation and deallocation.
	 */
	void backtrackFromIndex( T_size_t startIndex,
							 alignment_description<char> &alignmentOutcomeVector,
							 T_size_t &startPositionInColumn, 
						     T_size_t &endPositionInColumn,
						     T_size_t &startPositionInRow, 
						     T_size_t &endPositionInRow
						   )
	{
		T_size_t currentIndex = startIndex;
		T_size_t previousIndex = startIndex;

		while( backtrackMatrix[currentIndex] != STOP && (scoringOutcomeMatrix[currentIndex] > 0) )
		{
			/* We calculate the current column and row on the foundation of the current index
			 */
			T_size_t uxColumn = currentIndex % numberOfColumns;
			T_size_t uxRow = currentIndex / numberOfColumns;

			/* We check whether the two symbols match
			 * WARNING: The scoring matrix is shifted into X as well Y direction by one position
			 */
			if ( puxRowSequenceRef[uxRow - 1] == puxColumnSequenceRef[uxColumn - 1] )
			{
				/* They match, so we insert a match into the vector. (push_back using &&)
				 */
 				alignmentOutcomeVector.push_back( alignment_description_element<char> ( EQUAL_PAIR, 
																						GeneticSequence::translateACGTCodeToCharacter( puxRowSequenceRef[uxRow - 1] ), 
																						GeneticSequence::translateACGTCodeToCharacter( puxColumnSequenceRef[uxColumn - 1] ) ) );
			} // if
			else
			{	
				switch ( backtrackMatrix[currentIndex] )
				{
				case LEFT_UP :
					alignmentOutcomeVector.push_back( alignment_description_element<char> ( UNEQUAL_PAIR, 
																							GeneticSequence::translateACGTCodeToCharacter( puxRowSequenceRef[uxRow - 1] ), 
																							GeneticSequence::translateACGTCodeToCharacter( puxColumnSequenceRef[uxColumn - 1] ) ) );
					break;
				case LEFT :
					alignmentOutcomeVector.push_back( alignment_description_element<char> ( INSERTION_AT_ROW_SIDE, 
																							'+', 
																							GeneticSequence::translateACGTCodeToCharacter( puxColumnSequenceRef[uxColumn - 1] ) ) );
					break;
				case UP :
					alignmentOutcomeVector.push_back( alignment_description_element<char> ( INSERTION_AT_COLUMN_SIDE, 
																							GeneticSequence::translateACGTCodeToCharacter( puxRowSequenceRef[uxRow - 1] ), 
																							'+' ) );
				case STOP :
					;
				} // switch
			} // else

			/* Now we do the real backtracking.
			 */
			previousIndex = currentIndex;
			switch ( backtrackMatrix[currentIndex] )
			{
			case LEFT_UP :
				currentIndex = ((uxRow - 1) * numberOfColumns) + (uxColumn - 1);
				break;
			case LEFT :
				currentIndex--;
				break;
			case UP :
				currentIndex = ((uxRow - 1) * numberOfColumns) + uxColumn;
			case STOP :
				;
			}
		}
	
		/* Finally we inform about the begin and end of our sequences
		 * Here we take 1 way, so we get values according to a counting starting with 0 instead of 1
		 */
		startPositionInColumn = (previousIndex % numberOfColumns) - 1;
		endPositionInColumn = (startIndex % numberOfColumns) - 1;

		startPositionInRow = (previousIndex / numberOfColumns) - 1;
		endPositionInRow = (startIndex / numberOfColumns) - 1;

		/* !!!WARNING!!! alignmentOutcomeVector was created in REVERSED order.
		 * So, we reverse the alignmentOutcomeVector for getting the correct output
		 */
		std::reverse( alignmentOutcomeVector.begin(), alignmentOutcomeVector.end() );
	}

	void dumpAlignmentFromRowColumn( T_size_t row, T_size_t column )
	{
		std::cout << "row is here " << row << " and column is here " << column << std::endl;
		T_size_t index = (row * numberOfColumns) + column;
		std::cout << "index is here " << index;
		dumpAlignmentFromIndex( index, scoringOutcomeMatrix[index] );
	}

	void dumpAlignmentFromIndex( T_size_t index, T_scoring score )
	{
		int bufferSize = numberOfColumns + numberOfRows + 1;

		char *textBufferforRow = (char *)malloc( bufferSize * sizeof(char) );
		char *textBufferforColumn = (char *)malloc( bufferSize * sizeof(char) );

		size_t startColumn, endColumn;
		size_t startRow, endRow;

		std::cout << "index " << index << " has score " << score << " :: " << std::endl;
		backtrackFromIndexText( index, 
								textBufferforRow, 
								textBufferforColumn, 
								startColumn, endColumn, startRow, endRow);
		
		std::cout << textBufferforColumn << "  [" << startColumn << ".." << endColumn << "]" << std::endl;	
		std::cout << textBufferforRow << "  [" << startRow << ".." << endRow << "]" << std::endl;	
		
		std::cout  << std::endl;

		free(textBufferforRow);
		free(textBufferforColumn);
	}
	
	/* Dumps all alignments for the current matrix
	 */
	void dumpAllAlignments() 
	{
		T_size_t matrixSize = numberOfRows * numberOfColumns;
		sw_alignment_result_t<T_scoring>* swAlignmentOutcomes = new sw_alignment_result_t<T_scoring>[matrixSize];	
	
		for(T_size_t uxIterator = 0; uxIterator < matrixSize; uxIterator++ )
		{
			swAlignmentOutcomes[uxIterator].score = scoringOutcomeMatrix[ uxIterator ];
			swAlignmentOutcomes[uxIterator].uxIndex = uxIterator;
		}

		/* We take the C like quick sort because it is faster than the stl implementation
		 */
		qsort( swAlignmentOutcomes, matrixSize, sizeof(sw_alignment_result_t<T_scoring>), compare_sw_alignment_result_t<T_scoring> );

		for(T_size_t uxIterator = 0; uxIterator < matrixSize; uxIterator++ )
		{
			if ( swAlignmentOutcomes[uxIterator].score > 0 )
			{
				dumpAlignmentFromIndex( swAlignmentOutcomes[uxIterator].uxIndex, swAlignmentOutcomes[uxIterator].score );
			}
		} // for

		delete[] swAlignmentOutcomes;
	}
};