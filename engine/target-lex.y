

%option noyywrap
%option prefix="target_"
%option reentrant
%option	extra-type="struct gear_engine_context *"
%option header-file="lex.target_.h"


D			[0-9]
H			[a-fA-F0-9]
IS			(u|U|l|L)*

%{

#define	YY_INPUT(__buf, __result, __max_size)\
	do {\
		int i, j;\
		/* sanity checks */\
		if (yyextra->core_comm->lex_state.buf_idx <\
			yyextra->core_comm->lex_state.lex_idx)\
			panic("");\
\
		j = 0;\
		/* first, see if the input buffer is empty */\
		if (yyextra->core_comm->lex_state.buf_idx ==\
			yyextra->core_comm->lex_state.lex_idx)\
		{\
			enum GEAR_ENGINE_ERR_ENUM err;\
			/* input buffer empty - refill it */\
			yyextra->core_comm->lex_state.buf_idx =\
				yyextra->core_comm->lex_state.lex_idx = 0;\
			err = must_be_only_called_from_yylex_wait_incoming_data(yyextra);\
			/* see if an error occurred */\
			if (err != GEAR_ERR_NO_ERROR)\
			{\
				panic("");\
				/* abort scanning */\
				yyextra->core_comm->lex_state.lex_err_code = err;\
				j = YY_NULL;\
				goto	abort_yyinput;\
			}\
		}\
		i = __max_size;\
		while (yyextra->core_comm->lex_state.buf_idx !=\
			yyextra->core_comm->lex_state.lex_idx\
			&& i--)\
		{\
			int c;\
			c = yyextra->core_comm->lex_state.lex_idx\
				[yyextra->core_comm->lex_state.buf];\
			if (c == '\n')\
			{\
				if (yyextra->core_comm->lex_state.lex_idx !=\
					yyextra->core_comm->lex_state.buf_idx - 1)\
				{\
					gprintf("extra input found: %c (%i)\n",\
					(yyextra->core_comm->lex_state.lex_idx + 1)\
						[yyextra->core_comm->lex_state.buf],\
					(yyextra->core_comm->lex_state.lex_idx + 1)\
						[yyextra->core_comm->lex_state.buf]\
							);\
					panic("");\
				}\
				/*\
				yyextra->core_comm->lex_state.lex_idx\
					[yyextra->core_comm->lex_state.buf] = 0;\
				*/\	
				break;\
			}\
			yyextra->core_comm->lex_state.lex_idx++;\
			__buf[j++] = c;\
		}\
	abort_yyinput:\
		__result = j;\
\
	} while (0)

%}


%%
"TARGET_CORE_STATE_DEAD"		{
						yyextra->core_comm->lex_state.yylval.state = TARGET_CORE_STATE_DEAD;
						return LEX_TOKEN_TARGET_STATE_INFO;
					}
"TARGET_CORE_STATE_HALTED"		{
						yyextra->core_comm->lex_state.yylval.state = TARGET_CORE_STATE_HALTED;
						return LEX_TOKEN_TARGET_STATE_INFO;
					}
"TARGET_CORE_STATE_RUNNING"		{
						yyextra->core_comm->lex_state.yylval.state = TARGET_CORE_STATE_RUNNING;
						return LEX_TOKEN_TARGET_STATE_INFO;
					}
"GEAR_ERR_NO_ERROR"			{
						yyextra->core_comm->lex_state.yylval.status_code = GEAR_ERR_NO_ERROR;
						return LEX_TOKEN_STATUS_CODE;
					}
"GEAR_ERR_GENERIC_ERROR"		{
						yyextra->core_comm->lex_state.yylval.status_code = GEAR_ERR_GENERIC_ERROR;
						return LEX_TOKEN_STATUS_CODE;
					}
"GEAR_ERR_TARGET_CORE_DEAD"		{
						yyextra->core_comm->lex_state.yylval.status_code = GEAR_ERR_TARGET_CORE_DEAD;
						return LEX_TOKEN_STATUS_CODE;
					}
"GEAR_ERR_TARGET_CTL_BAD_PARAMS"	{
						yyextra->core_comm->lex_state.yylval.status_code = GEAR_ERR_TARGET_CTL_BAD_PARAMS;
						return LEX_TOKEN_STATUS_CODE;
					}
"GEAR_ERR_TARGET_CTL_MEM_READ_ERROR"	{
						yyextra->core_comm->lex_state.yylval.status_code = GEAR_ERR_TARGET_CTL_MEM_READ_ERROR;
						return LEX_TOKEN_STATUS_CODE;
					}
"GEAR_ERR_RESOURCE_UNAVAILABLE_WHILE_TARGET_RUNNING"	{
						yyextra->core_comm->lex_state.yylval.status_code = GEAR_ERR_RESOURCE_UNAVAILABLE_WHILE_TARGET_RUNNING;
						return LEX_TOKEN_STATUS_CODE;
					}
"ERRMSG_HINT"	{
						return LEX_TOKEN_ERRMSG_HINT_KW;
					}
0[xX]{H}+{IS}?	{
			/*! \todo	process conversion errors here */
			yyextra->core_comm->lex_state.yylval.n = strtoull(yytext, 0, 0);
			return LEX_TOKEN_NUMBER;
		}
0{D}+{IS}?	{
			/*! \todo	process conversion errors here */
			yyextra->core_comm->lex_state.yylval.n = strtoull(yytext, 0, 0);
			return LEX_TOKEN_NUMBER;
		}
{D}+{IS}?	{ 
			/*! \todo	process conversion errors here */
			yyextra->core_comm->lex_state.yylval.n = strtoull(yytext, 0, 0);
			return LEX_TOKEN_NUMBER;
		}
\"([^\"]|"\\\"")*\"	{
		yytext[yyleng - 1] = 0;
		char * cstr = strdup(yytext + 1);
		if (!cstr)
			panic("");
		yyextra->core_comm->lex_state.yylval.cstr = cstr;
		return LEX_TOKEN_CSTR;
		}

[ \t]		{ /* ignore whitespace */ }

.		{ return * yytext; }


%%

