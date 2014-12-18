#pragma once

#include <iostream>
#include "exception.h"
#include "support.h"
#include "sequence.h"

/* Structure that describes the data of some Fasta Record
 */
struct FastaDescriptor
{
	std::string sName;
	std::string sComment;

	/* Here we take a text sequence because Fasta files contain text.
	 */
	TextSequence sequence; 
	
	TextSequence qualifier;   
}; // struct

template<int BUFFER_SIZE>
class BufferedFileStreamer
{
private :
	/* We prohibit the copy of Objects of this class
	 */
	BufferedFileStreamer(const BufferedFileStreamer&);

	/* The buffer is placed on the heap in order to safe stack space.
	 */
	char* pcBufferRef;

	/* References to the first and last element in the buffer
	 */
	size_t uxBegin, uxEnd;

	/* We take c style file access
	 */
	FILE *pFileHandle;

	bool bIsEOF;   

	inline void vRefillBuffer()
	{
		// std::cout << "refill buffer" << std::endl;

		uxBegin = 0;   
		uxEnd = fread(pcBufferRef, sizeof(char), BUFFER_SIZE, pFileHandle); 

		if (uxEnd < BUFFER_SIZE)
		{
			bIsEOF = 1; 
		} // if
	} // inline method

public :
	BufferedFileStreamer( char* pcFileNameRef ) : uxBegin( 0 ), uxEnd( 0 ), bIsEOF( false ), pcBufferRef( NULL )
	{
#ifdef _MSC_VER
		if ( fopen_s( &pFileHandle, pcFileNameRef, "r" ) )
#else
		if ( ( pFileHandle = fopen( pcFileNameRef, "r" ) ) == NULL )
#endif
		{
			/* If something goes wrong we signalize this using an exception.
			 */
			throw fasta_reader_exception( "File open error" );
		} // if

		/* We allocate memory for the read-buffer. 
		 */
		pcBufferRef = new char[BUFFER_SIZE] ;
	} // constructor

	virtual ~BufferedFileStreamer()
	{
		/* We make sure, that we close the file in all circumstances.
		 */
		if ( pFileHandle != NULL )
		{
			fclose( pFileHandle );
		} // if

		/* We release all allocated resources. 
		 */
		delete[] pcBufferRef;
	} // destructor

	/* keep me here, so that I stay automatically inlined ...
	 */
	inline bool getSingleChar( char &cChar )                               
	{                                                                                                            
		if (uxBegin >= uxEnd) 
		{                                                   
			if ( bIsEOF == false )
			{
				vRefillBuffer();

				if (uxEnd == 0 )
				{
					return false;
				} // if level 3
			} // if level 2
			else
			{
				return false;
			} // else
		} // if level 1

		cChar = pcBufferRef[uxBegin++];

		return true;                                      
	} // inline method

	/* Reads from the stream until it sees the given delimiter
	 */
	inline bool bGetUntilDelimiter( const char cDelimiter, PlainSequence<char> &rSequence, char &cLastCharacterRead ) 
	{                                                                                                                                       
		cLastCharacterRead = '\0';  

		while ( true ) 
		{                                                                                                              
			size_t uxIterator;   

			if (uxBegin >= uxEnd) 
			{                                                                     
				/* We consumed the complete buffer content
				 */
				if ( bIsEOF == false )
				{                                                                              
					vRefillBuffer();
					
					if ( uxEnd == 0 ) 
					{
						/* We reached the end of file and couldn't read anything.
						 */
						return false; 
					}
				}  // if
				else
				{
					/* The buffer is empty and we have an EOF, so we exit the while loop and return
					 */
					return false;  
				} // else
			} // if                                                                                                                

			if ( cDelimiter != '\0' ) 
			{                                                                                        
				/* We read until we come to the end of the buffer or we see some delimiter
				 */
				for ( uxIterator = uxBegin; uxIterator < uxEnd; ++uxIterator ) 
				{
					if ( pcBufferRef[uxIterator] == cDelimiter ) 
					{
						break;
					} // if
				} // for			
			} // if
			else 
			{                                                                                                       
				/* If the first delimiter is the null-character, we consume all spaces.
				 */
				for ( uxIterator = uxBegin; uxIterator < uxEnd; ++uxIterator )
				{
					if ( isspace(pcBufferRef[uxIterator]) ) 
						break;
				} // for
			}   

			rSequence.vAppend( pcBufferRef + uxBegin, uxIterator - uxBegin );

			uxBegin = uxIterator + 1; 

			if (uxIterator < uxEnd) {
				/* We found the requested delimiters or spaces.
				 * We store the last character that we got in cLastCharacterRead
				 */
				cLastCharacterRead = pcBufferRef[uxIterator];
				
				break;                                                                                                
			} // outer if                                                                                                                
		} // while                                                                                                                             

		return true;                                                                                                 
	} // method
}; // class

