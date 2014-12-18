#include "sw_workframe.h"
#include "tinyxml2.h"
#include "exception.h"
#include "NCBI_xml.h"
#include "genetics.h"
#include <string>


#if 0
void applyAlignmentUsingStringsTest( alignment_description<char> &alignmentOutcome ) 
{
	char columnSequence[] = "AGCATA"; // "CTGG";  //  "ACACACTA"; //determines the number of columns
	char rowSequence[] = "CTAGAGTGTGACTAGATCTAGCCTAGATATGCCTATGAAGCAAA"; // "CATTG"; // "AGCACACA"; // determines the number of rows

	size_t columnSequenceLen = strlen( columnSequence );
	size_t rowSequenceLen = strlen( rowSequence );

	translateSequence( columnSequence );
	translateSequence( rowSequence );

	/* We create the aligner object. (Here we reserve our memory)
	 */
	size_t indexOfMaxElementInScoringTable;

	SW_align_type<true, int32_t, size_t>aligner( columnSequenceLen, (const uint8_t *)columnSequence, 
												 rowSequenceLen, (const uint8_t *)rowSequence, 
												 3,					
												 1 
											   );
	int32_t maxScore = aligner.swAlign(NULL, NULL, indexOfMaxElementInScoringTable);
	
	size_t startPositionInColumn, endPositionInColumn, startPositionInRow, endPositionInRow;
	
	/* In the context of the backtracking we get text.
	 */
	aligner.scoringMatrix.backtrackFromIndex( indexOfMaxElementInScoringTable,
											  alignmentOutcome,
											  startPositionInColumn, 
											  endPositionInColumn,
											  startPositionInRow, 
											  endPositionInRow );
	

#if 1
	/* DEBUGGING: We dump the matrix!
	 */
	aligner.scoringMatrix.dump();
#endif

	cout << "MAXIMUM :" << maxScore << endl; 
	cout << "Value at"  << indexOfMaxElementInScoringTable <<  " : " << aligner.scoringMatrix.scoringOutcomeMatrix[indexOfMaxElementInScoringTable] << endl; 
}
#endif

// /usr/bin/codelite_xterm '$(TITLE)' '$(CMD)'

// std::shared_ptr<A> ptr(new A(5));

using namespace tinyxml2;

void example_1()
{
	tinyxml2::XMLDocument doc;
	doc.LoadFile( "../../../Data/NM_001123168.xml" );

#if 0
	if ( doc.ErrorID() != tinyxml2::XML_SUCCESS )
	{
		throw fasta_reader_exception( "XML file loading failed" );
	} // if

	tinyxml2::XMLElement* titleElement = doc.FirstChildElement( "INSDSet" ); //->FirstChildElement( "INSDSeq" )->FirstChildElement( "INSDSeq_locus" );

	std::vector<std::string>vector;
	split( vector, "INSDSeq.INSDSeq_locus", '.' );

	std::for_each( vector.begin(),vector.end(), ( [&](std::string string) 
	{
		titleElement = titleElement->FirstChildElement( string.c_str() );
	} ) );

	const std::string title( titleElement->GetText() ) ;


	titleElement = doc.FirstChildElement( "INSDSet" );

	getElementByPath( "INSDSeq.INSDSeq_length", ( [&](std::string string) 
	{
		titleElement = titleElement->FirstChildElement( string.c_str() );
	}
		)
	);
#endif
	//titleElement= tinyXmlUtil::getElementByPath(titleElement, "INSDSeq.INSDSeq_length" );

	//const std::string title( titleElement->GetText() ) ;

	// std::cout << title << std::endl;
}

void vExploreGene( const char* pcFileNameRef )
{
	geneDescriptor gene;
	NCBI_Entrezgene::vLoadNCBI_Entrezgene( pcFileNameRef, gene );
	gene.vDumpToCout();
	// std::shared_ptr<GeneticSequence> pResultingSequenceRef = gene.compose_mRNA();
	std::shared_ptr<IntronVector> introns = gene.getIntronsFromExonsFor_mRNA();
	introns->vDumpToCout();
}

int main( ) 
{
#if 0
	applyAlignmentUsingStringsTest( alignmentOutcome );
	std::cout << "Alignment finished ..." << std::endl;
	std::cout << alignmentOutcome.rsAppendHTMLRepesentation( string );
#endif

#if 0
	FastaReader rowReader;
	
	rowReader.vLoadFastaFile("../../../Data/NC_000001.10.fasta" );
	std::cout << "size: " << rowReader.sequence.uxGetSequenceSize() << std::endl;

	//ChromosomeSequence cseq(rowReader.sequence);
	//GeneticSequenceSlice slice( cseq, 206138910, 206138950 );
	//std::cout << slice.asString() << std::endl;

	alignment<int16_t>( ChromosomeSequence( TextSequence("ATGGAATTATGATATATATGATAT") ).fullSequence() , 
						ChromosomeSequence( rowReader.sequence ).fullSequence()  
					  );
#endif

#if 1
	GeneticCompound_mRNA seq;
	INSD_XMLUtil::vLoadINSD_XML( "../../../Data/NM_001123168.xml", seq );
	seq.vDumpToCout();

	GeneticCompound_mRNA seq1;
	INSD_XMLUtil::vLoadINSD_XML( "../../../Data/NM_001100910.1.xml", seq1 );
	seq1.vDumpToCout();

	GeneticCompound_mRNA seq2;
	INSD_XMLUtil::vLoadINSD_XML( "../../../Data/NM_001287385.1.xml", seq2 );
	seq2.vDumpToCout();

	GeneticCompound_mRNA seq3;
	INSD_XMLUtil::vLoadINSD_XML( "../../../Data/NM_207418.2.xml", seq3 );
	seq3.vDumpToCout();
#endif

#if 0
	alignment<int16_t>( seq3.get3UTR_RNASequenceInclusicePolyATail(),
		seq.get3UTR_RNASequenceInclusicePolyATail() 
						  
	);
#endif

#if 1
	vExploreGene( "../../../Data/729533.xml" );
	vExploreGene( "../../../Data/554282.xml" );
	vExploreGene( "../../../Data/653820.xml" );
	vExploreGene( "../../../Data/728833.xml" );
	
	/*
	geneDescriptor gene;
	NCBI_Entrezgene::vLoadNCBI_Entrezgene( "../../../Data/729533.xml", gene );
	gene.vDumpToCout();
	std::shared_ptr<GeneticSequence> pResultingSequenceRef = gene.compose_mRNA();
	std::shared_ptr<IntronVector> introns = gene.getIntronsFromExonsFor_mRNA();
	introns->vDumpToCout();
	

	GeneticCompound_mRNA seq4;
	INSD_XMLUtil::vLoadINSD_XML( "../../../Data/NM_001123168.xml", seq4 );

	std::cout << "Size of Resulting Sequence:" << pResultingSequenceRef->uxGetSequenceSize() << std::endl;
	
	std::cout << "Equality of Sequences:" << (pResultingSequenceRef->fromTo(0, 1867)).equals(seq.xRNASequence.fromTo(0, 1867)) << std::endl;
	*/
	
	
	/* alignment<int16_t>( pResultingSequenceRef->fromTo(0, 1867),
		seq4.xRNASequence.fromTo(0,1867) );
		*/
#endif

	
	try
	{
		example_1();
	} // try
	catch (std::exception &exception)
	{
		std::cout << exception.what();
	} //catch
}

