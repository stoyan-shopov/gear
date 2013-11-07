#include <bkpt-types.h>
#define YY_DECL	enum yytokentype yy_bkptlex(YYSTYPE * yylval, Troll htroll, void * yyscanner)

YY_DECL;

int yy_bkptparse (struct bkpt_type_struct ** parse_head, Troll htroll, void * yyscanner);

void bkpt_destroy(struct bkpt_type_struct * node);
struct bkpt_type_struct * bkpt_clone(struct bkpt_type_struct * src);

