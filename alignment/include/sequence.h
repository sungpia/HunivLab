#pragma once

#include <iostream>
#include "exception.h"
#include "support.h"

/* 32bit rounding to the next exponent as define
 */
#ifndef kroundup32
#define kroundup32(x) (--(x), (x)|=(x)>>1, (x)|=(x)>>2, (x)|=(x)>>4, (x)|=(x)>>8, (x)|=(x)>>16, ++(x))
#endif

/* Generic reverse function, as it occurs in std::algorithms
 */
template <class T>
void reverse(T word[], size_t length)
{
	char temp;
	for ( size_t i = 0; i < length / 2; i++ )
	{
		temp = word[i];
		word[i] = word[length - i - 1 ];
		word[length - i - 1] = temp;
	} // for
} // reverse

/* Class for the management of strings that represent genetic sequences.
 * Special string class, for sequence handling.
 */
template <class ELEMENT_TYPE>
class PlainSequence 
{
private :
	/* We avoid the copy of PlainSequence Objects.
	 */
	PlainSequence(const PlainSequence&);

protected :
	/* The encapsulated sequence
	 */
	ELEMENT_TYPE *pxSequenceRef;

	/* Current size of the content of the encapsulated sequence
	 */
	size_t uxSize;

	/* Current size of the buffer.
	 */
	size_t uxCapacity;

	/* Resets all protected attributes to its initial values.
	 */
	void vReleaseMemory()
	{
		/* Allocated memory will be released!
		 */
		if ( pxSequenceRef != NULL ) 
		{
			free( pxSequenceRef );
		} // if
	} // protected method

	void vResetProtectedAttributes()
	{
		pxSequenceRef = NULL;
		uxSize = 0;
		uxCapacity = 0;
	} // protected method
	
	/* Tries to allocate the requested amount of memory and throws an exception if this process fails.
	 * uxRequestedSize is expressed in "number of requested elements"
	 */
	void vReserveMemory( size_t uxRequestedSize )
	{
		/* TO DO: This should be a bit more sophisticated ...
		 */
		kroundup32( uxRequestedSize );
		
		/* TO DO: If realloc fails try a malloc plus a copy operation.
		 * Or does realloc automatically finds a new location incl. copy?
		 */
		pxSequenceRef = (ELEMENT_TYPE*)realloc( pxSequenceRef, uxRequestedSize * sizeof(ELEMENT_TYPE) );

		if ( pxSequenceRef == NULL )
		{
			throw fasta_reader_exception("Memory Reallocation Failed");
		} // if

		uxCapacity = uxRequestedSize;
	} // method
	
public :
	PlainSequence() 
	{
		vResetProtectedAttributes();
	} // default constructor

	virtual ~PlainSequence()
	{
		/* This releases all allocated memory.
		 */
		vReleaseMemory();
	} // destructor

	/* This moves the ownership of the protected attributes to another object.
	 * The receiver of pxSequenceRef is responsible for its deletion.
	 */
	void vTransferOwnership( PlainSequence &rReceivingSequence )
	{
		/* We transport the three protected attributes to the receiver ...
		 */
		rReceivingSequence.pxSequenceRef = this->pxSequenceRef;
		rReceivingSequence.uxSize = this->uxSize;
		rReceivingSequence.uxCapacity = this->uxCapacity;

		/* ... and delete the knowledge here
		 */
		vResetProtectedAttributes();
	} // protected method

	/* Clears the inner sequence, but does not deallocate the memory.
	 */
	inline void vClear()
	{
		uxSize = 0;
	} // method

	/* Because we want the reference to the sequence private we offer a getter method.
	 * WARNING! Here you can get a null-pointer.
	 */
	inline const ELEMENT_TYPE* const uxGetSequence()
	{
		return this->pxSequenceRef;
	} // method

	/* Because we want to keep the size private we offer a getter method.
	 */
	inline const size_t uxGetSequenceSize() const
	{
		return this->uxSize;
	} // method
	
	/* Warning the inner string might not null-terminated after this operation
	 */
	inline void vAppend( const ELEMENT_TYPE* pSequence, size_t uxNumberOfElements )
	{
		size_t uxRequestedSize = uxNumberOfElements + this->uxSize;

		if ( uxCapacity < uxRequestedSize )
		{
			vReserveMemory ( uxRequestedSize );
		} // if

		/* WARNING: If we work later with non 8-bit data we have to be careful here
		 */
		memcpy( this->pxSequenceRef + uxSize, pSequence, uxNumberOfElements * sizeof(ELEMENT_TYPE) );

		uxSize = uxRequestedSize;
	} // method

	inline bool equal(const PlainSequence &rOtherSequence)
	{
		if ( this->uxSize == rOtherSequence.uxSize )
		{
			return memcmp(this->pxSequenceRef, rOtherSequence.pxSequenceRef, sizeof(ELEMENT_TYPE) * uxSize ) == 0;
		} // if
		
		return false;
	}
}; // class

class TextSequence : public PlainSequence<char>
{
public :
	TextSequence() : PlainSequence<char>()
	{
	} // default constructor

