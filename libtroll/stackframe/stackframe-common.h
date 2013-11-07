#include "stackframe-types.h"
#define YY_DECL	enum yytokentype yy_stackframelex(YYSTYPE * yylval, Troll htroll, void * yyscanner)

YY_DECL;

int yy_stackframeparse (struct stackframe_type_struct ** parse_head, Troll htroll, void * yyscanner);

void stackframe_destroy(struct stackframe_type_struct * node);
struct stackframe_type_struct * stackframe_clone(struct stackframe_type_struct * src);

