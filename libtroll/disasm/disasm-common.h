#include <disasm-types.h>
#define YY_DECL	enum yytokentype yy_disasmlex(YYSTYPE * yylval, Troll htroll, void * yyscanner)

YY_DECL;

int yy_disasmparse (struct disasm_type_struct ** parse_head, Troll htroll, void * yyscanner);

void disasm_destroy(struct disasm_type_struct * node);
struct disasm_type_struct * disasm_clone(struct disasm_type_struct * src);

