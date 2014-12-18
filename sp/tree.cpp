
#include "tinyxml2.h"
#include <iostream>
#include <string>
#include <vector>
#include <exception>

class classHTMLNode{
public:
	std::vector<classHTMLNode*> vChildAddress; // To traverse Next Child
	std::string sTagName; // Tag name as <HTML>, <HEAD> ..
	std::string sAttributeType; // attribute type as style
	std::string sAttribute; // attribute name as "color=red" ...
	tinyxml2::XMLNode* xXmlNodeAddress;

	classHTMLNode();
	~classHTMLNode();
private:

};
classHTMLNode::classHTMLNode()
{
	sAttributeType = "NA"; // default as no attribute 
	xXmlNodeAddress = NULL;
}
classHTMLNode::~classHTMLNode()
{

}

classHTMLNode* funcInitTree()
{
	try
	{
		classHTMLNode* ROOT = new classHTMLNode;
		ROOT->sTagName = "HTML";
		classHTMLNode* HEAD = new classHTMLNode;
		classHTMLNode* BODY = new classHTMLNode;
		ROOT->vChildAddress.push_back(HEAD);
		ROOT->vChildAddress.push_back(BODY);
		HEAD->sTagName = "HEAD";
		BODY->sTagName = "BODY";
		return ROOT;
	}//end of try
	catch(std::exception& e)
	{

	}//end of catch
}
void funcTraverseTree(classHTMLNode* pNodeRef, tinyxml2::XMLDocument* pXHTMLDocumentRef, tinyxml2::XMLNode* pElementRef)
{
	//Insert into XML
	tinyxml2::XMLElement* xmlElementToInsert = pXHTMLDocumentRef->NewElement(pNodeRef->sTagName.c_str());
	
	//if attribute exists
	if(pNodeRef->sAttributeType != "NA")
	{
		xmlElementToInsert->SetAttribute(pNodeRef->sAttributeType.c_str(), pNodeRef->sAttribute.c_str());
		/* example :  sAttribute Type = "style"
		*				sAttribute = "color=red"
		* result =		style = "color=red"
		*/
	} // end of if

	//Inserting from Tree to XML
	if(pElementRef == NULL)
	{
		pNodeRef->xXmlNodeAddress = pXHTMLDocumentRef ->InsertEndChild( xmlElementToInsert);
	} // end of if
	else
	{
		pNodeRef->xXmlNodeAddress = pElementRef;
		pElementRef->InsertEndChild(xmlElementToInsert);
	} // end of else
	
	//post order traversal
	for(unsigned xNextAddress = 0 ; xNextAddress < pNodeRef->vChildAddress.size() ; ++xNextAddress)
	{
		funcTraverseTree(pNodeRef->vChildAddress[xNextAddress], pXHTMLDocumentRef, pNodeRef->xXmlNodeAddress);
	}// end of for
}
int main(void)
{
	tinyxml2::XMLDocument* HTML = new tinyxml2::XMLDocument;
	
	
	
	classHTMLNode* ROOT;
	ROOT = funcInitTree();
	funcTraverseTree(ROOT, HTML, NULL);
	HTML->Print();


	tinyxml2::XMLDocument HTML_Document;
	tinyxml2::XMLNode* pRootHTMLElementRef = HTML_Document.InsertEndChild( HTML_Document.NewElement( "HTML") );
	tinyxml2::XMLNode* pHEADElementRef = pRootHTMLElementRef->InsertEndChild( HTML_Document.NewElement( "HEAD") );
	tinyxml2::XMLNode* pBODYElementRef = pRootHTMLElementRef->InsertEndChild( HTML_Document.NewElement( "BODY") );
	tinyxml2::XMLNode* pNOBRElementRef =  pBODYElementRef->InsertEndChild( HTML_Document.NewElement( "nobr") );

	for ( unsigned int uxIterator = 0;  uxIterator < 1000; uxIterator++ )
	{
		tinyxml2::XMLNode* pSpanElementRef = HTML_Document.NewElement( "FONT");
		pSpanElementRef->InsertEndChild( HTML_Document.NewText("T") );
		pSpanElementRef ->ToElement()->SetAttribute("color", (uxIterator % 2 == 0) ? "red" : "black" );

		pNOBRElementRef->InsertEndChild( pSpanElementRef );
	} // for
	
	
	if ( HTML_Document.SaveFile("output.html") != tinyxml2::XML_NO_ERROR )
	{
		std::cout << "Couldn't write to file" << std::endl;
	}
	
	
}