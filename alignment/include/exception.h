#pragma once

#include <string>

/* An annotated exception class on the foundation of std::exception.
 */
template <int N>
class annotated_exception : public std::exception
{
private :
	/* automated memory deallocation !
	 */
	std::string text;

public :
	annotated_exception(const char* info)  
	{
		text = std::string( (N == 0) ? "Memory Manager:" :
							(N == 1) ? "SW Aligner:"	 :
							(N == 2) ? "Fasta Reader:"	 :
							(N == 3) ? "Null Pointer"
									 : "Unknown Source"
						  );
		text += info;
	}

	~annotated_exception() throw() 
	{} // Updated
	
	virtual const char* what() const throw() 
	{ 
		return text.c_str(); 
	}
};

/* Class for memory manager exceptions.
 */
class memory_manager_exception : public annotated_exception<0> 
{
public :
	memory_manager_exception(const char* info) : annotated_exception( info )
	{}
};

/* The aligner throws exceptions of this class.
 */
class aligner_exception : public annotated_exception<1> 
{
public :
	aligner_exception(const char* info) : annotated_exception( info )
	{}
};

/* Exceptions for the fasta reader
 */
class fasta_reader_exception : public annotated_exception<2> 
{
public :
	fasta_reader_exception(const char* info) : annotated_exception( info )
	{}
};

/* Exceptions for null pointer
 */
class NullPointerException : public annotated_exception<3> 
{
public :
	NullPointerException(const char* info) : annotated_exception( info )
	{}
};

/* A little method for null-pointer exception testing using our exception class
 */
template<typename T>
T notNull( T pointer )
{
	if ( pointer == NULL )
	{
		throw NullPointerException( "" );
	} // if
} // generic function