	TextSequence( const char* pcString ) : PlainSequence<char>()
	{
		vAppend(pcString );
	} // text constructor

	/* Terminates the inner string in the C-style using a null-character and 
	 * returns a reference to the location of the inner string.
	 */
	inline char* cString()
	{
		if ( uxCapacity < this->uxSize + 1 )
		{
			vReserveMemory ( this->uxSize + 1 );
		} // if

		this->pxSequenceRef[this->uxSize] = '\0';
		
		return pxSequenceRef;
	} // method

	inline void vAppend( const char &pcChar )
	{
		this->uxSize++;

		if ( uxCapacity < this->uxSize )
		{
			vReserveMemory ( this->uxSize );
		} // if

		this->pxSequenceRef[this->uxSize - 1] = pcChar;
	} // method

	/* Appends the content of pcString to the current buffer
	 */
	inline void vAppend( const char* pcString )
	{
		PlainSequence<char>::vAppend( pcString, strlen( pcString ) );
	} // method
}; // class

class GeneticSequenceSlice;

/* Special Class for Genetic Sequences
 * IDEA: BioSequence objects use numbers instead of characters for sequence representation
 * Supports:
 *  - translation from textual representation to representation as sequence of numbers.
 *  - generation of the reverse strand 
 */
class GeneticSequence : public PlainSequence<uint8_t>
{
public :
	typedef enum 
	{
		SEQUENCE_IS_NUCLEOTIDE,
		SEQUENCE_IS_PEPTIDE
	} SequenceType;

private :
	/* We avoid the copy of FastString Objects, because it could create trouble in the context of the realloc.
	 * In C++11: FastaString(const FastaString& ) = delete;
	 */
	GeneticSequence( const GeneticSequence& );

	/* The type of elements represented by our sequence.
	 */
	SequenceType eContentType; 

	/* The table used to translate from base pairs to numeric codes for nucleotides
	 */
	static const unsigned char xNucleotideTranslationTable[256];

	/* transforms the character representation into a representation on the foundation of digits.
	 */
	void vTranslateToNumericFormUsingTable( const unsigned char *alphabetTranslationTable,
											size_t uxStartIndex
										  )
	{
		for( size_t uxIterator = uxStartIndex; uxIterator < uxSize; uxIterator++ )
		{
			pxSequenceRef[uxIterator] = alphabetTranslationTable[ pxSequenceRef[uxIterator] ];
		} // for
	} // method

#if 0
	char translateACGTCodeToCharacter(uint8_t uxACGTCode)
	{
		const char chars[4] = {'A', 'C', 'G', 'T'};
		if (uxACGTCode < 4)
		{
			return chars[uxACGTCode];
		} // if
		else
		{
			return 'N';
		} // else
	} // method
#endif

public :
	GeneticSequence( SequenceType eContentType ) : eContentType( eContentType )
	{
	} // default constructor

	/* Any changes to this constructor have to be done with big care!
	 * This constructor is specially for the interface to file readers that deliver TextSequence objects.
	 * Be aware that the TextSequence will be stripped of its content.
	 */
	GeneticSequence( SequenceType eContentType, TextSequence &rSequence ) : eContentType( eContentType )
	{
		/* We strip the given sequence of its content and move it to our new sequence.
		 * WARNING: Here we assume that the sizes for the types char and uint8_t are equal.
		 */
		rSequence.vTransferOwnership( (PlainSequence<char>&)(*this) );

		/* The given PlainSequence should be in textual, we have to translate it.
		 */
		vTranslateToNumericFormUsingTable( xNucleotideTranslationTable, 0 );
	} // constructor
	
	GeneticSequenceSlice fullSequence();
	GeneticSequenceSlice fromTo( size_t from, size_t end );

#if 1
	/* Gives the textual representation for some numeric representation.
	 * TO DO: Remove this static function. At the moment it's only used in the context of backtracking.
	 */
	static char translateACGTCodeToCharacter(uint8_t uxACGTCode)
	{
		const char chars[4] = {'A', 'C', 'G', 'T'};
		if (uxACGTCode < 4)
		{
			return chars[uxACGTCode];
		} // if
		else
		{
			return 'N';
		} // else
	} // static method
#endif

	/* The symbol on some position in textual form.
	 * We count starting from 0.
	 */
	inline char charAt( size_t uxPosition )
	{
		if ( uxPosition >= uxSize)
		{
			throw fasta_reader_exception("Index out of range");
		} // if

		return translateACGTCodeToCharacter( pxSequenceRef[uxPosition] );
	} // method

	/* Appends text containing a genetic sequence to our sequence.
	 * Includes automatic translation.
	 */
	inline void vAppend( const char* pcString )
	{
		size_t uxSizeBeforeAppendOperation = this->uxSize;
		
		/* WARNING! char and uint8_t must have the same size or we get a serious problem here!
		 */
		PlainSequence<uint8_t>::vAppend( (const uint8_t*)pcString, strlen( pcString ) );

		vTranslateToNumericFormUsingTable( xNucleotideTranslationTable, uxSizeBeforeAppendOperation );
	} // method

#if 0
	/* Delivers a string object that contains a textual representation of our genetic sequence.
	 * Using this function can trigger inefficiency due to many repeated string copy operations.
	 */
	std::string getContentAsString( ) const
	{
		std::string sSequenceAsText;
		sSequenceAsText.resize(uxSize);

		for( size_t uxIterator = 0; uxIterator < uxSize; uxIterator++ )
		{
			sSequenceAsText[uxIterator] = translateACGTCodeToCharacter( pxSequenceRef[uxIterator] );
		} // for

		return sSequenceAsText;
	} // method
#endif
}; // class

