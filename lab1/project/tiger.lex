%{
#include <string.h>
#include "util.h"
#include "tokens.h"
#include "errormsg.h"

int charPos = 1;
int strMaxLen = 100; 
int strPos = 1;     // 记录字符串的pos
char *str = NULL;

int count = 0;
bool flag;

int yywrap(void)
{
 charPos = 1;
 return 1;
}


void adjust(void)
{
 EM_tokPos=charPos;
 charPos+=yyleng;
}


void str_start() {
    strPos = charPos - 1;
    str = checked_malloc(sizeof(*str) * strMaxLen);
    str[0] = '\0';
}

void str_end() {
    free(str);
    str = NULL; 
}

void str_add(char c) {
    if (strlen(str) + 1 == strMaxLen) {
        strMaxLen *= 2;
        str = realloc(str, strMaxLen);
        if (!str) {
            printf("malloc error!\n");
            exit(1);
        }
    }
    str[strlen(str)] = c;
    str[strlen(str)] = '\0';
}

%}

%x COMMENT STR VSTR

%%



<COMMENT>{
    "/*" {adjust(); count++;}
    "*/" {adjust(); count--; if(!count) BEGIN(0);}
    \n   {adjust(); EM_newline(); continue;}
    \r  
    . adjust(); 
    <<EOF>> {adjust(); EM_error(EM_tokPos,"EOF in comment"); return 0;}
}

<STR>{
    \" {adjust(); yylval.sval = strlen(str) == 0? "(null)": String(str); EM_tokPos = strPos; BEGIN(0); if(flag) {flag = FALSE; return STRING;}}
    \\f {adjust(); BEGIN(VSTR);}
    \\n  {adjust(); str_add('\n');}
    \\t {adjust(); str_add('\t');}
    \\^[a-zA-Z] adjust();
    \\[0-9]{3}  {adjust(); str_add(atoi(yytext+1));}
    \\\"    {adjust(); str_add('\"');}
    \\\\    {adjust(); str_add('\\');}
    \\[^ntf\"\\] {adjust(); flag = FALSE; EM_error(EM_tokPos, "unknown escape sequence: '\\%c'", yytext[1]);}
    \n   {adjust(); EM_newline(); str_add('\n');}
    \r
    . {adjust(); str_add(yytext[0]);}
    <<EOF>> {adjust(); EM_error(EM_tokPos,"EOF in string"); str_end(); return 0;}
}

<VSTR>{
    f\\ {adjust(); BEGIN(STR);}
    \n   {adjust(); EM_newline();}
    \r
    .    adjust(); 
    <<EOF>> {adjust(); EM_error(EM_tokPos,"EOF in string"); return 0;}
}


"/*" {adjust(); count++; BEGIN(COMMENT);}

\"   {adjust(); str_start(); flag = TRUE; BEGIN(STR);}
\n	 {adjust(); EM_newline(); continue;}
[\t ]+  {adjust(); continue;}

array   {adjust(); return ARRAY;}
if      {adjust(); return IF;}
then    {adjust(); return THEN;}
else    {adjust(); return ELSE;}
while   {adjust(); return WHILE;}
for     {adjust(); return FOR;}
to      {adjust(); return TO;}
do      {adjust(); return DO;}
let     {adjust(); return LET;}
in      {adjust(); return IN;}
end     {adjust(); return END;}
of      {adjust(); return OF;}
break   {adjust(); return BREAK;}
nil     {adjust(); return NIL;}
function {adjust(); return FUNCTION;}
var     {adjust(); return VAR;}
type    {adjust(); return TYPE;}

","  {adjust(); return COMMA;}
":"  {adjust(); return COLON;}
";"  {adjust(); return SEMICOLON;}
"("  {adjust(); return LPAREN;}
")"  {adjust(); return RPAREN;}
"["  {adjust(); return LBRACK;}
"]"  {adjust(); return RBRACK;}
"{"  {adjust(); return LBRACE;}
"}"  {adjust(); return RBRACE;}
"."  {adjust(); return DOT;}
"+"  {adjust(); return PLUS;}
"-"  {adjust(); return MINUS;}
"*"  {adjust(); return TIMES;}
"/"  {adjust(); return DIVIDE;}
"="  {adjust(); return EQ;}
"<>" {adjust(); return NEQ;}
"<"  {adjust(); return LT;}
"<=" {adjust(); return LE;}
">"  {adjust(); return GT;}
">=" {adjust(); return GE;}
"&"  {adjust(); return AND;}
"|"  {adjust(); return OR;}
":=" {adjust(); return ASSIGN;}

[a-zA-Z][a-zA-Z0-9_]*  {adjust(); yylval.sval = String(yytext); return ID;}

[0-9]+	 {adjust(); yylval.ival=atoi(yytext); return INT;}
.	 {adjust(); EM_error(EM_tokPos,"illegal token");}


