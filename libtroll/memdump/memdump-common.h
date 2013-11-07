#include "memdump-types.h"
#define YY_DECL	enum yytokentype yy_memdumplex(YYSTYPE * yylval, Troll htroll, void * yyscanner)

YY_DECL;

int yy_memdumpparse (struct memdump_type_struct ** parse_head, Troll htroll, void * yyscanner);

void memdump_destroy(struct memdump_type_struct * node);
struct memdump_type_struct * memdump_clone(struct memdump_type_struct * src);

