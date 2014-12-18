#pragma once


#include <vector>
#include <algorithm>
#include "support.h"
#include "sequence.h"
#include "fasta_reader.h"


/* DistanceDescriptor is for keeping data of ans start-end form
 */
class IntervalDescriptor
{
public :
	const uint32_t uxStart;
	const uint32_t uxEnd;
	std::string sAccession;

	IntervalDescriptor( uint32_t uxStart, uint32_t uxEnd ) : uxStart(uxStart), uxEnd(uxEnd)
	{ } // constructor
	
	inline void vStringAppendTextualDescription( std::string &string )
	{
		string.append("Accession/ID :").append( sAccession )
			  .append(" start : ")  .append( numberToString( (signed)uxStart ) )
			  .append(" end : ")    .append( numberToString( (signed)uxEnd ) )
			  .append(" size : ")	.append( numberToString( (signed)uxEnd - (signed)uxStart ) );
	} // method
}; // class

/* Intervals on chromosomes are additionally on the forward or reverse strand
 * This class keeps this additional information.
 */
class DoubleStrandIntervalDescriptor : public IntervalDescriptor
{
public :
	ChromosomeSequence::tStrandType eStrand;

	DoubleStrandIntervalDescriptor( uint32_t uxStart, uint32_t uxEnd ) : IntervalDescriptor( uxStart, uxEnd )
	{ } // constructor

	inline void vStringAppendTextualDescription( std::string &string )
	{
		IntervalDescriptor::vStringAppendTextualDescription( string );

		string.append(" strand : ").append( ChromosomeSequence::pcTextForStrandType( eStrand ) );
	} // method
};

/* Abstract class for general Distance related data collections.
 */
template <typename BASECLASS>
class IntervalVector : public std::vector<BASECLASS> 
{
private :
	/* IntervalDescriptorVector(const IntervalDescriptorVector&);
	 * Override this in a subclass, so that it delivers the appropriate value.
	 * This should give a textual description of the nature of data hold in the vector.
	 */
	virtual std::string sName()
	{
		return std::string( "Intervals" );
	} // method

public :
	/* For getting a textual representation of our sequence data
	 */
	void vStringAppendTextualDescription( std::string &string )
	{
		string.append( sName() ).append( "\n" );
		std::for_each
		(	begin(), 
			end(), 
			( [&] ( BASECLASS descriptor ) 
				{ 
					descriptor.vStringAppendTextualDescription( string ); 
					string.append("\n");
				} // lambda
			) 
		); // for_each
	} // method

	/* Debug function for dumping an interval vector
	 */
	void vDumpToCout()
	{
		std::string string;
		vStringAppendTextualDescription( string );
		std::cout << string << std::endl;
	} // method
}; // class

/* Vector class for keeping intron related interval information
 */
class IntronVector : public IntervalVector<DoubleStrandIntervalDescriptor>
{
public :
	std::string sName( )
	{
		return std::string( "Introns" );
	} // method
}; // class

/* Vector class for keeping excon related interval information
 */
class ExonVector : public IntervalVector<IntervalDescriptor>
{
public :
	std::string sName( )
	{
		return std::string( "Exons" );
	} // method
}; // class

/* Coding Sequence (CDS) is the translated area of a mRNA or DNA. 
 * (Should start with a start cordon.)
 * Normally this vector should contain only one single element.
 */
class CDSVector : public IntervalVector<IntervalDescriptor>
{
public :
	std::string sName( )
	{
		return std::string( "Coding Sequence (CDS)" );
	} // function
}; // class

/* From: http://en.wikipedia.org/wiki/Sequence-tagged_site
 * A sequence-tagged site (or STS) is a short (200 to 500 base pair) DNA sequence that has a single occurrence 
 * in the genome and whose location and base sequence are known.
 * Normally this vector should contain only one single element.
 */
class STSVector : public IntervalVector<IntervalDescriptor>
{
public :
	std::string sName( )
	{
		return std::string( "Sequence-Tagged Site (STS)" );
	} // method
}; // class

/* Objects of this class describe mRNAs
 */
class GeneticCompound_mRNA
{
public:
	/* If we have exon data, we save them here as vector of distance info.
	 */
	ExonVector exonVector;
	
	/* The Coding Sequence (CDS).
	 * This sequence should normally consists of a single element merely
	 */
	CDSVector cdsVector;
	
	/* The sequence-tagged site (or STS).
	 * This sequence should normally consists of a single element merely
	 */
	STSVector stsVector;

	/* The name / textual identification of our mRNA.
	 */
	std::string sDefinition;

	/* The complete genetic sequence of the mRNA
	 */
	GeneticSequence xRNASequence;

	/* The size of the sequence
	 */
	uint32_t uxSequenceSize;

public:	
	/* In the context of the constructor we create an empty sequence only.
	 * The sequence has to filled later by vAppend operations.
	 */
	GeneticCompound_mRNA( ) : xRNASequence( GeneticSequence::SEQUENCE_IS_NUCLEOTIDE )
	{
	} // constructor

