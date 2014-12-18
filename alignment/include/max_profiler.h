#pragma once
#include <algorithm>
#include "support.h"

#define CONF_USE_MALLOC	( 1 )
#define DEF_ITERATIONS_PER_LAYER ( 2 )

#if 0
#define DEBUG_MSG(str) do { std::cout << str << std::endl; } while( false )
#else
#define DEBUG_MSG(str) do { } while ( false )
#endif


/* bUseVectorClass is a boolean switch that determines, whether we take the STL vector class or plain C style malloc, realloc and free.
 */
template<class T_scoring, class T_size_t, class T_profile_elements>
class max_profiler_core
{
private:
	/* We avoid the copy of Profiler Objects, because it could create trouble in the context of the realloc.
	 */
	max_profiler_core(const max_profiler_core&);

	bool			bMaxiumAhead;
	T_scoring		localMinScore;
	const T_size_t	capacityOfProfile;
	T_profile_elements *profile;
	
	max_profiler_core<T_scoring, T_size_t, T_profile_elements> *pNextLevel;
	
	/* Stores the value of largest maximum ever seen, during profile filling
	 */
	T_scoring xLargestMaximumEverSeen;

	static inline T_profile_elements uxPack( T_scoring xScore, T_size_t xRow )
	{
		return (T_profile_elements)xScore << 32 | xRow;
	} // inline method

	/* Extracts the score part of some profiler entry
	 */
	static inline T_scoring uxExtractScore( T_profile_elements uxELement )
	{
		return (T_scoring)(uxELement >> 32); 
	} // inline method

	/* Extracts the row part of some profiler entry
	 */
	static inline T_size_t uxExtractRow( T_profile_elements uxELement )
	{
		return (T_size_t)(uxELement & 0xFFFFFFFF);
	} // inline method

	/* Delivers a compare function, required for sorting.
	 */
	static bool vCompareProfileElements( T_profile_elements i, T_profile_elements j ) 
	{ 
		return ( uxExtractScore(i) > uxExtractScore(j) ); 
	} // method

#if 0
	inline void vUnpack( T_profile_elements uxELement, T_scoring &xScore, T_size_t &xRow )
	{
		xRow = uxExtractRow( uxELement ); 
		xScore = uxExtractScore( uxELement );
	} // inline method
#endif

	/* Flushes the current buffer one level deeper.
	 * The profiler has a multilayer architecture.
	 */
	void flush()
	{
		DEBUG_MSG( "flush: length of profile: " << sizeOfProfile );

		/* We apply automatic shrinking by filtering in order to avoid memory problems
		 */
		if ( sizeOfProfile > 0 )
		{
			/* If there is no next level, we have to create one. We forward the largestMaximumEverSeen.
			 * We have to double the size in the next level due to the existence of many maxima.
			 */
			if ( pNextLevel == NULL )
			{
				pNextLevel = new max_profiler_core<T_scoring, T_size_t, T_profile_elements>( xLargestMaximumEverSeen, 2 * this->capacityOfProfile );
			} // if
			
			/* Data shrinking. (The final flush should use 0 here)
			 */
			inPlaceFiltering( DEF_ITERATIONS_PER_LAYER );
			
			/* ...and add the filtered data one layer down.
			 * HINT: We have an additional filtering with the push operation
			 */
			for( T_size_t vectorIterator = 0; vectorIterator < this->sizeOfProfile; vectorIterator++ )
			{
				DEBUG_MSG( "push to next level. score:" << uxExtractScore( profile[vectorIterator] ) << " of row:" << uxExtractRow( profile[vectorIterator] ) );
				pNextLevel->push( uxExtractScore( profile[vectorIterator] ), uxExtractRow( profile[vectorIterator] ) );
			} // for

			vResetProfile();
		} // if
	} // private method
	
