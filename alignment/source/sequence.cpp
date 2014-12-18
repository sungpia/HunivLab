#include "sequence.h"



/* Delivers a slice that spans over the complete sequence.
 */
GeneticSequenceSlice GeneticSequence::fullSequence()
{
	return GeneticSequenceSlice( *this, 0, this->uxSize );
} // method

/* TO DO: Check the index for correct range.
 */
GeneticSequenceSlice GeneticSequence::fromTo( size_t uxFrom, size_t uxEnd )
{
	return GeneticSequenceSlice( *this, uxFrom, uxEnd - uxFrom );
} // methid

