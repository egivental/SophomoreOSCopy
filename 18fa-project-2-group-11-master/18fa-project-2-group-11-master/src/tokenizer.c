#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>
#include "tokenizer.h"




/**
 * Initializes the tokenizer
 *
 * @param string the string that will be tokenized.  Should be non-NULL.
 * @return an initialized string tokenizer on success, NULL on error.
 */
TOKENIZER *init_tokenizer( char *string )
{
  TOKENIZER *tokenizer;
  int len;
  assert( string != NULL );

  tokenizer = (TOKENIZER *)malloc(sizeof(TOKENIZER));
  assert( tokenizer != NULL );
  len = strlen(string) + 1;	/* don't forget \0 char */
  tokenizer->str = (char *)malloc(len);
  assert( tokenizer->str != NULL );
  memcpy( tokenizer->str, string, len );
  tokenizer->pos = tokenizer->str;
  return tokenizer;
}



/**
 * Deallocates space used by the tokenizer.
 * @param tokenizer a non-NULL, initialized string tokenizer
 */
void free_tokenizer( TOKENIZER *tokenizer )
{
  assert( tokenizer != NULL );
  free( tokenizer->str );
  free( tokenizer );
}



/**
 * Retrieves the next token in the string.  The returned token is
 * malloc'd in this function, so you should free it when done.
 *
 * @param tokenizer an initiated string tokenizer
 * @return the next token
 */
char *get_next_token( TOKENIZER *tokenizer )
{
  assert( tokenizer != NULL );
  char *startptr = tokenizer->pos;
  char *endptr;
  char *tok;

  if( *tokenizer->pos == '\0' )	/* handle end-case */
    return NULL;

  

  /* if current position is a delimiter, then return it */
  if( (*startptr == '|') || (*startptr == '&') || 
      (*startptr == '<') || (*startptr == '>') ) {
    tok = (char *)malloc(2);
    tok[0] = *startptr;
    tok[1] = '\0';
    tokenizer->pos++;
    return tok;
  }

  while( isspace(*startptr) )	/* remove initial white spaces */
    startptr++;

  if( *startptr == '\0' )
    return NULL;

  /* go until next character is a delimiter */
  endptr = startptr;
  for( ;; ) {
    if( (*(endptr+1) == '|') || (*(endptr+1) == '&') || (*(endptr+1) == '<') ||
	(*(endptr+1) == '>') || (*(endptr+1) == '\0') || (isspace(*(endptr+1))) ) {
      tok = (char *)malloc( (endptr - startptr) + 2 );
      memcpy( tok, startptr, (endptr - startptr) + 1 );
      tok[(endptr - startptr) + 1] = '\0'; /* null-terminate the string */
      tokenizer->pos = endptr + 1;
      while( isspace(*tokenizer->pos) ) /* remove trailing white space */
	tokenizer->pos++;
      return tok;
    }
    endptr++;
  }
  
  assert( 0 );			/* should never reach here */
  return NULL;			/* but satisfy compiler */
}



