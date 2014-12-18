/* The purpose of the memory manager shall support an repeated incremantal allocation of memory.
 */

#pragma once

#include <malloc.h>
#include <vector>
#include "support.h"
#include "exception.h"

/* The core memory manager class.
 */
class memory_manager
{
private:
	uint8_t *pStartOfAllocatedMemory;
	uint8_t *pCurrentOwner;
	size_t sizeOfAllocatedMemory;
	const size_t capacity;
	std::vector<uint8_t **> allReferingPointers;

	/* Resets the private attributes.
	 */
	void resetAttributes()
	{
		pCurrentOwner = NULL;
		sizeOfAllocatedMemory = 0;
		allReferingPointers.clear();
	}
	
	/* This method is called only once by the constructors for initializing the memory.
	 */
	void initializeMemory( )
	{
		pStartOfAllocatedMemory = (uint8_t *)malloc( (size_t)capacity );

		/* If the memory allocation failed, we throw some memory manager exception.
		 */
		if ( pStartOfAllocatedMemory == NULL )
		{
			throw memory_manager_exception( "Memory allocation failed" );
		}

		resetAttributes();
	} // private method

public:
	memory_manager(): capacity( (size_t)1000000 )
	{
		initializeMemory();
	} // default constructor

	memory_manager( size_t size ): capacity(size)
	{
		initializeMemory();
	} // constructor

	/* Resets the memory manager. All external pointers that refer into the managed memory are
	 * reset to NULL.
	 */
	void clear()
	{
		/* First we set all references into the managed memory area to NULL
		 */
		for ( auto iterator = allReferingPointers.begin(); iterator != allReferingPointers.end(); iterator++ ) 
		{
			*iterator = NULL;
		} // for

		resetAttributes();
	} // method

	/* Allocates memory and sets the current owner
	 */
	uint8_t* alloc( size_t uxRequestedSize )
	{
		if ( sizeOfAllocatedMemory + uxRequestedSize > capacity )
		{
			throw memory_manager_exception( "demanded memory exceeds capacity" );
		}

		pCurrentOwner = pStartOfAllocatedMemory + sizeOfAllocatedMemory;
		sizeOfAllocatedMemory = sizeOfAllocatedMemory + uxRequestedSize;

		allReferingPointers.push_back( &pCurrentOwner );
		
		return pCurrentOwner;
	} // method
	
	/* Realloc extends the amount of memory available for the current owner.
	 * The current owner is the pointer returned by the previous alloc.
	 * (The pointer given as argument and the current owner have to be equal.)
	 */
	void realloc( uint8_t* pRequester, size_t uxRequestedSize )
	{
		if ( pRequester != pCurrentOwner )
		{
			throw memory_manager_exception( "requester is not allowed to realloc memory" );
		}
		
		/* This is the altogether amount of memory necessary, if the allocation succeeds
		 */
		size_t sizeOfRequiredMemory = (pRequester - pStartOfAllocatedMemory) + uxRequestedSize; 
		if (sizeOfRequiredMemory > capacity )
		{
			throw memory_manager_exception( "realloc failed" );
		}

		sizeOfAllocatedMemory = sizeOfRequiredMemory;
	} // method

	~memory_manager()
	{
		/* We free all allocated memory.
		 */
		if ( pStartOfAllocatedMemory != NULL )
		{
			free( pStartOfAllocatedMemory );
		}
	} // destructor
};

/*
ftp://ftp.ensembl.org/pub/release-73/fasta/homo_sapiens/dna/
*/
