#pragma once

#include <stdio.h>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <malloc.h>
#include <string.h>
#include "support.h"
#include "exception.h"
#include "tinyxml2.h"
#include "genetics.h"

/* The namespace tinyXmlUtil conatins many little help function for XML handling
 */
namespace tinyXmlUtil
{
	/* Dummy filter predicate.
	 */
	template<typename TP>
	inline bool bAlwaysTrue( TP argument )
	{
		return true;
	} // function

	template <typename TP_FUNC_APPLY, typename TP_FUNC_FILTER>
	void forSelfAndAllNextSiblingElementDo( tinyxml2::XMLElement* pElementRef, TP_FUNC_APPLY &&fApply, TP_FUNC_FILTER fFilter  ) 
	{
		while ( pElementRef != NULL )
		{
			if ( fFilter( pElementRef ) == true )
			{
				fApply( pElementRef );
			} // if
			pElementRef = pElementRef->NextSiblingElement();
		} // while
	} // function

	/* Applies the function fApply to all child elements of some given element reference.
	 */
	template <typename TP_FUNC_APPLY>
	void forAllChildElementsDo( tinyxml2::XMLElement* pElementRef, TP_FUNC_APPLY &&fApply )
	{
		if ( pElementRef != NULL )
		{
			forSelfAndAllNextSiblingElementDo( pElementRef->FirstChildElement(), fApply, bAlwaysTrue<tinyxml2::XMLElement*> );
		} // if
	} // function

	/* Little Debug function for the display of all element names
	 */
	void vDumpAllElementNamesOfChildren( tinyxml2::XMLElement* pElementRef )
	{
		forAllChildElementsDo
		( 
			pElementRef,
			/* Predicate function
			 */
			[&](tinyxml2::XMLElement *pSequenceElement ) 
			{
				std::cout << pSequenceElement->Name() << std::endl;
			} // lambda
		);
	} // function

	/* Applies the function fApply to all children of some element, where the predicate function delivers true.
	 * pElementRef can be NULL.
	 */
	template <typename TP_FUNC_APPLY, typename TP_FUNC_FILTER>
	void forFilteredChildElementsDo( tinyxml2::XMLElement* pElementRef, TP_FUNC_APPLY &&fApply, TP_FUNC_FILTER fPredicate )
	{
		if ( pElementRef != NULL )
		{
			forSelfAndAllNextSiblingElementDo( pElementRef->FirstChildElement(), fApply, fPredicate );
		} // if
	} // function

	/* Be careful, pElementRef is not allowed to be NULL.
	 * Assumes that the child of the given reference is a text node.
	 * Compares pcString to the text of the child-node.
	 */
	bool bCompareTextInsideElement( tinyxml2::XMLElement* pElementRef, const char *pcString )
	{
		const char* pcElementText = pElementRef->GetText();
		
		return ( pcElementText != NULL ) ? strcmp( pcString, pcElementText ) == 0
									     : false;
	} // function
	
	/* Tries to find some element starting from pElementRef, using the path syntax in path.
	 * WARNING!: Delivers NULL if the search fails.
	 * The function is a variant of the split-string function introduced elsewhere.
	 * TO DO: Improve performance by getting rid of the std::string stuff.
	 */
	tinyxml2::XMLElement* findElementByPath( tinyxml2::XMLElement* pElementRef, std::string path )
	{
		std::string bufferString;
		std::string::iterator iterator;

		for ( iterator = path.begin(); iterator < path.end(); iterator++ ) 
		{
			if ( (const char)*iterator != '.' ) 
			{
				bufferString += *iterator;
			} // if 
			else 
			{
				if ( pElementRef != NULL )
				{
					pElementRef = pElementRef->FirstChildElement( bufferString.c_str() );
				} // if
				else
				{
					return NULL;
				} // else
				bufferString.clear();
			} // else
		} // for

		if (! bufferString.empty() )
		{
			if ( pElementRef != NULL )
			{
				pElementRef = pElementRef->FirstChildElement( bufferString.c_str() );
			} // if
		} // if

		return pElementRef;
	} // function

	template <typename TP_FUNC_APPLY>
	void vDoIfPathLeadsToELement( tinyxml2::XMLElement* pElementRef, std::string path, TP_FUNC_APPLY &&fApply )
	{
		pElementRef = findElementByPath( pElementRef, path );
		
		if ( pElementRef != NULL )
		{
			fApply( pElementRef );
		} // if
	} // function