	/* Iterative filter.
	 * We apply the extraction of local maxima so long, until we have only requestedLengthOfProfile many elements
	 */
	void inPlaceFiltering( T_size_t numberOfIterations ) 
	{
		DEBUG_MSG( "call inPlaceFiltering ..." );

		for ( T_size_t iterator = 0; iterator < numberOfIterations; iterator++ ) 
		{
			T_size_t lengthOfProfileBackup = sizeOfProfile;
			vResetProfile();
			
			DEBUG_MSG( "FROM size: " <<  lengthOfProfileBackup );
			/* There is no danger of overwriting existing elements. The situation is like for merging into the longer vector.
			 */
			for( T_size_t vectorIterator = 0; vectorIterator < lengthOfProfileBackup; vectorIterator++ )
			{
				push( uxExtractScore( profile[vectorIterator] ), uxExtractRow( profile[vectorIterator] ) );
			} // for
			DEBUG_MSG( "TO size: " <<  sizeOfProfile  << std::endl );

			/* Security check
			 */
			if ( sizeOfProfile > lengthOfProfileBackup )
			{
				std::cout << "Something is wrong with the filter" << sizeOfProfile << std::endl;
				exit( 1 );
			}

			/* If the size of the profile doesn't change any more, we have no change to get a profile of size requestedLengthOfProfile and exit
			 */
			if ( sizeOfProfile == lengthOfProfileBackup )
			{
				break;
			} //if
		} // for
	} // private method

	void vResetProfile()
	{
		 bMaxiumAhead = false; // Don't change this or you run into trouble!
		 sizeOfProfile = 0;
		 localMinScore = 0;
	} // private method	

public:
	T_size_t		sizeOfProfile;

	/* Constructor
	 * TO DO: check whether 64 bit are enough.
	 */
	max_profiler_core( T_scoring xLargestMaximumEverSeen, T_size_t  capacityOfProfile ) 
		: xLargestMaximumEverSeen( xLargestMaximumEverSeen ),
		  capacityOfProfile( capacityOfProfile ),
		  pNextLevel( NULL )
	{
		vResetProfile();

#if (CONF_USE_MALLOC == 1)
		/* malloc is much faster than using a std::vector.
		 */
		profile = (T_profile_elements *)malloc( sizeof(T_profile_elements) * capacityOfProfile );
		if ( profile == NULL )
		{
			std::cout << "malloc with the profiler failed:" << capacityOfProfile << std::endl;
		} // if
#else
		profile.reserve( capacityOfProfile );
#endif
	} // constructor

	~max_profiler_core()
	{
	#if (CONF_USE_MALLOC == 1)
		if ( profile != NULL )
		{
			free(profile);
		} // if
	#endif
		/* Delete the recursive filtered profile if necessary.
		 */
		if ( pNextLevel != NULL )
		{
			delete pNextLevel;
		} // if
	} // destructor

	/* This method has to be called only once at the end of the alignment process.
	 * The deepest layer itself doesn't require any flush!
	 */
	void deep_flush( )
	{
		if ( pNextLevel != NULL )
		{
			/* We flush all elements, so that this level becomes empty
			 */
			flush();
			pNextLevel->deep_flush( );
		}
	} // method

	/* the deepest child contains always the deepest filtered data.
	 */
	max_profiler_core<T_scoring, T_size_t, T_profile_elements> *getDeepestChild()
	{
		if ( pNextLevel == NULL )
		{
			return this;
		}
		else
		{
			return pNextLevel->getDeepestChild();
		}
	} // method

	void appendElementToProfile( T_scoring maxScoreInCurrentRow, T_size_t currentRow )
	{
		/* We compose the fresh element and store it in the profile
		 */
		T_profile_elements uxNewProfileElement = uxPack( maxScoreInCurrentRow, currentRow );
		
		profile[sizeOfProfile++] = uxNewProfileElement;
	} // method
	
