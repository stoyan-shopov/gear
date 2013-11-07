
/* it is vital that the enumeration constants below fit in a single
 * char as there is a one-to-one correspondence between source code
 * text characters and token types; C_TOKEN_BASE is subtracted when
 * storing the tokens in the token buffer so that they fit in a single byte */
enum C_LEX_TOKEN_ENUM
{
	/* 0 and 0x100 are special and are used by the syntax
	 * highlighter, dont use them here */
	C_TOKEN_INVALID = 0x101,
	C_TOKEN_BASE = 0x101,
	C_TOKEN_TYPE_RELATED_KEYWORD,
	C_TOKEN_KEYWORD,
	C_TOKEN_IDENTIFIER,
	C_TOKEN_CONSTANT,
	C_TOKEN_STRING_LITERAL,
	C_TOKEN_PUNCTUATOR,
	C_TOKEN_CHAR_CONSTANT,
	/* these are not really tokens */
	C_TOKEN_COMMENT,
	C_TOKEN_PREPROCESSOR,
};