	/* Generic function for the parsing of text contents of XML-elements.
	 * Improvement: Here we could throw an exception if something goes wrong.
	 */
	template <typename TP_RETURN_VALUE, typename TP_FUNC_PARSE>
	TP_RETURN_VALUE genericGetByPath( tinyxml2::XMLElement* pElementRef, 
									  const char* pcPathAsString,
									  TP_FUNC_PARSE parsefun,
									  TP_RETURN_VALUE uxReturnedValue
									)
	{
		/* First we try to get the element...
		 */
		if ( ( pElementRef = tinyXmlUtil::findElementByPath( pElementRef, pcPathAsString ) ) != NULL)
		{
			/* ... and if we could find some, we try to get the text and parse it.
			 */
			const char *pcText = pElementRef->GetText();
			if ( pcText != NULL )
			{
				parsefun( pcText, &uxReturnedValue );
			} // inner if
		} // outer if

		return uxReturnedValue;
	} // function

	template <typename TP_FUNC>
	void doIfAttributeByPathDoesExists( tinyxml2::XMLElement* pElementRef, 
										const char* pcPathAsString,
										const char* pcAttributeName,
										TP_FUNC doFunction
									  )
	{
		if ( ( pElementRef = tinyXmlUtil::findElementByPath( pElementRef, pcPathAsString ) ) != NULL)
		{
			 const char* pcAttributeTextRef = pElementRef->Attribute( pcAttributeName );

			 if ( pcAttributeTextRef != NULL )
			 {
				 doFunction( pcAttributeTextRef );
			 } // inner if
		} // outer if
	} // generic function

	/* parse function for genericGetByPath.
	 */
	void cStringToStdString(const char* pcText, std::string* pStringRef )
	{
		pStringRef->append( pcText );
	} // function

	/* parse function for genericGetByPath.
	 */
	void CStringToCSTring(const char* pcText, const char **ppcText )
	{
		*ppcText = pcText;
	} // function

	/* Gets an parsed unsigned integer from some element.
	 */
	uint32_t getUnsignedByPath( tinyxml2::XMLElement* pElementRef, const char* pcPathAsString )
	{
		return genericGetByPath<uint32_t>( pElementRef, pcPathAsString, tinyxml2::XMLUtil::ToUnsigned, 0 );
	} // function
	
	/* Gets a text child as std:string.
	 */
	std::string getStringByPath( tinyxml2::XMLElement* pElementRef, const char* pcPathAsString )
	{
		return genericGetByPath<std::string>( pElementRef, pcPathAsString, cStringToStdString, "" );
	} // function

	/* Gets a text child as constant C-string.
	 */
	const char* getCStringByPath( tinyxml2::XMLElement* pElementRef, const char* pcPathAsString )
	{
		return genericGetByPath<const char *>( pElementRef, pcPathAsString, CStringToCSTring, "" );
	} // function
} // namespace tinyXmlUtil

/* Function for debug purposes:
 * tinyXmlUtil::vDumpAllElementNamesOfChildren( pEntrezgeneElementRef );
 */
namespace INSD_XMLUtil 
{
	/* Special tailored function for the loading of sequential XML records in INSD_XML files.
	 */
	template <typename TP_FUNC_APPLY>
	void doForXMLSequenceInINSD_XML( tinyxml2::XMLElement* pRelativeRootElement, 
									 TP_FUNC_APPLY &&fApply,
									 const char* pcPathToSequenceContainer,
									 const char* pcNameOfElementSelectedForStringComparsion,
									 const char* pcRequestedTextOfTextNode )
	{
		tinyXmlUtil::forFilteredChildElementsDo
		( 
			/* Starting from the given root, we search the start of the sequence of XML elements (given by pcPathToSequenceContainer).
			 */
			tinyXmlUtil::findElementByPath( pRelativeRootElement, pcPathToSequenceContainer ),  
			
			/* fApply is applied to all elements where the predicate delivers true.
			 */
			fApply,

			/* Predicate function
			 */
			[&](tinyxml2::XMLElement *pSequenceElement ) -> bool
			{
				/* We look for a candidate...
				 */
				auto pElementRef = tinyXmlUtil::findElementByPath( pSequenceElement, pcNameOfElementSelectedForStringComparsion );
				
				/* We found a matching element, now we have to compare the text child.
				 */
				if ( pElementRef != NULL )
				{
					return tinyXmlUtil::bCompareTextInsideElement( pElementRef, pcRequestedTextOfTextNode );
				} // if

				return false;
			} // lambda
		);
	} // function

