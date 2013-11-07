
/*
int;
float;
*/
#include "stdio.h"
#include "clang.h"
#include "parser.tab.h"
#include "clang-parser-common.h"
int x = 1+2;

extern struct xlat_unit_parse_node * parse_head;
extern long int calc_expr(struct xlat_unit_parse_node * expr);

static void print_declaration(struct xlat_unit_parse_node * p)
{
struct decl_spec_node * ds;
struct declarator_node * decl;

	if (p->type != PARSE_NODE_DECLARATION)
		panic("");
	ds = &p->declaration.decl_spec->decl_spec;
	decl = &p->declaration.declarator_list->declarator;
	if (!(ds && decl))
		panic("");
	if (ds->is_typedef) printf("typedef ");
	if (ds->is_extern) printf("extern ");
	if (ds->is_static) printf("static ");
	if (ds->is_auto) printf("auto ");
	if (ds->is_register) printf("register ");

	if (ds->is_const) printf("const ");
	if (ds->is_volatile) printf("volatile ");


	if (ds->is_void) printf("void ");
	if (ds->is_char) printf("char ");
	if (ds->is_short) printf("short ");
	if (ds->is_int) printf("int ");
	if (ds->is_long) printf("long ");
	if (ds->is_float) printf("float ");
	if (ds->is_double) printf("double ");
	if (ds->is_signed) printf("signed ");
	if (ds->is_unsigned) printf("unsigned ");
	if (ds->is_struct_or_union_spec) printf("struct_or_union_spec ");
	if (ds->is_enum_spec) printf("enum_spec ");
	if (ds->is_type_name) printf("type_name ");

	if (ds->detail)
		panic("");

	if (decl->type != DECLARATOR_TYPE_IDENT)
		panic("");
	printf("%s ", decl->id);
	if (decl->initializer)
	{
 		printf(" = %i", calc_expr(decl->initializer));
	}

	printf(";\n");
}

int main()
{
#if 0
class clang_parser::parser cparser;
	cparser.parse();
#endif
	clang_parser_parse();
	while (parse_head)
	{
		if (parse_head->type == PARSE_NODE_EXPR)
		{
		long int l;
			printf("expression found\n");
			l = calc_expr(parse_head);
			printf("%li (0x%08lx)\n", l, l);
		}
		else if (parse_head->type == PARSE_NODE_DECLARATION)
			print_declaration(parse_head);
		parse_head = parse_head->next;
	}
	return 0;
	parse_head->decl_spec.is_typedef = true;
	parse_head->decl_spec.storage_class_flags = 0;
	parse_head->decl_spec.is_const = true;
	parse_head->decl_spec.type_qualifier_flags = 0;
	return 0;
}

