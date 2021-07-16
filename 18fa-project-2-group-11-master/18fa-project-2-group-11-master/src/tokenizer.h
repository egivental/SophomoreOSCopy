#ifndef __TOKENIZER_H__
#define __TOKENIZER_H__


#include <stdio.h>
#include <stdlib.h>
#include <string.h>



/**
 * Control structure for a string tokenizer.  Maintains the
 * tokenizer's state.
 */
typedef struct tokenizer {
  char *str;			/* the string to parse */
  char *pos;			/* position in string */
} TOKENIZER;



/**
 * Initializes the tokenizer
 *
 * @param string the string that will be tokenized.  Should be non-NULL.
 * @return an initialized string tokenizer on success, NULL on error.
 */
TOKENIZER *init_tokenizer( char *string );



/**
 * Deallocates space used by the tokenizer.
 * @param tokenizer a non-NULL, initialized string tokenizer
 */
void free_tokenizer( TOKENIZER *tokenizer );



/**
 * Retrieves the next token in the string.  The returned token is
 * malloc'd in this function, so you should free it when done.
 *
 * @param tokenizer an initiated string tokenizer
 * @return the next token
 */
char *get_next_token( TOKENIZER *tokenizer );


#endif
