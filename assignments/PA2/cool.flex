/*
 *  The scanner definition for COOL.
 */
%option noyywrap 
/*
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Don't remove anything that was here initially
 */
%{
#include <cool-parse.h>
#include <stringtab.h>
#include <utilities.h>

/* The compiler assumes these identifiers. */
#define yylval cool_yylval
#define yylex  cool_yylex

/* Max size of string constants */
#define MAX_STR_CONST 1025
#define YY_NO_UNPUT   /* keep g++ happy */

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the Cool compiler.
 */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread( (char*)buf, sizeof(char), max_size, fin)) < 0) \
		YY_FATAL_ERROR( "read() in flex scanner failed");

char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr;

extern int curr_lineno;
extern int verbose_flag;

extern YYSTYPE cool_yylval;

/*
 *  Add Your own definitions here
 */

// number of (* unmatched yet
int comment_count=0;

%}

/*
 * Define names for regular expressions here.
 */

DARROW              =>
LE				    <=
ASSIGN			    <-
LETTER			    [a-zA-Z]
DIGIT			    [0-9]
ULETTER			    [A-Z]
LLETTER			    [a-z]
WHITESPACE		    [ \n\f\r\t\v]
TYPEID			    {ULETTER}({LETTER}|{DIGIT}|_)*
OBJECTID		    {LLETTER}({LETTER}|{DIGIT}|_)*
%x COMMENT STRING SKIP
%%

 /*
  *  Nested comments
  */
--.*            {}
"(*"            {
                    BEGIN(COMMENT);
                    comment_count++;
                }         
"*)"            {
                    cool_yylval.error_msg="Unmatched *)";
                    return(ERROR);
                }         

<COMMENT>"(*"   {
                    comment_count++;
                }
<COMMENT>"*)"   {
                    comment_count--;
                    if(!comment_count)
                        BEGIN(INITIAL);
                }
<COMMENT>\n     {curr_lineno++;}
<COMMENT>.      {}
<COMMENT><<EOF>> {
                    cool_yylval.error_msg="EOF in comment";
                    BEGIN(INITIAL); 
                    return(ERROR);
                }      



{DARROW}		                {
// multi-line comments above/below deleted because 
// they somehow trigger "unrecognized rule" issue
									return (DARROW);
								}
{LE}							{return (LE);}
{ASSIGN}						{return (ASSIGN);}



(?i:class)                          {   // 10.4 keywords
// seems that -i means case insensitivity...
// too late to know
                                        return(CLASS);
                                    }
[Ee][Ll][Ss][Ee]                    {return(ELSE);}
[Ff][Ii]                            {return(FI);}
[Ii][Ff]                            {return(IF);}
[Ii][Nn]                            {return(IN);}
[Ii][Nn][Hh][Ee][Rr][Ii][Tt][Ss]    {return(INHERITS);}
[Ii][Ss][Vv][Oo][Ii][Dd]            {return(ISVOID);}
[Ll][Ee][Tt]                        {return(LET);}
[Ll][Oo][Oo][Pp]                    {return(LOOP);}
[Pp][Oo][Oo][Ll]                    {return(POOL);}
[Tt][Hh][Ee][Nn]                    {return(THEN);}
[Ww][Hh][Ii][Ll][Ee]                {return(WHILE);}
[Cc][Aa][Ss][Ee]                    {return(CASE);}
[Ee][Ss][Aa][Cc]                    {return(ESAC);}
[Oo][Ff]                            {return(OF);}
[Nn][Ee][Ww]                        {return(NEW);}
[Nn][Oo][Tt]                        {return(NOT);}

t[Rr][Uu][Ee]                       {
                                        cool_yylval.boolean=false;
                                        return(BOOL_CONST);
                                    }
f[Aa][Ll][Ss][Ee]                   {
                                        cool_yylval.boolean=false;
                                        return(BOOL_CONST);
                                    }

 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */
\"						{
							BEGIN(STRING);
							string_buf_ptr=string_buf;
						}
<STRING>\"              {
                            if(string_buf_ptr-string_buf>=MAX_STR_CONST)
                            {
                                cool_yylval.error_msg="String constant too long";
                                BEGIN(INITIAL);
                                return (ERROR);
                            }
                            *string_buf_ptr='\0';
                            cool_yylval.symbol=stringtable.add_string(string_buf);
                            BEGIN(INITIAL);
                            return (STR_CONST);
                        }