	/* The CDS section of a RNA as sequence of nucleotide codes [0 .. 3]
	 */
	GeneticSequenceSlice getCDSAsRNASequence()
	{
		return GeneticSequenceSlice( xRNASequence, cdsVector[0].uxStart, cdsVector[0].uxEnd );
	} // method

	/* The 3UTR of a mRNA inclusive the polyAtail.
	 */
	GeneticSequenceSlice get3UTR_RNASequenceInclusicePolyATail()
	{
		return GeneticSequenceSlice( xRNASequence, cdsVector[0].uxEnd, xRNASequence.uxGetSequenceSize() );
	} // method

	/* Delivers a textual representation of our mRNA object.
	 */
	void vStringAppendTextualDescription( std::string &string )
	{
		string.append(sDefinition).append("\n");
		exonVector.vStringAppendTextualDescription( string );
		cdsVector.vStringAppendTextualDescription( string );
		stsVector.vStringAppendTextualDescription( string );
		string.append("Full-Sequence  : ").append( xRNASequence.fullSequence().asString() );
		string.append("\nCDS-Section  :").append( getCDSAsRNASequence().asString() );
		string.append("\nCDS-Size     :").append( numberToString( getCDSAsRNASequence().uxGetSequenceSize() ) );
		string.append("\nFull 3UTR    :").append( get3UTR_RNASequenceInclusicePolyATail().asString() );
	} // method

	/* Debug function for dumping the content of a mRNA
	 */
	void vDumpToCout()
	{
		std::string string;
		vStringAppendTextualDescription( string );
		std::cout << string << std::endl;
	} // method
}; // class

/* Descriptor that keep gene location related data.
 * In NCBI-XML files these data are assembled by reading comments.
 * This data structure is hierarchically organized, due to the nature of the stored data.
 * Example genomic -> mRNA -> peptide
 */
class GeneLocusDescriptor
{
private:
	/* We block the copy constructor, for security reasons.
	 */
	GeneLocusDescriptor(const GeneLocusDescriptor&);
	
	/* If there is a further (more specialized) description it is anchored here.
	 * Example genomic -> mRNA -> peptide
	 */
	std::shared_ptr<GeneLocusDescriptor> pProductRef;

	static const char *classificationNames[9];

public:
	typedef enum {
		GENOMIC	= 1,
		MRNA	= 3,
		PEPTIDE = 8
	} eNCBIClassificationType;

	/* Delivers a textual descripton for some classification.
	 */
	const char *textForClassifcation( eNCBIClassificationType eClassification )
	{
		if ( eClassification <= PEPTIDE )
		{
			return classificationNames[eClassification];
		} // if
		else
		{
			return "UNKOWN";
		} // else
	} // method

	/* The Locations (expressed as sequence of intervals of the genetic data)
	 */
	IntervalVector<DoubleStrandIntervalDescriptor> locationsVector;

	std::string sLabel;
	
	/* 1 == genomic, 3 == mRNA, 6 == peptide
	 */
	eNCBIClassificationType eNCBIClassification;

	/* NCBI reference code.
	 * This should be a reference to the location of a genetic host sequence.
	 */
	std::string sNCBIAccession;

	GeneLocusDescriptor() : pProductRef( nullptr )
	{ 	
	} // constructor

	std::shared_ptr<GeneLocusDescriptor> pGetProduct()
	{
		return pProductRef != nullptr ?  pProductRef : pProductRef = std::make_shared<GeneLocusDescriptor>();
	} // method

	/* Destructor must be virtual, due to later polymorphism
	 */
	virtual ~GeneLocusDescriptor()
	{
	} // destructor

	/* Creates a string representation of the locus descriptor
	 */
	void vStringAppendTextualDescription( std::string &string ) 
	{
		string.append( "Locus information for: " ).append( textForClassifcation((eNCBIClassificationType)eNCBIClassification) ).append( "\n" )
			  .append( "NCBIAccession: ").append( sNCBIAccession ).append( "\n" );

		locationsVector.vStringAppendTextualDescription( string );

		/* Visit the sub-element, if existing
		 */
		if ( pProductRef != nullptr )
		{
			pProductRef->vStringAppendTextualDescription( string);
		} // if
	} // method

	/* Looks whether some of the childes or the element itself has the correct classification.
	 * If the search fails you get a NULL pointer.
	 */
	GeneLocusDescriptor* pGetLocusDescriptorAccordingToClassification( eNCBIClassificationType eClassification )
	{
		return   ( this->eNCBIClassification == eClassification ) ? this
			   : ( this->pProductRef != nullptr)				  ? pProductRef->pGetLocusDescriptorAccordingToClassification( eClassification )
			   : NULL;		
	} // method

}; // class

class geneDescriptor
{
public :
	/* The NCBI reference description of the gene.
	 */
	std::string sReferenceDescriptions;

