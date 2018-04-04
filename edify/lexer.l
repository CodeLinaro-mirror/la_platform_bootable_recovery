%{
/*
 * Copyright (C) 2009 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "expr.h"
#include "yydefs.h"
#include "parser.h"

int gLine = 1;
int gColumn = 1;
int gPos = 0;

char *string_buffer = NULL;
size_t string_buffer_size = MAX_STRING_LEN;
char* string_pos;

#define ADVANCE do {yylloc.start=gPos; yylloc.end=gPos+yyleng; \
                    gColumn+=yyleng; gPos+=yyleng;} while(0)

static void check_string_buffer(void)
{
    char* p;
    /* the last one space used to store '\0', so -1 */
    if ((string_pos - string_buffer) > (string_buffer_size-1)) {
        string_buffer_size += MAX_STRING_LEN;
        p = realloc(string_buffer, string_buffer_size);
        if(NULL == p) {
            fprintf(stderr, "edify realloc string_buffer failed!");
            exit(-1);
        }
        /* adjust the string buffer and position */
        string_pos = p + (string_pos-string_buffer);
        string_buffer = p;

        fprintf(stdout, "edify realloc string_buffer as size %d!\n", (int)string_buffer_size);
    }
}
%}

%x STR

%option noyywrap

%%


\" {
    BEGIN(STR);
    if (NULL == string_buffer) {
        string_buffer = malloc(string_buffer_size);
        if (NULL == string_buffer) {
            fprintf(stderr, "edify malloc string_buffer failed!");
            exit(-1);
        }
    }

    string_pos = string_buffer;
    yylloc.start = gPos;
    ++gColumn;
    ++gPos;
}

<STR>{
  \" {
      ++gColumn;
      ++gPos;
      BEGIN(INITIAL);
      *string_pos = '\0';
      yylval.str = strdup(string_buffer);
      yylloc.end = gPos;
      return STRING;
  }

  \\n   { gColumn += yyleng; gPos += yyleng; *string_pos++ = '\n'; check_string_buffer(); }
  \\t   { gColumn += yyleng; gPos += yyleng;  *string_pos++ = '\t'; check_string_buffer(); }
  \\\"  { gColumn += yyleng; gPos += yyleng;  *string_pos++ = '\"'; check_string_buffer(); }
  \\\\  { gColumn += yyleng; gPos += yyleng;  *string_pos++ = '\\'; check_string_buffer(); }

  \\x[0-9a-fA-F]{2} {
      gColumn += yyleng;
      gPos += yyleng;
      int val;
      sscanf(yytext+2, "%x", &val);
      *string_pos++ = val;
      check_string_buffer();
  }

  \n {
      ++gLine;
      ++gPos;
      gColumn = 1;
      *string_pos++ = yytext[0];
      check_string_buffer();
  }

  . {
      ++gColumn;
      ++gPos;
      *string_pos++ = yytext[0];
      check_string_buffer();
  }
}

if                ADVANCE; return IF;
then              ADVANCE; return THEN;
else              ADVANCE; return ELSE;
endif             ADVANCE; return ENDIF;

[a-zA-Z0-9_:/.]+ {
  ADVANCE;
  yylval.str = strdup(yytext);
  return STRING;
}

\&\&              ADVANCE; return AND;
\|\|              ADVANCE; return OR;
==                ADVANCE; return EQ;
!=                ADVANCE; return NE;

[+(),!;]          ADVANCE; return yytext[0];

[ \t]+            ADVANCE;

(#.*)?\n          gPos += yyleng; ++gLine; gColumn = 1;

.                 return BAD;