/* In addition to a general genetic sequence a chromosome sequence can have a forward and reverse strand
 */
class ChromosomeSequence : public GeneticSequence
{
public :
	typedef enum 
	{
		UNKNOWN_STRAND		= 0,
		IS_FORWARD_STRAND	= 1,
		IS_REVERSE_STRAND	= 2
	} tStrandType;

private :
	tStrandType eStrandType;

	static const char *namesForStrandTypes[3];

	/* Creates the reverse strand if the sequence represents a forward strand.
	 * Creates the forward strand if the sequence represents a reverse strand.
	 * Works only for nucleotides.
	 */
	void vInverseStrand()
	{
		/* First we reverse the sequence itself
			*/
		reverse( pxSequenceRef, uxSize );

		/* And in a second step we invert the nucleotides itself
			*					   0  1  2  3
			*/
		const char chars[4] = {3, 2, 1, 0};

		for( size_t uxIterator = 0; uxIterator < uxSize; uxIterator++ )
		{
			if ( pxSequenceRef[uxIterator] < 4 )
			{
				pxSequenceRef[uxIterator] = chars[ pxSequenceRef[uxIterator] ];
			} // if
		} // for
	} // function

public :
	static const char *pcTextForStrandType( const tStrandType eStrand )
	{
		return namesForStrandTypes[ eStrand <= IS_REVERSE_STRAND ? eStrand : 0 ];
	}; // static function
		

	ChromosomeSequence( TextSequence &rSequence ) 
		: GeneticSequence( SEQUENCE_IS_NUCLEOTIDE, rSequence ),
		  eStrandType( IS_FORWARD_STRAND)	  
	{
	} // constructor

	/* Delivers the forward strand.
	 */
	ChromosomeSequence& rForwardStrand()
	{
		if ( eStrandType == IS_REVERSE_STRAND )
		{
			vInverseStrand();
		} // if 
		
		return *this;
	} // method

	/* Delivers the reverse strand
	 */
	ChromosomeSequence& rReverseStrand()
	{
		if ( eStrandType == IS_FORWARD_STRAND )
		{
			vInverseStrand();
		} // if 

		return *this;
	} // method
}; // class

/* Objects of this class represent slices of sequences that contain genetic data.
 * For such slices are for reading purposes merely. 
 */
class GeneticSequenceSlice
{
private :
	GeneticSequence &hostObject;

	/* The offset from the start of the Host section.
	 */
	const size_t uxOffsetInHostSection;

	/* The size of the slice
	 */
	const size_t uxSliceSize;

public :
	/* uxStartOffsetInHostSection is inclusive,
	 * uxEndOffsetInHostSection is exclusive, i.e. first element after the end of the sequence
	 */
	GeneticSequenceSlice( GeneticSequence &hostObject, size_t uxStartOffsetInHostSection, size_t uxEndOffsetInHostSection )
		 : hostObject( hostObject ), 
		   uxOffsetInHostSection( uxStartOffsetInHostSection ), 
		   uxSliceSize( uxEndOffsetInHostSection - uxStartOffsetInHostSection )
	{
	} // constructor

	/* Because we want the reference to the sequence private we offer a getter method.
	 * WARNING! Here you can get a null-pointer.
	 */
	inline const uint8_t* uxGetSequence() const 
	{
		return hostObject.uxGetSequence() + uxOffsetInHostSection;
	} // method

	/* Because we want to keep the size private we offer a getter method
	 */
	inline size_t uxGetSequenceSize() const
	{
		return this->uxSliceSize;
	} // method

	bool equals(const GeneticSequenceSlice &rOtherSequenceSlice)
	{
		if ( this->uxSliceSize == rOtherSequenceSlice.uxSliceSize )
		{
			return std::memcmp( (void *)(hostObject.uxGetSequence()), (void *)(rOtherSequenceSlice.hostObject.uxGetSequence()), (size_t)(sizeof(uint8_t) * this->uxSliceSize) ) == 0;
		} // if 

		return false;
	} // method

	/* Delivers a string object that contains a textual representation of the referred genetic sequence.
	 * Using this function can trigger inefficiency due to many repeated string copy operations.
	 */
	std::string asString( ) const
	{
		std::string sSequenceAsText;
		sSequenceAsText.resize( this->uxSliceSize );

		for( size_t uxIterator = 0; uxIterator < this->uxSliceSize; uxIterator++ )
		{
			sSequenceAsText[uxIterator] = GeneticSequence::translateACGTCodeToCharacter( uxGetSequence()[uxIterator] );
		} // for

		return sSequenceAsText;
	} // method
}; // class