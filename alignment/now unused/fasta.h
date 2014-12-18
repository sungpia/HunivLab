/* The MIT License

   Copyright (c) 2008, by Heng Li <lh3@sanger.ac.uk>

   Permission is hereby granted, free of charge, to any person obtaining
   a copy of this software and associated documentation files (the
   "Software"), to deal in the Software without restriction, including
   without limitation the rights to use, copy, modify, merge, publish,
   distribute, sublicense, and/or sell copies of the Software, and to
   permit persons to whom the Software is furnished to do so, subject to
   the following conditions:

   The above copyright notice and this permission notice shall be
   included in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
   NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
   BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
   ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
   CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
   SOFTWARE.
*/

#ifndef AC_KSEQ_H
#define AC_KSEQ_H

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define __bufsize 4096
#define __read fread

#ifndef kroundup32
#define kroundup32(x) (--(x), (x)|=(x)>>1, (x)|=(x)>>2, (x)|=(x)>>4, (x)|=(x)>>8, (x)|=(x)>>16, ++(x))
#endif


typedef struct {
	int length, max_length;
	char *text;
} fasta_string_t;
                                               
typedef struct __kstream_t {                            
	char *buffer;                                                             
	int begin, end, is_eof;                                 
	FILE * file_handler;                                                               
} fasta_stream_t;
	                                           
typedef struct {                                                        
	fasta_string_t name, comment, sequence, qual;             
	int last_char;                                                 
	fasta_stream_t *stream;                                                   
} fasta_record_t;

#ifdef __cplusplus
extern "C" {
#endif

int kseq_read(fasta_record_t *fasta);     
fasta_record_t *kseq_init(FILE* fd); 

void kseq_destroy(fasta_record_t *ks);
void kseq_rewind(fasta_record_t *ks);

#ifdef __cplusplus
}
#endif

#endif