	/* pElementRef must be a reference to a <INSDFeature>.
	 * The <INSDFeature> should have children called <INSDInterval_from> and <INSDInterval_to>.
	 * In the NCBI interval world the counting start with 1, where the end-index seems to be "inclusive".
	 * So start = 1 and end = 2 means internally start = 0 and end = 2, because our end-element is exclusive and we start counting with 0
	 * Edit:
	 * Now we have already cases, where NCBI seems to count from zero as well.
	 */
	IntervalDescriptor xCreateIntervalDescriptorFromINSDFeatureElement( tinyxml2::XMLElement* pINSDFeatureElementRef )
	{
		

		tinyxml2::XMLElement* pInterval = tinyXmlUtil::findElementByPath( pINSDFeatureElementRef, "INSDFeature_intervals.INSDInterval" );

		IntervalDescriptor intervalDescriptor( tinyXmlUtil::getUnsignedByPath( pInterval, "INSDInterval_from" ) - 1, // start ( !!! - 1 !!! )
											   tinyXmlUtil::getUnsignedByPath( pInterval, "INSDInterval_to" )        // end
											 );
		
		intervalDescriptor.sAccession	= tinyXmlUtil::getStringByPath( pInterval, "INSDInterval_accession" );

		return intervalDescriptor;
	} // method

	/* Special tailored function for loading single sequences in a <INSDSeq_feature-table> section.
	 */
	template <typename TP>
	void vLoadFromINSDSeq_feature_table( tinyxml2::XMLElement* INSDSeqElementRef,
										 std::vector<TP> &vector,
										 const char* pcINSDFeature_keyText
									   )
	{
		INSD_XMLUtil::doForXMLSequenceInINSD_XML
		( 
			/* A INSDSeq_feature-table is the child of a INSDSeq element.
			 */
			INSDSeqElementRef,

			/* element will be a reference to a INSDFeature element.
			 * exons is bound on object level
			 */
			[&](tinyxml2::XMLElement *pINSDFeatureElement) 
			{
				vector.push_back( xCreateIntervalDescriptorFromINSDFeatureElement( pINSDFeatureElement ) );
			}, // lambda
			
			/* The children of INSDSeq_feature-table are a sequence of INSDFeature elements.
			 */
			"INSDSeq_feature-table",

			/* We select all INSDFeature elements where the child INSDFeature_key contains a text element with content pcINSDFeature_keyText.
			 */
			"INSDFeature_key",
			pcINSDFeature_keyText
		);
	} // generic function

	/* Extracts the content of a <INSDSeq> element as mRNA.
	 */
	void vGet_mRNAFromINSDSeq( tinyxml2::XMLElement* pISNDSeqElementRef, GeneticCompound_mRNA &rCompoundmRNA )
	{
		/* We get the definition (description of our sequence data)
		 */
		rCompoundmRNA.sDefinition = tinyXmlUtil::getStringByPath( pISNDSeqElementRef, "INSDSeq_definition" );

		/* We get the RNA sequence data
		 */
		rCompoundmRNA.xRNASequence.vAppend( tinyXmlUtil::getCStringByPath( pISNDSeqElementRef, "INSDSeq_sequence" ) );

		/* We get the size of the RNA sequence data. This data can be used for data inconsistency checks.
		 * TO DO: Impelemnt such a consistency check.
		 */
		rCompoundmRNA.uxSequenceSize = tinyXmlUtil::getUnsignedByPath( pISNDSeqElementRef, "INSDSeq_length" );
		
		/* We get the exon data
		 */
		vLoadFromINSDSeq_feature_table( pISNDSeqElementRef, rCompoundmRNA.exonVector, "exon" );

		/* We get the CDS data
		 */
		vLoadFromINSDSeq_feature_table( pISNDSeqElementRef, rCompoundmRNA.cdsVector, "CDS" );

		/* We get the STS data
		 */
		vLoadFromINSDSeq_feature_table( pISNDSeqElementRef, rCompoundmRNA.stsVector, "STS" );
	} // function

	/* Loads the XML file with the name pcFileName and extracts a mRNA description out of the file.
	 */
	void vLoadINSD_XML( const char* pcFileName, GeneticCompound_mRNA &rCompoundmRNA )
	{
		tinyxml2::XMLDocument xmlDocument;
	
		/* We try to load the XML document from the given location
		 */
		xmlDocument.LoadFile( pcFileName );

		if ( xmlDocument.ErrorID() != tinyxml2::XML_SUCCESS )
		{
			/* Something went wrong ...
			 * We throw some exception
			 */
			throw fasta_reader_exception( "XML file loading failed" );
		} // if
		
		/* We load from the first sequence
		 */
		INSD_XMLUtil::vGet_mRNAFromINSDSeq( tinyXmlUtil::findElementByPath( xmlDocument.RootElement(), "INSDSeq" ), rCompoundmRNA );
	} // function
} // namespace INSD_XMLUtil 


