/*
Copyright (c) 2016, Vlad Meșco
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
%include {
    #include <parser_types.h>
    #include <assert.h>
    #include <stdlib.h>
    #include <stdio.h>
    extern int tokenizer_lineno;
}

%parse_accept {
    fprintf(stderr, "Successfully parsed file.\n");
}
%parse_failure {
    fprintf(stderr, "Parse failure, last line read: %d\n", tokenizer_lineno);
}
%syntax_error {
    fprintf(stderr, "Syntax error somewhere, last line read: %d\n", tokenizer_lineno);
    exit(2);
}

%extra_argument { File* FileHead }
%token_type { char* }

%type items { List* }
%type item { IValue* }
%type value { IValue* }
%type option { Option* }
%type options { Section* }
%type section { Section* }
%type title { char* }

%start_symbol file

%nonassoc EQUALS LPAREN RPAREN LSQUARE RSQUARE.

file ::= sections.
sections ::= .
sections ::= sections section(S). {
    FileHead->Add(S);
}
section(S) ::= title(T) options(S1). {
    S1->SetName(T);
    S = S1;
}
title(T) ::= LSQUARE STRING(S) RSQUARE. {
    T = S;
}
options(S) ::= . {
    S = new Section;
}
options(S) ::= options(S1) option(O). {
    S1->options.push_back(O);
    S = S1;
}
option(O) ::= STRING(S) EQUALS value(V). {
    O = new Option(S, V);
}
value(V) ::= STRING(S). {
    V = new Scalar(S);
}
value(V) ::= LPAREN items(I) RPAREN. {
    V = I;
}
items(I) ::= . {
    I = new List();
}
items(I) ::= items(I1) STRING(S). {
    I1->values.push_back(new Scalar(S));
    I = I1;
}
items(I) ::= items(I1) option(O). {
    I1->values.push_back(O);
    I = I1;
}
