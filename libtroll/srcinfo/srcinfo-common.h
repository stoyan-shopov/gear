#include "srcinfo-types.h"
#define YY_DECL	enum yytokentype yy_srcinfolex(YYSTYPE * yylval, Troll htroll, void * yyscanner)

YY_DECL;

int yy_srcinfoparse (struct srcinfo_type_struct ** parse_head, Troll htroll, void * yyscanner);

void srcinfo_destroy(struct srcinfo_type_struct * node);
struct srcinfo_type_struct * srcinfo_clone(struct srcinfo_type_struct * src);

