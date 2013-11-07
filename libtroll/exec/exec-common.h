#include "exec-types.h"
#define YY_DECL	enum yytokentype yy_execlex(YYSTYPE * yylval, Troll htroll, void * yyscanner)

YY_DECL;

int yy_execparse (struct exec_type_struct ** parse_head, Troll htroll, void * yyscanner);

void exec_destroy(struct exec_type_struct * node);
struct exec_type_struct * exec_clone(struct exec_type_struct * src);

