
%option noyywrap

D			[0-9]
H			[a-fA-F0-9]
IS			(u|U|l|L)*

%%

"assign"	{ lex_count(); return ASSIGN_KW; }
"attach"	{ lex_count(); return ATTACH_KW; }
"bsearch"	{ lex_count(); return BSEARCH_KW; }
"call"		{ lex_count(); return CALL_KW; }
"cancel"	{ lex_count(); return CANCEL_KW; }
"catch"		{ lex_count(); return CATCH_KW; }
"check"		{ lex_count(); return CHECK_KW; }
"clear"		{ lex_count(); return CLEAR_KW; }
"collector"	{ lex_count(); return COLLECTOR_KW; }
"cont"		{ lex_count(); return CONT_KW; }
"dalias"	{ lex_count(); return DALIAS_KW; }
"dbx"		{ lex_count(); return DBX_KW; }
"dbxenv"	{ lex_count(); return DBXENV_KW; }
"debug"		{ lex_count(); return DEBUG_KW; }
"delete"	{ lex_count(); return DELETE_KW; }
"detach"	{ lex_count(); return DETACH_KW; }
"dis"		{ lex_count(); return DIS_KW; }
"display"	{ lex_count(); return DISPLAY_KW; }
"down"		{ lex_count(); return DOWN_KW; }
"dump"		{ lex_count(); return DUMP_KW; }
"edit"		{ lex_count(); return EDIT_KW; }
"examine"	{ lex_count(); return EXAMINE_KW; }
"exception"	{ lex_count(); return EXCEPTION_KW; }
"exists"	{ lex_count(); return EXISTS_KW; }
"file"		{ lex_count(); return FILE_KW; }
"files"		{ lex_count(); return FILES_KW; }
"fix"		{ lex_count(); return FIX_KW; }
"fixed"		{ lex_count(); return FIXED_KW; }
"fortran_modules"	{ lex_count(); return FORTRAN_MODULES_KW; }
"frame"		{ lex_count(); return FRAME_KW; }
"func"		{ lex_count(); return FUNC_KW; }
"funcs"		{ lex_count(); return FUNCS_KW; }
"gdb"		{ lex_count(); return GDB_KW; }
"handler"	{ lex_count(); return HANDLER_KW; }
"halt"		{ lex_count(); return HALT_KW; }
"hide"		{ lex_count(); return HIDE_KW; }
"ignore"	{ lex_count(); return IGNORE_KW; }
"import"	{ lex_count(); return IMPORT_KW; }
"intercept"	{ lex_count(); return INTERCEPT_KW; }
"java"		{ lex_count(); return JAVA_KW; }
"jclasses"	{ lex_count(); return JCLASSES_KW; }
"joff"		{ lex_count(); return JOFF_KW; }
"jon"		{ lex_count(); return JON_KW; }
"jpkgs"		{ lex_count(); return JPKGS_KW; }
"kill"		{ lex_count(); return KILL_KW; }
"language"	{ lex_count(); return LANGUAGE_KW; }
"line"		{ lex_count(); return LINE_KW; }
"list"		{ lex_count(); return LIST_KW; }
"listi"		{ lex_count(); return LISTI_KW; }
"loadobject"	{ lex_count(); return LOADOBJECT_KW; }
"lwp"		{ lex_count(); return LWP_KW; }
"lwps"		{ lex_count(); return LWPS_KW; }
"mmapfile"	{ lex_count(); return MMAPFILE_KW; }
"module"	{ lex_count(); return MODULE_KW; }
"modules"	{ lex_count(); return MODULES_KW; }
"native"	{ lex_count(); return NATIVE_KW; }
"next"		{ lex_count(); return NEXT_KW; }
"nexti"		{ lex_count(); return NEXTI_KW; }
"pathmap"	{ lex_count(); return PATHMAP_KW; }
"pop"		{ lex_count(); return POP_KW; }
"print"		{ lex_count(); return PRINT_KW; }
"proc"		{ lex_count(); return PROC_KW; }
"prog"		{ lex_count(); return PROG_KW; }
"quit"		{ lex_count(); return QUIT_KW; }
"regs"		{ lex_count(); return REGS_KW; }
"replay"	{ lex_count(); return REPLAY_KW; }
"rerun"		{ lex_count(); return RERUN_KW; }
"restore"	{ lex_count(); return RESTORE_KW; }
"rprint"	{ lex_count(); return RPRINT_KW; }
"rtc showmap"	{ lex_count(); return RTC_KW; }
"rtc skippatch"	{ lex_count(); return RTC_KW; }
"rtc"		{ lex_count(); return RTC_KW; }
"run"		{ lex_count(); return RUN_KW; }
"runargs"	{ lex_count(); return RUNARGS_KW; }
"save"		{ lex_count(); return SAVE_KW; }
"scopes"	{ lex_count(); return SCOPES_KW; }
"search"	{ lex_count(); return SEARCH_KW; }
"showblock"	{ lex_count(); return SHOWBLOCK_KW; }
"showleaks"	{ lex_count(); return SHOWLEAKS_KW; }
"showmemuse"	{ lex_count(); return SHOWMEMUSE_KW; }
"source"	{ lex_count(); return SOURCE_KW; }
"status"	{ lex_count(); return STATUS_KW; }
"step"		{ lex_count(); return STEP_KW; }
"stepi"		{ lex_count(); return STEPI_KW; }
"stop"		{ lex_count(); return STOP_KW; }
"stopi"		{ lex_count(); return STOPI_KW; }
"at"		{ lex_count(); return AT_KW; }