<STRING>\n              {	//ab"
						 	//c"
                            cool_yylval.error_msg="Unterminated string constant";
                            curr_lineno++;	//increment line no.
                            BEGIN(INITIAL);	
                            return (ERROR);
                        }
<STRING>\\n             {	// \n \t \b \f \c
                            //"ab\nc"
                            if(string_buf_ptr-string_buf>=MAX_STR_CONST)
                            {
                                cool_yylval.error_msg="String constant too long";
                                BEGIN(SKIP);
                                return (ERROR);
                            }
                            *string_buf_ptr++='\n';
                        }
<STRING>\\t             {	// "\t"
                            if(string_buf_ptr-string_buf>=MAX_STR_CONST)
                            {
                                cool_yylval.error_msg="String constant too long";
                                BEGIN(SKIP);
                                return (ERROR);
                            }
                            *string_buf_ptr++='\t';
                        }
<STRING>\\b             {	// "\b"
                            if(string_buf_ptr-string_buf>=MAX_STR_CONST)
                            {
                                cool_yylval.error_msg="String constant too long";
                                BEGIN(SKIP);
                                return (ERROR);
                            }
                            *string_buf_ptr++='\b';
                        }
<STRING>\\f             {	// "\f"
                            if(string_buf_ptr-string_buf>=MAX_STR_CONST)
                            {
                                cool_yylval.error_msg="String constant too long";
                                BEGIN(SKIP);
                                return (ERROR);
                            }
                            *string_buf_ptr++='\f';
                        }
<STRING>\\\0            {   // patch for "\null"
// seems that \\\0 and \\. are of the same length
// this rule has to be on top of \\. 
                            cool_yylval.error_msg="String contains escaped null character.";
							BEGIN(SKIP);
							return (ERROR);
						}
<STRING>\\.				{	// "ab\c" 
							if(string_buf_ptr-string_buf>=MAX_STR_CONST)
                            {
                                cool_yylval.error_msg="String constant too long";
                                BEGIN(SKIP);
                                return (ERROR);
                            }
                            *string_buf_ptr++=yytext[1];	
						}
<STRING>\\\n			{	// "ab\
							// c"
// this rule, however, manages to work under \\.
// no idea why
							curr_lineno++;	
							if(string_buf_ptr-string_buf>=MAX_STR_CONST)
                            {
                                cool_yylval.error_msg="String constant too long";
                                BEGIN(SKIP);
                                return (ERROR);
                            }
                            *string_buf_ptr++='\n';
						}
<STRING>\0				{
							cool_yylval.error_msg="String contains null character.";
							BEGIN(SKIP);
							return (ERROR);
						}
<STRING><<EOF>>			{
							cool_yylval.error_msg="EOF in string constant";
							BEGIN(INITIAL);
							return (ERROR);
						}
				


<STRING>.				{	// any other characters
							if(string_buf_ptr-string_buf>=MAX_STR_CONST)
                            {
                                cool_yylval.error_msg="String constant too long";
                                BEGIN(SKIP);
                                return (ERROR);
                            }
                            *string_buf_ptr++=yytext[0];
						}


<SKIP>\n				{   // ignore rest of the string because of ERROR or buf overflow
							curr_lineno++; 
							BEGIN(INITIAL); 
						}
<SKIP>\\\n			    {	//"\
							// "
							curr_lineno++;
						}
<SKIP>\"				{	//"\""
                            // end of the string
							BEGIN(INITIAL);
						}
<SKIP>\\\"			    {
							// "ab\"c"
						}
<SKIP>\\\\			    {   
							// "ab\\c"
						}
<SKIP>.				    {
							// ignore anything other characters
						}

{OBJECTID}				{   // normal text defined bellow
							cool_yylval.symbol=idtable.add_string(yytext);
							return(OBJECTID);
						}
{TYPEID}				{
							cool_yylval.symbol=idtable.add_string(yytext);
							return(TYPEID);
						}
{DIGIT}+				{
							cool_yylval.symbol=inttable.add_string(yytext);
							return (INT_CONST);
						}


\n						{curr_lineno++;}
{WHITESPACE}			{}
[-@~*+<={}();,.:/]		{	
// "-" and "/" must not be in the middle
							return yytext[0];
						}
.						{
							cool_yylval.error_msg=yytext;
							return (ERROR);
						}
%%   