/* Code for loading gene information in a NCBI XML-file that start with a <Entrezgene-Set> element
 */
namespace NCBI_Entrezgene
{
	/* pElementRef must be a reference to a <Seq-interval>.
	 * TO DO: Read the strand. (Reverse or forward)
	 */
	DoubleStrandIntervalDescriptor xCreateIntervalDescriptorFromSeq_interval( tinyxml2::XMLElement* pSeq_intervalElementRef )
	{
		/* WARNING: Counting starts here suddenly with 0. (Normally the NCBI world starts with 1)
		 * So from 0 to 2 means from the first until the third (inclusive)
		 * In our internal representation 0, 3 because we take the first element after.
		 */
		DoubleStrandIntervalDescriptor intervalDescriptor( tinyXmlUtil::getUnsignedByPath( pSeq_intervalElementRef, "Seq-interval_from" ),  // start
														 tinyXmlUtil::getUnsignedByPath( pSeq_intervalElementRef, "Seq-interval_to" ) + 1 // end
 													   );
		/* TO DO: change this part here to id
		 */
		intervalDescriptor.sAccession	= tinyXmlUtil::getStringByPath( pSeq_intervalElementRef, "Seq-interval_id.Seq-id.Seq-id_gi" );

		tinyXmlUtil::doIfAttributeByPathDoesExists
		(	
			pSeq_intervalElementRef,			// start element ref
			"Seq-interval_strand.Na-strand",	// path
			"value",							// attribute value
			[&] (const char* pcAttributeTextRef)
			{
				intervalDescriptor.eStrand =   strcmp( pcAttributeTextRef, "plus"  ) == 0 ? ChromosomeSequence::IS_FORWARD_STRAND 
											 : strcmp( pcAttributeTextRef, "minus" ) == 0 ? ChromosomeSequence::IS_REVERSE_STRAND
											 : ChromosomeSequence::UNKNOWN_STRAND;
			}
		);

		return intervalDescriptor;
	} // method
	
	/* Special tailored function for loading many <Sec-loc> sections in a <Seq-loc-mix> section.
	 */
	template <typename TP>
	void vExtractIntervalsFromSeq_loc_mix( tinyxml2::XMLElement* Seq_loc_mixElementRef,
									   std::vector<TP> &vector
									 )
	{
		tinyXmlUtil::forAllChildElementsDo
		( 
			/* We iterate over the children of Seq_loc_mixElementRef
			 */
			Seq_loc_mixElementRef,  
			
			/* pSeq_locElementRef refers a <Sec-loc> element
			 * We get the single <Seq-loc_int> child of <Sec-loc> and read all intervals
			 */
			[&]( tinyxml2::XMLElement *pSeq_locElementRef ) 
			{
				vector.push_back( xCreateIntervalDescriptorFromSeq_interval( tinyXmlUtil::findElementByPath( pSeq_locElementRef, "Seq-loc_int.Seq-interval" ) ) );
			} // lambda
		);
	} // generic function

	/* Gets all intervals below 
	 * starting by <Gene-commentary_genomic-coords> : "Gene-commentary_genomic-coords.Seq-loc.Seq-loc_mix.Seq-loc-mix"
	 * starting by <Gene-commentary_seqs>			: "Gene-commentary_seqs.Seq-loc" 
	 */
	template <typename TP>
	void vExtractIntervalsOfGene_commentary( tinyxml2::XMLElement* pGene_commentaryElementRef,
										 std::vector<TP> &vector,
										 const char* pcPath
									   )
	{
		vExtractIntervalsFromSeq_loc_mix
		(
			tinyXmlUtil::findElementByPath( pGene_commentaryElementRef, pcPath ),
			vector
		);
	} // generic function