"suppress"	{ lex_count(); return SUPPRESS_KW; }
"sync"		{ lex_count(); return SYNC_KW; }
"syncs"		{ lex_count(); return SYNCS_KW; }
"thread"	{ lex_count(); return THREAD_KW; }
"threads"	{ lex_count(); return THREADS_KW; }
"trace"		{ lex_count(); return TRACE_KW; }
"tracei"	{ lex_count(); return TRACEI_KW; }
"uncheck"	{ lex_count(); return UNCHECK_KW; }
"undisplay"	{ lex_count(); return UNDISPLAY_KW; }
"unhide"	{ lex_count(); return UNHIDE_KW; }
"unintercept"	{ lex_count(); return UNINTERCEPT_KW; }
"unsuppress"	{ lex_count(); return UNSUPPRESS_KW; }
"unwatch"	{ lex_count(); return UNWATCH_KW; }
"up"		{ lex_count(); return UP_KW; }
"use"		{ lex_count(); return USE_KW; }
"watch"		{ lex_count(); return WATCH_KW; }
"whatis"	{ lex_count(); return WHATIS_KW; }
"when"		{ lex_count(); return WHEN_KW; }
"wheni"		{ lex_count(); return WHENI_KW; }
"where"		{ lex_count(); return WHERE_KW; }
"whereami"	{ lex_count(); return WHEREAMI_KW; }
"whereis"	{ lex_count(); return WHEREIS_KW; }
"which"		{ lex_count(); return WHICH_KW; }
"whocatches"	{ lex_count(); return WHOCATCHES_KW; }



"$r0"		{ lex_count(); dbx_lval.reg_nr = 0; return HACK_REG_KW; }
"$r1"		{ lex_count(); dbx_lval.reg_nr = 1; return HACK_REG_KW; }
"$r2"		{ lex_count(); dbx_lval.reg_nr = 2; return HACK_REG_KW; }
"$r3"		{ lex_count(); dbx_lval.reg_nr = 3; return HACK_REG_KW; }
"$r4"		{ lex_count(); dbx_lval.reg_nr = 4; return HACK_REG_KW; }
"$r5"		{ lex_count(); dbx_lval.reg_nr = 5; return HACK_REG_KW; }
"$r6"		{ lex_count(); dbx_lval.reg_nr = 6; return HACK_REG_KW; }
"$r7"		{ lex_count(); dbx_lval.reg_nr = 7; return HACK_REG_KW; }
"$r8"		{ lex_count(); dbx_lval.reg_nr = 8; return HACK_REG_KW; }
"$r9"		{ lex_count(); dbx_lval.reg_nr = 9; return HACK_REG_KW; }
"$r10"		{ lex_count(); dbx_lval.reg_nr = 10; return HACK_REG_KW; }
"$r11"		{ lex_count(); dbx_lval.reg_nr = 11; return HACK_REG_KW; }
"$r12"		{ lex_count(); dbx_lval.reg_nr = 12; return HACK_REG_KW; }
"$r13"		{ lex_count(); dbx_lval.reg_nr = 13; return HACK_REG_KW; }
"$sp"		{ lex_count(); dbx_lval.reg_nr = 13; return HACK_REG_KW; }
"$r14"		{ lex_count(); dbx_lval.reg_nr = 14; return HACK_REG_KW; }
"$lr"		{ lex_count(); dbx_lval.reg_nr = 14; return HACK_REG_KW; }
"$r15"		{ lex_count(); dbx_lval.reg_nr = 15; return HACK_REG_KW; }
"$pc"		{ lex_count(); dbx_lval.reg_nr = 15; return HACK_REG_KW; }
"="		{ lex_count(); return '='; }


0[xX]{H}+{IS}?		{
		lex_count();
		/*! \todo	handle conversion errors here */
		dbx_lval.num = strtoull(yytext, 0, 0);
		return DBX_LEX_NUM_CONSTANT_TOKEN;
	}
0{D}+{IS}?		{
		lex_count();
		/*! \todo	handle conversion errors here */
		dbx_lval.num = strtoull(yytext, 0, 0);
		return DBX_LEX_NUM_CONSTANT_TOKEN;
	}
{D}+{IS}?		{ 
		lex_count();
		/*! \todo	handle conversion errors here */
		dbx_lval.num = strtoull(yytext, 0, 0);
		return DBX_LEX_NUM_CONSTANT_TOKEN; 
	}

\"([^\"]|"\\\"")*\"	{
		lex_count();
		yytext[yyleng - 1] = '\0';
		char * cstr = strdup(yytext + 1);
		if (!cstr)
			panic("");
		dbx_lval.cstr = cstr;
		return HACK_CSTR; }

[ \t\v\n\f]		{ lex_count(); }
.			{ lex_count(); return * dbxtext; }

%%

