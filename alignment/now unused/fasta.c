#include "fasta.h"

#define ks_eof(fastaReader) ((fastaReader)->bIsEOF && (fastaReader)->uxBegin >= (fastaReader)->uxEnd)
#define ks_rewind(fastaReader) ((fastaReader)->bIsEOF = (fastaReader)->uxBegin = (fastaReader)->uxEnd = 0)
                                                          
static __inline fasta_stream_t *ks_init(FILE * f)                                              
{                                                                                                                              
	fasta_stream_t *ks = (fasta_stream_t*)calloc(1, sizeof(fasta_stream_t));     
	ks->file_handler = f;                                                                                                     
	ks->buffer = (char*)malloc(__bufsize);                                                     
	return ks;                                                                                                   
}

static __inline void ks_destroy(fasta_stream_t *ks)                                    
{                                                                                                                               
	if (ks) {                                                                                                       
		free(ks->buffer);                                                                                  
		free(ks);                                                                                               
	}                                                                                                                      
}                                       

static __inline int stream_getc(fasta_stream_t *stream)                               
{                                                                                                            
	 if (stream->is_eof && stream->begin >= stream->end)
	 { 
		 return -1;
	 }

	 if (stream->begin >= stream->end) 
	 {                                                   
			 stream->begin = 0;                                                                 
			 stream->end = __read(stream->buffer, 1, __bufsize, stream->file_handler); 
			 
			 if (stream->end < __bufsize)
			 {
				 stream->is_eof = 1; 
			 }

			 if (stream->end == 0)
			 {
				 return -1;
			 }
	 }

	 return (int)stream->buffer[stream->begin++];                                      
 }


                                                                
static int ks_getuntil(fasta_stream_t *stream, int delimiter, fasta_string_t *str, int *dret) 
{                                                                                                                                       
	if (dret) { 
		*dret = 0;  
	}
    str->length = 0;                                                                                                             
    if (stream->begin >= stream->end && stream->is_eof)
	{
		return -1;
	}
    
	for (;;) 
	{                                                                                                              
		int i;   
		
	    if (stream->begin >= stream->end) 
		{                                                                     
			if (!stream->is_eof) 
			{                                                                              
				stream->begin = 0;                                                                          
				stream->end = __read(stream->buffer, 1, __bufsize, stream->file_handler);         
				if (stream->end < __bufsize) 
					stream->is_eof = 1;                        
				if (stream->end == 0) 
					break;                                                        
			} 
			else 
				break;                                                                                   
	    }                                                                                                                       
	    
		if (delimiter) {                                                                                        
			for (i = stream->begin; i < stream->end; ++i) 
			{
				if (stream->buffer[i] == delimiter) 
					break;
			}			
	    } else {                                                                                                       
			for (i = stream->begin; i < stream->end; ++i)
			{
				if (isspace(stream->buffer[i])) 
					break;
			}
	    }   
	    
		if (str->max_length - str->length < i - stream->begin + 1) 
		{                                     
			str->max_length = str->length + (i - stream->begin) + 1;                                  
			kroundup32(str->max_length);                                                                            
			str->text = (char*)realloc(str->text, str->max_length);                                
	    }                                                                                                                      
	    memcpy(str->text + str->length, stream->buffer + stream->begin, i - stream->begin); 
	    str->length = str->length + (i - stream->begin);  
		
	    stream->begin = i + 1;                                                                                      
	    if (i < stream->end) {                                                                                     
	            if (dret) *dret = stream->buffer[i];                                                  
	            break;                                                                                                
			}                                                                                                                       
	    }                                                                                                                               
	    
		/* Finish the text with null-character */
		str->text[str->length] = '\0';                                                                                 
	    return str->length;                                                                                                 
}
                                                                             
fasta_record_t *kseq_init(FILE* fd)                                                      
{                                                                                                                                      
        fasta_record_t *s = (fasta_record_t*)calloc(1, sizeof(fasta_record_t));                                 
        s->stream = ks_init(fd);                                                                                            
        return s;                                                                                                            
}                                                                                                                                       
void kseq_rewind(fasta_record_t *ks)                                                     
{                                                                                                                                      
        ks->last_char = 0;                                                                                             
        ks->stream->is_eof = ks->stream->begin = ks->stream->end = 0;                                  
}                                                                                                                                      