/* A FastaStreamReader can read a fasta-file and creates the appropriate fasta record.
 */
template<int BUFFER_SIZE>
class FastaStreamReader : BufferedFileStreamer<BUFFER_SIZE>
{ 
private:
	/* Return values:
	   >=0  length of the sequence (more sequences following)
	    0 : read a sequence no more sequences following
		1 : There are more sequences following
	   -1   reading failed
	   -2   truncated quality string
	 */                                                                                                           
	int readSequence( FastaDescriptor &fastaRecord, bool bIsFirstSequence )                                                                      
	{                                                                                                                                       
		char cChar;     

		if ( bIsFirstSequence ) 
		{ 
			/* then jump to the next header line 
			 */ 
			while ( true )
			{
				if ( getSingleChar( cChar ) == false )
				{
					return -1;
				} // if
				
				if ( cChar == '>' || cChar == '@' )
				{
					/* We did see the beginning of a fasta record. 
					 */
					break;
					//goto CONTINUE; // break;
				} // if
			} // while                                                                      
		} // if    
		//CONTINUE :

		/* We read the name. (String until the first space / new line)
		 */
		TextSequence bufferSequence;
		if ( bGetUntilDelimiter( 0, bufferSequence, cChar ) == false )
		{
			return -1;
		} // if

		fastaRecord.sName = bufferSequence.cString();
		bufferSequence.vClear();

		/* We read the remaining part of the first line as comment
		 */
		if ( cChar != '\n' )
		{
			if ( bGetUntilDelimiter( '\n', bufferSequence, cChar ) == false )
			{
				return -1;
			} // inner if
		} // if

		fastaRecord.sComment = bufferSequence.cString();
		bufferSequence.vClear();
	
		/* We read the core sequence
		 */
		while (	true ) 
		{ 
			if ( getSingleChar( cChar ) == false )
			{
				/* EOF there isn't anything more ...
				 */
				return 0;
			} // if

			if ( (cChar == '>') || (cChar == '@') )
			{
				/* We found the beginning of a next sequence
				 */
				return 1;
			} // if

			if ( cChar == '+' )
			{
				/* We found a qualifier ...
				 */
				break;
			}
			
			if ( isgraph( cChar ) ) 
			{ 
				/* We found a printable non-space character and 
				 * append the single character to the sequence
				 */
				fastaRecord.sequence.vAppend( cChar );
			} // if                                                                                                                 
		} // while
	    
		/* skip the rest of the '+' line 
		 */
		bufferSequence.vClear();
		bGetUntilDelimiter( '\n', bufferSequence, cChar );

		/* TO OD; Fore a reserve for qualifier using the size of the sequence
		 */
		while (	true ) 
		{ 
			if ( getSingleChar( cChar ) == false )
			{
				/* EOF there isn't anything more ...
				 */
				break;
			} // if

			if ( cChar >= 33 && cChar <= 127 ) 
			{
				fastaRecord.qualifier.vAppend( cChar );
			} // if
		} // while

		if ( fastaRecord.sequence.uxGetSequenceSize() != fastaRecord.qualifier.uxGetSequenceSize() )
		{
			return -2;
		}  // if  

		return 0;
	} //method

public :
	/* Sequence provider gets access to the private stuff! */
	friend class FastaReader;

	FastaStreamReader( char* pcFileNameRef, FastaDescriptor &fastaRecord ) : BufferedFileStreamer( pcFileNameRef )
	{
		/* We read a single record.
		 */
		readSequence( fastaRecord, true );
	} // constructor

	virtual ~FastaStreamReader()
	{
		/* Automatic call of the superclass destructor
		 */
	} // destructor	
}; // class

class FastaReader : public FastaDescriptor
{                                                                                    
public :
	/* The default constructor.
	 */
	FastaReader()
	{ } 

	/* The central load function.
	 */
	void vLoadFastaFile( const char *pFileName )
	{
		/* If the Fasta reader experiences some problem, it will throw an exception.
		 */
		FastaStreamReader<8192>fastaReader( (char *)pFileName, *this );
	} // method Anonymous

	/* The destruction will free the memory of the Fasta-Record
	 */
	~FastaReader()
	{ } // destructor
}; // class