	/* Push a fresh element into the profile. All elements that does not represent local maxima are automatically deleted.
	 */
	void push( T_scoring maxScoreInCurrentRow, T_size_t currentRow )
	{ 
		/* In the case we that the profile is full we move all content one layer down.
		 */
		if ( sizeOfProfile == capacityOfProfile )  
		{
			/* We shrink the data and flush, but leave a single element, so that the profile stays non-empty
			 */
			flush(); 
		} // if

		DEBUG_MSG( "push score:" <<  maxScoreInCurrentRow << " in row:" << currentRow );
		xLargestMaximumEverSeen = max( maxScoreInCurrentRow, xLargestMaximumEverSeen );
		/* This code monitors/saves local peeks in the scores distribution.
		 */
		if ( bMaxiumAhead )
		{
			if  ( maxScoreInCurrentRow < uxExtractScore( profile[sizeOfProfile - 1] ) )
			{
				 /* We see an decreasing score, so we switch the mode to "wait for new increase"
				  */
				 bMaxiumAhead = false;
				 localMinScore = maxScoreInCurrentRow;
			} // if level 2
			else
			{
				if (   ( maxScoreInCurrentRow == uxExtractScore( profile[sizeOfProfile - 1] ) )
					&& ( maxScoreInCurrentRow == xLargestMaximumEverSeen)
				   )
				{
					/* The current element and the previous element are equal. We are still looking for a maximum ahead.
					 * We store, but only if it belongs to the family of largest elements seen so far.
					 */ 
					appendElementToProfile( maxScoreInCurrentRow, currentRow );
				} // if level 3
				else
				{
					/* We see an increasing score. We overwrite the previous element.
					 * Security check
					 */
					if ( sizeOfProfile <= 0 )
					{
						std::cout << "Something is wrong with the profiler" << sizeOfProfile << std::endl;
						exit( 1 );
					}
					
					profile[sizeOfProfile - 1] = uxPack( maxScoreInCurrentRow, currentRow );
				} // else level 3
			} // else level 2
		} // outer if
		else
		{
			/* We are on some decreasing edge
			 */
			if ( maxScoreInCurrentRow <= localMinScore )
			{
				localMinScore = maxScoreInCurrentRow;
			} // inner if
			else
			{
				appendElementToProfile( maxScoreInCurrentRow, currentRow );
				bMaxiumAhead = true;
			} // inner else
		} // outer else
	} // method

	
	/* If we have to many equal maxima, we can get more than the requested number of elements
	 */
	void filterUntilRequestedSize( T_size_t requestedSizeOfProfile )
	{
		while( sizeOfProfile > requestedSizeOfProfile )
		{
			T_size_t  lengthOfProfileBackup = sizeOfProfile;
			inPlaceFiltering( 1 );

			if ( lengthOfProfileBackup == sizeOfProfile )
			{
				break;
			} // if
		} // while
	} // method

	/* function requires -std=c++0x switch with gcc
	 */
	template <class TP_FUNC_APPLY>
	void doForAllProfileEntries( TP_FUNC_APPLY&& f )
	{
		DEBUG_MSG( "call for all do with lengthOfProfile: " << sizeOfProfile );

		for( T_size_t vectorIterator = 0; vectorIterator < sizeOfProfile; vectorIterator++ )
		{
			f ( uxExtractRow( profile[vectorIterator] ), uxExtractScore( profile[vectorIterator] ) ); 
		} // for
	} // method

	void vSort()
	{
		/* This form of sorting is a bit inefficient. Improvement: Use an operator instead of the function reference, than we get inlineing.
		 */
		std::sort( this->profile, this->profile + sizeOfProfile, &max_profiler_core<T_scoring, T_size_t, T_profile_elements>::vCompareProfileElements ); 
	} // method

	/* Dump the content of the vector for debug purposes.
	 */
	void dump()
	{
		std::cout << "Profile Content:" << std::endl;
		doForAllProfileEntries( [&](T_size_t row, T_scoring score) 
								 { 
									 std::cout << "row:" << row  << "  max: " << score << std::endl; 
								 } 
							    );
	} // method

}; // struct

#if (CONF_USE_MALLOC == 1)
	template<class T_scoring, class T_size_t>
	class max_profiler : public max_profiler_core<T_scoring, T_size_t, uint64_t> 
	{
	public :
		/* first arg: largest element ever seen,
		 * second arg: initial capacity of profile
		 */
		max_profiler( ) :  max_profiler_core( 0, 100000 ) { }
	};
#else
	template<class T_scoring, class T_size_t>
	class max_profiler : public max_profiler_core<T_scoring, T_size_t, std::vector<uint64_t>> 
	{ };
#endif