void kseq_destroy(fasta_record_t *ks)                                                     
{                                                                                                                                      
        if (!ks) return;                                                                                                
        free(ks->name.text); free(ks->comment.text); free(ks->sequence.text); free(ks->qual.text); 
        ks_destroy(ks->stream);                                                                                              
        free(ks);                                                                                                               
}

/* Return value:
   >=0  length of the sequence (more sequences follwoing)
   -1   end-of-file (no more sequences)
   -2   truncated quality string
   -3   memory allocation failed
 */                                                                                                           
int kseq_read(fasta_record_t *fasta)                                                                      
{                                                                                                                                       
	int c;     
	fasta_stream_t *stream = fasta->stream; 
	
	if (fasta->last_char == '\0') 
	{ /* then jump to the next header line */ 
		while ((c = stream_getc(stream)) != -1 && c != '>' && c != '@');       
		
		if (c == -1)
		{
			return -1; /* end of file */
		}
		
		fasta->last_char = c;                                                                                    
	} /* the first header char has been read */                                            
	
	fasta->comment.length = fasta->sequence.length = fasta->qual.length = 0;                                 
	
	if (ks_getuntil(stream, 0, &fasta->name, &c) < 0)
	{
		return -1;
	}

	if (c != '\n')
	{
		ks_getuntil(stream, '\n', &fasta->comment, 0);
	}
	
	while (	  (c = stream_getc(stream)) != -1 
			&& c != '>' 
			&& c != '+' 
			&& c != '@') 
	{ 
		if (isgraph(c)) { /* printable non-space character */           
			if (fasta->sequence.length + 1 >= fasta->sequence.max_length) 
			{ /* double the memory */ 
				fasta->sequence.max_length = fasta->sequence.length + 2;  

				/* rounded to next closest 2^k */ 
				kroundup32(fasta->sequence.max_length);

				fasta->sequence.text = (char*)realloc(fasta->sequence.text, fasta->sequence.max_length);
				
				if (fasta->sequence.text == NULL)
				{
					/* Memory allocation failed */
					return -3;
				}
			}                                                                                                             
			
			/* Write the received character to the sequence and inclement the length */
			fasta->sequence.text[fasta->sequence.length++] = (char)c;     
		}                                                                                                                    
	}
	
	if (c == '>' || c == '@')
		/* the first header char has been read */ 
		fasta->last_char = c; 

	/* null terminate string  -  PROBLEM LINE*/  
	fasta->sequence.text[fasta->sequence.length] = '\0';               
	
	if (c != '+')
	{
		/* FASTA
		 * length == -1 indicates here end of file
		 */
		return fasta->sequence.length;
	}

	if (fasta->qual.max_length < fasta->sequence.max_length) 
	{	
		/* allocate enough memory */    
		fasta->qual.max_length = fasta->sequence.max_length;                                                                      
		fasta->qual.text = (char*)realloc(fasta->qual.text, fasta->qual.max_length);        
	}                                                                                                                               
	
	while ((c = stream_getc(stream)) != -1 && c != '\n')
		; /* skip the rest of '+' line */ 
	
	if (c == -1)
		/* we should not stop here */  
		return -2;                 
	
	while ((c = stream_getc(stream)) != -1 && fasta->qual.length < fasta->sequence.length)
	{
		if (c >= 33 && c <= 127) 
			fasta->qual.text[fasta->qual.length++] = (unsigned char)c;
	}
	
	/* null terminate string */  
	fasta->qual.text[fasta->qual.length] = '\0';

	/* we have not come to the next header line */
	fasta->last_char = 0;     

	if (fasta->sequence.length != fasta->qual.length) 
		return -2; /* qual string is shorter than seq string */ 
	
	return fasta->sequence.length;                                                                                             
}