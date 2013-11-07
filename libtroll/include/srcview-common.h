#define YY_DECL	enum srcview_lex_type yy_srcviewlex(union srcview_yylval * yylval, class comm_class ** comm_obj)

union srcview_yylval
{
	unsigned int	num;
	char		* cstr;
};

enum srcview_lex_type
{
	SRCFILE_INFO_KW		= 256,
	NR_COMP_UNITS_KW,
	NR_SRCLINES_KW,
	SRCNAME_KW,
	SRCLINE_LIST_KW,
	NUM,
	CSTR,
};

YY_DECL;

