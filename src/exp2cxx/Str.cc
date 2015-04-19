/*
* NIST Utils Class Library
* clutils/Str.cc
* April 1997
* K. C. Morris
* David Sauder

* Development of this software was funded by the United States Government,
* and is not subject to copyright.
*/

#include "Str.h"
#include <sstream>
#include <string>

static int contiguous_block_number( unsigned char c )
{
    if (c == 0)               /* \0 */
        return 0;
    if (c == 32)              /* SPACE */
        return 1;
    if (48 <= c && c <= 57)   /* 0-9 */
        return 2;
    if (97 <= c && c <= 122)  /* a-z */
        return 3;
    if (65 <= c && c <= 90)   /* A-Z */
        return 4;
    if (c == 95)              /* _ */
        return 5;
    if (33 <= c && c <= 34)   /* !" */
        return 6;
    if (c == 42)              /* * */
        return 7;
    if (36 <= c && c <= 38)   /* $%& */
        return 8;
    if (c == 46)              /* . */
        return 9;
    if (c == 35)              /* # */
        return 10;
    if (43 <= c && c <= 45)   /* +,- */
        return 11;
    if (40 <= c && c <= 41)   /* () */
        return 12;
    if (c == 63)              /* ? */
        return 13;
    if (c == 47)              /* / */
        return 14;
    if (58 <= c && c <= 62)   /* :<=> */
        return 15;
    if (c == 64)              /* @ */
        return 16;
    if (c == 91)              /* [ */
        return 17;
    if (c == 93)              /* ] */
        return 18;
    if (123 <= c && c <= 125) /* {|} */
        return 19;
    if (c == 94)              /* ^ */
        return 20;
    if (c == 96)              /* ` */
        return 21;
    if (c == 126)             /* ~ */
        return 22;
    if (c == 92)              /* \ */
        return 23;
    if (c == 39)              /* ' */
        return 24;

  return 25;
}

static int entity_char_cmp( unsigned char c1, unsigned char c2 )
{
    int block1 = contiguous_block_number( c1 );
    int block2 = contiguous_block_number( c2 );

    if (block1 != block2)
        return block1 - block2;

    return c1 - c2;
}

/**************************************************************//**
 ** \fn  entity_name_cmp (const char * str1, const char * str2)
 ** \returns  Comparison result
 ** Compares two strings according to P21 entity naming rules
 ** Returns < 0  when str1 less then str2
 **         == 0 when str1 equals str2
 **         > 0  when str1 greater then str2
 ******************************************************************/
int entity_name_cmp( const char *str1, const char *str2 )
{
  const unsigned char *s1 = ( const unsigned char * ) str1;
  const unsigned char *s2 = ( const unsigned char * ) str2;
  unsigned char c1, c2;

    do {
        c1 = ( unsigned char ) *s1++;
        c2 = ( unsigned char ) *s2++;
        if( c1 == '\0' )
            return c1 - c2; /* 0 if both c1 and c2 are \0, otherwise returns -ve for c1 terminating first */
    } while( c1 == c2 );

    return entity_char_cmp( c1, c2 );
}
