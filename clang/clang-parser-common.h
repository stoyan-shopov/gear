//x #define YY_DECL	clang_parser::parser::token::yytokentype clang_parserlex(clang_parser::parser::semantic_type * yylval,\
//x			clang_parser::parser::location_type * yylloc)
#define YY_DECL	int clang_parser_lex(YYSTYPE * yylval,\
			YYLTYPE * yylloc,\
			void * x)
	
YY_DECL;

#define panic(msg) do { printf("%s:%i; %s(): %s\n", __FILE__, __LINE__, __func__, msg); while(1); } while(0)
