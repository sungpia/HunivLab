#include <iostream>
#include <fstream>
#include <string>
#include <algorithm>
#include <sstream>
#include "tinyxml2.h"
tinyxml2::XMLDocument HTML_Document(false);
tinyxml2::XMLNode* pRootHTMLElementRef = HTML_Document.InsertEndChild( HTML_Document.NewElement( "HTML") );
tinyxml2::XMLNode* pHEADElementRef = pRootHTMLElementRef->InsertEndChild( HTML_Document.NewElement( "HEAD") );
tinyxml2::XMLNode* pBODYElementRef = pRootHTMLElementRef->InsertEndChild( HTML_Document.NewElement( "BODY") );
tinyxml2::XMLNode* pFONTElementRef =  pBODYElementRef->InsertEndChild( HTML_Document.NewElement( "FONT") );
tinyxml2::XMLNode* pNOBRElementRef =  pFONTElementRef->InsertEndChild( HTML_Document.NewElement( "NOBR") );
std::ofstream fileToWrite("output.txt");
struct structRna
{
	std::string sName;
	std::string sProteinSequence;
	std::string sSequenceBetweenCDS;
};

std::string funcsIntToString(int a)
{
	std::string tmpString;
	std::ostringstream temp;
	temp<<a;
	return temp.str();
}
void funcEndline()
{
	tinyxml2::XMLNode* pTextElementRef = HTML_Document.NewElement("FONT");
	pTextElementRef->InsertEndChild( HTML_Document.NewElement("BR"));
	pNOBRElementRef->InsertEndChild(pTextElementRef);
}
void funcPrintDifference(structRna xRna[], const bool* boolDifferenceFlag, const int iNumberofInput);
void funcCompare(structRna xRna[], const int iNumberofInput);
void funcSimpleCompare(structRna xRna[], const int iNumberofInput);
void funcSequenceCompare(structRna xRna[], const int iNumberofInput);
void funcPrintDifferenceinSequence(structRna xRna[], const bool* boolDifferenceFlag, const int iNumberofInput);
void funcInputData();
int main(void)
{
	funcInputData();
	HTML_Document.SaveFile("output.html");
}
void funcPrintDifferenceinSequence(structRna xRna[], const bool* boolDifferenceFlag, const int iNumberofInput)
{
	//tinyxml2::XMLNode* pTextElementRef = HTML_Document.NewElement("FONT");
	funcEndline();
/*	pTextElementRef->InsertEndChild( HTML_Document.NewText("Map : Asterisks for difference sequences"));
	funcEndline();
	for(int xCurrentPos = 0 ; xCurrentPos < xRna[0].sSequenceBetweenCDS.size() ; ++xCurrentPos ){
		if(boolDifferenceFlag[xCurrentPos] == true)
		{
			tinyxml2::XMLNode* pTextElementRef = HTML_Document.NewText("*");

			pNOBRElementRef->InsertEndChild(pTextElementRef);
		}else 
		{
			tinyxml2::XMLNode* pTextElementRef = HTML_Document.NewText("&nbsp;&nbsp;");

			pNOBRElementRef->InsertEndChild(pTextElementRef);
		};
	}
	funcEndline();

	pNOBRElementRef->InsertEndChild(pTextElementRef);
	funcEndline();
*/	for(int xToPrint=0; xToPrint < iNumberofInput; ++xToPrint){
		for(int xCurrentPos=0 ; xCurrentPos < xRna[0].sSequenceBetweenCDS.size() ; ++xCurrentPos){
			tinyxml2::XMLNode* pTextElementRef = HTML_Document.NewElement("FONT");

			std::string tempChar;
			tempChar = xRna[xToPrint].sSequenceBetweenCDS[xCurrentPos];
			pTextElementRef->InsertEndChild(HTML_Document.NewText(tempChar.c_str() ));
			pTextElementRef->ToElement()->SetAttribute("color", (boolDifferenceFlag[xCurrentPos]) ? "red" : "black");

			pNOBRElementRef->InsertEndChild(pTextElementRef);
		}
		funcEndline();
	}


}
void funcSequenceCompare(structRna xRna[], const int iNumberofInput)
{
	//	std::ofstream fileToWrite("result.txt"); // Output file "result.txt"
	bool* boolDifferenceFlag = new bool [ xRna[0].sSequenceBetweenCDS.size() ]; // set Flag to print location where is different. TRUE = different

	for(int xCurrentPosition=0 ; 	xCurrentPosition < xRna[0].sSequenceBetweenCDS.size() ; ++xCurrentPosition){ // First iterator : index of string to compare
		boolDifferenceFlag[xCurrentPosition] = false;
		for(int xToCmpr = 0 ; xToCmpr < iNumberofInput; ++xToCmpr) // Second iterator : Set the string to compare
		{
			for(int xBeingCmpr = xToCmpr+1 ; xBeingCmpr < iNumberofInput ; ++xBeingCmpr) // Third iterator : Set the string that is being compared with above
			{
				// if ProteinSequence is different between two string
				if(xRna[xToCmpr].sSequenceBetweenCDS[xCurrentPosition] != xRna[xBeingCmpr].sSequenceBetweenCDS[xCurrentPosition]) 
				{
					boolDifferenceFlag[xCurrentPosition] = true;
					tinyxml2::XMLNode* pTextElementRef = HTML_Document.NewElement("FONT");
					pTextElementRef->InsertEndChild( HTML_Document.NewText(xRna[xToCmpr].sName.c_str()));
					pTextElementRef->InsertEndChild( HTML_Document.NewText(" AND "));
					pTextElementRef->InsertEndChild( HTML_Document.NewText(xRna[xBeingCmpr].sName.c_str()));
					pTextElementRef->InsertEndChild( HTML_Document.NewText(" Difference in Position "));
					pTextElementRef->InsertEndChild (HTML_Document.NewText(funcsIntToString(xCurrentPosition).c_str()));
					pTextElementRef->InsertEndChild( HTML_Document.NewText(" ("));
					std::string tempA, tempB;
					tempA = xRna[xToCmpr].sSequenceBetweenCDS[xCurrentPosition];
					tempB = xRna[xBeingCmpr].sSequenceBetweenCDS[xCurrentPosition];
					pTextElementRef->InsertEndChild( HTML_Document.NewText(tempA.c_str()));
					pTextElementRef->InsertEndChild( HTML_Document.NewText(","));
					pTextElementRef->InsertEndChild( HTML_Document.NewText(tempB.c_str()));
					pTextElementRef->InsertEndChild( HTML_Document.NewText(")"));
					funcEndline();
					pNOBRElementRef->InsertEndChild(pTextElementRef);
					//Output example "_member_A_(FAM72A),_mRNA AND _member_C_(FAM72C),_mRNA  difference in position 81(G , V)"
				} // end of if
			} // end of for
		} // end of for
	} // end of for
	funcPrintDifferenceinSequence(xRna, boolDifferenceFlag, iNumberofInput);
}
void funcInputData()
{
	/* Example of Input files :
	* numberofInputs (not existing now)
	* NameofRNA 
	* ProteinSequence
	* ... * numberofInputs
	*
	* CDSforA
	* 455 1000
	* (string of sequences~)
	* ... * numberofInputs
	*/
	pFONTElementRef->ToElement()->SetAttribute("face",  "consolas");
	structRna xRna[4]; // Assume there is only 4 input.
	int iNumberofInput ; // Fix variable for now, but made for If there are more inputs
	//Deal with Proteins
	std::ifstream fileToRead("data.txt");
	fileToRead >> iNumberofInput;
	{ 
		std::string tmpFlush;
		getline(fileToRead, tmpFlush);
	}
	for(int xIterator = 0 ; xIterator < iNumberofInput ; ++xIterator )
	{
		getline( fileToRead, xRna[xIterator].sName );
		getline( fileToRead, xRna[xIterator].sProteinSequence );
	}
	funcCompare(xRna, iNumberofInput);
		//Deal with Sequences
	for(int xIterator = 0 ; xIterator < iNumberofInput ; ++xIterator )
	{
		int tmpCDSstarts;
		int tmpCDSends;
		std::string tmpString;
		fileToRead >> tmpCDSstarts >> tmpCDSends;
		fileToRead >> tmpString;
		for(int xCurrentPos = tmpCDSstarts-1 ; xCurrentPos < tmpCDSends ; ++xCurrentPos){
			xRna[xIterator].sSequenceBetweenCDS.push_back(tmpString[xCurrentPos]);
		} // inner for
	} // outer for
	funcSequenceCompare(xRna, iNumberofInput);
}
void funcPrintDifference(structRna xRna[], const bool* boolDifferenceFlag, const int iNumberofInput)
{

	/*tinyxml2::XMLNode* pTextElementRef = HTML_Document.NewElement("FONT");
	funcEndline();
	pTextElementRef->InsertEndChild( HTML_Document.NewText("Map : Asterisks for difference sequences"));
	funcEndline();
	for(int xCurrentPos = 0 ; xCurrentPos < xRna[0].sProteinSequence.size() ; ++xCurrentPos ){
		if(boolDifferenceFlag[xCurrentPos] == true) pTextElementRef->InsertEndChild( HTML_Document.NewText("*"));
		else 
		{
				pTextElementRef->InsertEndChild(HTML_Document.NewText("&#160;"));
		};
	}
	funcEndline();

	pNOBRElementRef->InsertEndChild(pTextElementRef);
*/	funcEndline();
	for(int xToPrint=0; xToPrint < iNumberofInput; ++xToPrint){
		for(int xCurrentPos=0 ; xCurrentPos < xRna[0].sProteinSequence.size() ; ++xCurrentPos){
			tinyxml2::XMLNode* pTextElementRef = HTML_Document.NewElement("FONT");

			std::string tempChar;
			tempChar = xRna[xToPrint].sProteinSequence[xCurrentPos];
			pTextElementRef->InsertEndChild(HTML_Document.NewText(tempChar.c_str() ));
			pTextElementRef->ToElement()->SetAttribute("color", (boolDifferenceFlag[xCurrentPos]) ? "red" : "black");

			pNOBRElementRef->InsertEndChild(pTextElementRef);
		}
		funcEndline();
	}

}
void funcSimpleCompare(structRna xRna[], const int iNumberofInput)
// Reduced time complexity of funcCompare and prints only asterisks
{

	bool* boolDifferenceFlag = new bool [ xRna[0].sProteinSequence.size() ]; // set Flag to print location where is different. TRUE = different
	for(int xCurrentPosition=0 ; 	xCurrentPosition < xRna[0].sProteinSequence.size() ; ++xCurrentPosition){ // First iterator : index of string to compare
		boolDifferenceFlag[xCurrentPosition] = false;
		int xToCmpr = 0;
		for(int xBeingCmpr = xToCmpr+1 ; xBeingCmpr < iNumberofInput ; ++xBeingCmpr) // Third iterator : Set the string that is being compared with above
		{
				// if ProteinSequence is different between two string
			if(xRna[xToCmpr].sProteinSequence[xCurrentPosition] != xRna[xBeingCmpr].sProteinSequence[xCurrentPosition]) 
			{
				boolDifferenceFlag[xCurrentPosition] = true;
			} // end of if
		} // end of for		
	} // end of for
	funcPrintDifference(xRna, boolDifferenceFlag, iNumberofInput);
}
void funcCompare(structRna xRna[], const int iNumberofInput)
{
//	std::ofstream fileToWrite("result.txt"); // Output file "result.txt"
	bool* boolDifferenceFlag = new bool [ xRna[0].sProteinSequence.size() ]; // set Flag to print location where is different. TRUE = different

	for(int xCurrentPosition=0 ; 	xCurrentPosition < xRna[0].sProteinSequence.size() ; ++xCurrentPosition){ // First iterator : index of string to compare
		boolDifferenceFlag[xCurrentPosition] = false;
		for(int xToCmpr = 0 ; xToCmpr < iNumberofInput; ++xToCmpr) // Second iterator : Set the string to compare
		{
			for(int xBeingCmpr = xToCmpr+1 ; xBeingCmpr < iNumberofInput ; ++xBeingCmpr) // Third iterator : Set the string that is being compared with above
			{
				// if ProteinSequence is different between two string
				if(xRna[xToCmpr].sProteinSequence[xCurrentPosition] != xRna[xBeingCmpr].sProteinSequence[xCurrentPosition]) 
				{
					boolDifferenceFlag[xCurrentPosition] = true;
					tinyxml2::XMLNode* pTextElementRef = HTML_Document.NewElement("FONT");
					pTextElementRef->InsertEndChild( HTML_Document.NewText(xRna[xToCmpr].sName.c_str()));
					pTextElementRef->InsertEndChild( HTML_Document.NewText(" AND "));
					pTextElementRef->InsertEndChild( HTML_Document.NewText(xRna[xBeingCmpr].sName.c_str()));
					pTextElementRef->InsertEndChild( HTML_Document.NewText(" Difference in Position "));
					pTextElementRef->InsertEndChild (HTML_Document.NewText(funcsIntToString(xCurrentPosition).c_str()));
					pTextElementRef->InsertEndChild( HTML_Document.NewText(" ("));
					std::string tempA, tempB;
					tempA = xRna[xToCmpr].sProteinSequence[xCurrentPosition];
					tempB = xRna[xBeingCmpr].sProteinSequence[xCurrentPosition];
					pTextElementRef->InsertEndChild( HTML_Document.NewText(tempA.c_str()));
					pTextElementRef->InsertEndChild( HTML_Document.NewText(","));
					pTextElementRef->InsertEndChild( HTML_Document.NewText(tempB.c_str()));
					pTextElementRef->InsertEndChild( HTML_Document.NewText(")"));
					funcEndline();
					pNOBRElementRef->InsertEndChild(pTextElementRef);
					//Output example "_member_A_(FAM72A),_mRNA AND _member_C_(FAM72C),_mRNA  difference in position 81(G , V)"
				} // end of if
			} // end of for
		} // end of for
	} // end of for
	funcPrintDifference(xRna, boolDifferenceFlag, iNumberofInput);
}