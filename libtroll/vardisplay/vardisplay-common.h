#include "vardisplay-types.h"
#define YY_DECL	enum yytokentype yy_vardisplaylex(YYSTYPE * yylval, Troll htroll, void * yyscanner)

YY_DECL;

int yy_vardisplayparse (struct vardisplay_type_struct ** parse_head, Troll htroll, void * yyscanner);

void vardisplay_destroy(struct vardisplay_type_struct * node);
struct vardisplay_type_struct * vardisplay_clone(struct vardisplay_type_struct * src);