	/* Reads a commentary below the given <Gene-commentary> element.
	 * In the moment we read here the locus information.
	 */
	void vExtractGene_commentary( tinyxml2::XMLElement* pGen_commentaryElementRef, std::shared_ptr<GeneLocusDescriptor> const &pLocusDescriptorRef )
	{
		/* Get the classification e.g. genomic or mRNA
		 */
		pLocusDescriptorRef->eNCBIClassification = (GeneLocusDescriptor::eNCBIClassificationType)tinyXmlUtil::getUnsignedByPath( pGen_commentaryElementRef, "Gene-commentary_type" );

		/* TExtual description of the info in the Gene-commentary.
		 */
		pLocusDescriptorRef->sLabel = tinyXmlUtil::getStringByPath( pGen_commentaryElementRef, "Gene-commentary_label" );

		/* NCBI reference code
		 */
		pLocusDescriptorRef->sNCBIAccession = tinyXmlUtil::getStringByPath( pGen_commentaryElementRef, "Gene-commentary_accession" );
		
		/* We read all the intervals that belong to the comment.
		 * Here we have to be careful, because the intervals are on different location for different types!
		 */
		vExtractIntervalsOfGene_commentary
		( 
			pGen_commentaryElementRef, 
			pLocusDescriptorRef->locationsVector, 
			
			pLocusDescriptorRef->eNCBIClassification == GeneLocusDescriptor::GENOMIC 
				? "Gene-commentary_seqs"											// for genomic comments the sequences are here
				: "Gene-commentary_genomic-coords.Seq-loc.Seq-loc_mix.Seq-loc-mix"	// otherwise we get them here
		);

		if ( pGen_commentaryElementRef->FirstChildElement( "Gene-commentary_products" ) != NULL )
		{
			/* There is a <Gene-commentary_products>. So, we recursively parse this comment.
			 * In the moment we parse only the first child. (TO DO: Check the DTD, if we can have more than one product)
			 */
			vExtractGene_commentary
			(  
				tinyXmlUtil::findElementByPath( pGen_commentaryElementRef, "Gene-commentary_products.Gene-commentary" ),

				/* get product creates a sub-object, if required.
				 */
				pLocusDescriptorRef->pGetProduct()
			);
		} // if
	} // function

	void vFindNCBI_Entrezgene_geneElementAndGetData( tinyxml2::XMLElement* pEntrezgeneElementRef, geneDescriptor &rGene )
	{
		tinyXmlUtil::vDoIfPathLeadsToELement
		(	pEntrezgeneElementRef,		// start element ref
		    "Entrezgene_gene.Gene-ref",	// path
			
			[&]( tinyxml2::XMLElement *pGene_refElementRef ) 
			{
				/* Store the result in the interpretations section of the gene record.
				 */
				rGene.sReferenceDescriptions = tinyXmlUtil::getStringByPath( pGene_refElementRef, "Gene-ref_desc" );
			} // lambda
		); // function call
		
	} // function

	/* gets genom information of a NCBI XML-tree with root element <Entrezgene-Set>
	 * (pEntrezgeneElementRef should refer some <Entrezgene> element)
	 */
	void vFindNCBI_Entrezgene_locusElementAndGetData( tinyxml2::XMLElement* pEntrezgeneElementRef, geneDescriptor &rGene )
	{
		tinyXmlUtil::vDoIfPathLeadsToELement
		(	pEntrezgeneElementRef,	// start element ref
		    "Entrezgene_locus",		// path
			
			[&]( tinyxml2::XMLElement *pEntrezgene_locusElementRef ) 
			{
				/* We have a <Entrezgene_locus> section, so we create a new locus descriptor.
				 * We work here with shared pointers in order to get automatic garbage collection.
				 */
				std::shared_ptr<GeneLocusDescriptor> pLocusDescriptorRef = std::make_shared<GeneLocusDescriptor>();

				/* Take the Gene-commentary child and parse locus information.
				 */
				vExtractGene_commentary( pEntrezgene_locusElementRef->FirstChildElement( "Gene-commentary" ), pLocusDescriptorRef );

				/* Store the result in the interpretations section of the gene record.
				 */
				rGene.geneLocusDescriptions = pLocusDescriptorRef;
			} // lambda
		);
	} // function

	/* Loads the XML file with the name pcFileName and extracts a gene description from the XML content.
	 */
	void vLoadNCBI_Entrezgene( const char* pcFileName, geneDescriptor &rGene  )
	{
		tinyxml2::XMLDocument xmlDocument;
	
		/* We try to load the XML document from the given location
		 */
		xmlDocument.LoadFile( pcFileName );

		if ( xmlDocument.ErrorID() != tinyxml2::XML_SUCCESS )
		{
			/* Something went wrong ...
			 * We throw some exception
			 */
			throw fasta_reader_exception( "XML file loading failed" );
		} // if
		
		/* We get the <Entrezgene> in the XML tree and keep a reference to this element
		 */
		tinyxml2::XMLElement* pEntrezgeneElementRef = tinyXmlUtil::findElementByPath( xmlDocument.RootElement(), "Entrezgene" );

		/* We get the data contained in the <Entrezgene_locus> section
		 */
		vFindNCBI_Entrezgene_locusElementAndGetData( pEntrezgeneElementRef, rGene );
		
		/* We get the data contained in the <Entrezgene_gene> section
		 */
		vFindNCBI_Entrezgene_geneElementAndGetData( pEntrezgeneElementRef, rGene );
	} // function
} // namespace NCBI_Entrezgene



