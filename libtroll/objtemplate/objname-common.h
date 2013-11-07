#include "objname-types.h"
#define YY_DECL	enum yytokentype yy_objprefixlex(YYSTYPE * yylval, Troll htroll, void * yyscanner)

YY_DECL;

int yy_objprefixparse (struct objname_type_struct ** parse_head, Troll htroll, void * yyscanner);

void objname_destroy(struct objname_type_struct * node);
struct objname_type_struct * objname_clone(struct objname_type_struct * src);