	/* Here we store the locus information of NCBI.
	 */
	std::shared_ptr<GeneLocusDescriptor> geneLocusDescriptions;

	geneDescriptor() : geneLocusDescriptions( nullptr )
	{
	} // default constructor

	/* Generic static function for iterating over all intervals of some given classification.
	 */
	template <typename TP_FUNC_APPLY>
	static void doForAllIntervalsOfClassification( std::shared_ptr<GeneLocusDescriptor> geneLocusDescriptions,
												   GeneLocusDescriptor::eNCBIClassificationType eClassification, 
												   TP_FUNC_APPLY &&fApply
												 )
	{
		if ( geneLocusDescriptions != nullptr )
		{
			/* We try to find a mRNA section in the locus descriptions.
				*/
			GeneLocusDescriptor* pmRNALocusRef = geneLocusDescriptions->pGetLocusDescriptorAccordingToClassification( GeneLocusDescriptor::MRNA );
			
			if ( pmRNALocusRef != nullptr )
			{
				std::for_each
				(	pmRNALocusRef->locationsVector.begin(), 
					pmRNALocusRef->locationsVector.end(), 
					[&] ( DoubleStrandIntervalDescriptor &interval ) 
						{ 
							fApply( interval );
						} // lambda
				); // for_each
			} // inner if
		} // outer if
	} // static method

	/* We extract the introns by using the information for exons for a mRNA.
	 */
	std::shared_ptr<IntronVector> getIntronsFromExonsFor_mRNA()
	{
		size_t uxStart, uxEnd;
		bool isFirstExonInterval = true;
		
		std::shared_ptr<IntronVector> pIntronVectorRef = std::make_shared<IntronVector>();
		
		doForAllIntervalsOfClassification
		(	
			/* We start the search on geneLocusDescriptions of our current object
			 */
			this->geneLocusDescriptions,
			GeneLocusDescriptor::MRNA,
			[&] ( DoubleStrandIntervalDescriptor &exonInterval )
			{
				/* For the first exon interval we skip the intron creation
				 */
				if ( isFirstExonInterval == false )
				{
					uxEnd = exonInterval.uxStart;
					
					if ( exonInterval.eStrand == ChromosomeSequence::IS_REVERSE_STRAND )
					{
						/* For the reverse strand we have to swap start and end in order to
						 * get a positive interval size.
						 */
						std::swap( uxStart, uxEnd );
					}
					/* We have a start and endpoint of an intron
					 */
					pIntronVectorRef->push_back( DoubleStrandIntervalDescriptor( uxStart, uxEnd ) );
				} // if
				else
				{
					/* We got the first exon interval, we ignore this interval and change the appropriate flag.
					 */
					isFirstExonInterval = false;
				} // else
				
				uxStart = exonInterval.uxEnd;
			} // lambda
		); // function call

		return pIntronVectorRef;
	} // method

	/* If the gene locus data contain information for a mRNA, this method collects these data and constructs the mRNA by using the
	 * host sequence referred in the genomic section.
	 * WARNING: If we fetch these data from some chromosome, this process can last quite long, because we have to read the complete chromosome.
	 * TO DO: Implement a version that works with reverse strand as well.
	 */
	std::shared_ptr<GeneticSequence> compose_mRNA()
	{
		/* We create an empty sequence encapsulated by creating a shared pointer that we will deliver as result.
		 */
		std::shared_ptr<GeneticSequence> pResultingSequenceRef = std::make_shared<GeneticSequence>(GeneticSequence::SEQUENCE_IS_NUCLEOTIDE);

		/* The info about the required data comes from the genomic section
		 */
		FastaReader sequenceReader;
		sequenceReader.vLoadFastaFile("../../../Data/NC_000001.10.fasta" );
		ChromosomeSequence hostSequence( sequenceReader.sequence );
		
		doForAllIntervalsOfClassification
		(	
			/* We start the search on geneLocusDescriptions of our current object
			 */
			this->geneLocusDescriptions,
			GeneLocusDescriptor::MRNA,
			[&] ( IntervalDescriptor &interval ) 
				{ 
					/* According to the interval data info we take a section out of the host-sequence and 
						* append this section to the resulting sequence.
						*/
					pResultingSequenceRef->PlainSequence::vAppend
					( 
						hostSequence.uxGetSequence() + interval.uxStart, 
						(size_t)( interval.uxEnd - interval.uxStart )
					); // function arguments
				} // lambda
		); // function call
		
		return pResultingSequenceRef;
	} // method

	/* Debug function for dumping the content of a gene
	 */
	void vDumpToCout()
	{
		std::string string; 
		std::cout << "GENE : " << sReferenceDescriptions << std::endl;
		
		if ( geneLocusDescriptions != nullptr )
		{
			geneLocusDescriptions->vStringAppendTextualDescription( string );
		} // if
		
		std::cout << string << std::endl;
	} // method
};