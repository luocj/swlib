/**
 *  @file JRegex.h
 *
 *  @brief  Regular expression functions
 *
 *  Original code by Henry Spencer
 *  Henry Spencer's regex library                     
 *  ftp://ftp.zoo.toronto.edu/pub/bookregexp.{tar|shar}
 *
 *  Wrapped by JoStudio
 *
 *
 */
#ifndef _JREGEX_H_
#define _JREGEX_H_

#ifdef __cplusplus
extern "C" {
#endif

/*---------------------------------------------------------------------------*/
/** Perform a regex find on string
 *  @param [in] string            string to regex find
 *  @param [in] regex_expression  regex expression
 *  @param [out] start            start position of match string
 *  @param [out] length           length of match string
 *  @return     returns 1 (or TRUE) if succeed, and start,length specify the match string.
 *              you may use the start and length to determin the match string.
 *              return 0 if failed or regex expression is invalid.
 */
int RegexFind(const char *string,const char * regex_expression,int *start, int *length);
/*---------------------------------------------------------------------------*/
/** Perform a regex find on string
 *  @param [in] string             string to regex find
 *  @param [in] find_regex_expr    find regex expression
 *  @param [in] replace_regex_expr replace regex expression
 *  @param [out] start             start position of match string
 *  @param [out] length            length of replaced string
 *  @return     returns 1 (or TRUE) if succeed, and start,length specify the match string.
 *              you may use the start and length to determin the next position after replace
 *              return 0 if failed or regex expression is invalid.
 *  @note  replace string is insert to the string directly, make sure the string has enough space.
 */
int RegexReplace(char *string,const char * find_regex_expr,char * replace_regex_expr,
                 int *start,int *length);
/*---------------------------------------------------------------------------*/
/** Extract a sub-string from string at specified start position and length
 *  @param [in] string           string to regex find
 *  @param [in] start            substring start position
 *  @param [in] length           substring length
 *  @return     returns substring if succeed
 *              return NULL if failed
 *  @note  the memory of return string is alloc with malloc(), and should free() later
 *         example:  ExtractString("abcd",1,2) will retuns "bc"
 */
char * ExtractString(char *string, int start, int length);
/*---------------------------------------------------------------------------*/
/** Replace a string with replace_string , eg:  WildCharFind("abcd",1,2,"88")=="a88d"
 *  @param [in] string            string to regex find
 *  @param [in] replace_string    find expression with wild char ( * or ? )
 *  @param [out] start            replace start position
 *  @param [out] length           replace length
 *  @return     returns 1 (or TRUE) if succeed
 *              return 0 if failed
 *  @note  replace string is insert to the string directly, make sure the string has enough space.
 */
int ReplaceString(char *string, int start, int length,char * replace_string);
/*---------------------------------------------------------------------------*/
/** Convert wildchar to regex expression
 *  @param [in] wild_char_string   string contains wild char, eg. "*.txt"
 *  @return     Alloc memory and returns the regex expression
 *              returns NULL if fail
 *  @note  the memory of return string is alloc with malloc(), and should free() later
 */
char *WildCharToRegex(char *wild_char_string);
/*---------------------------------------------------------------------------*/
/** Perform a wild char find on string, eg:  WildCharFind("abcd","a*c",start,length)
 *  @param [in] string            string to regex find
 *  @param [in] find_expression   find expression with wild char ( * or ? ), eg. "*.txt"
 *  @param [out] start            start position of match string
 *  @param [out] length           length of match string
 *  @return     returns 1 (or TRUE) if succeed, and start,length specify the match string.
 *              you may use the start and length to determin the match string.
 *              return 0 if failed or regex expression is invalid.
 */
int WildCharFind(char *string, char * find_expression,int *start, int *length);
/*---------------------------------------------------------------------------*/

#ifdef __cplusplus
}
#endif

#endif // _JREGEX_H_


