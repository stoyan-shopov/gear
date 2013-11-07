#ifndef __DISASM_TYPES_H__
#define __DISASM_TYPES_H__
#include <libtroll.h>

struct disasm_text_node
{
	/* type of this node - either containing a source line, or
	 * a disassembly text line */
	enum
	{
		DIS_TYPE_INVALID = 0,
		DIS_TYPE_SRC,
		DIS_TYPE_DISASSEMBLY,
	}
	dis_type;
	union
	{
		/* this is applicable only if the type of this node is
		 * DIS_TYPE_DISASSEMBLY */
		ARM_CORE_WORD		addr;
		/* this is applicable only if the type of this node is
		 * DIS_TYPE_SRC */
		int		line_nr;
	}
	num;
	/* either a source code file name (dis_type == DIS_TYPE_SRC),
	 * or a disassembly text line (dis_type == DIS_TYPE_DISASSEMBLY) */  
	/*! \todo	the source code file name (braindamaged)
	 *		is scheduled for replacement by a source
	 *		code file number */
	char	* text;
	struct disasm_text_node * next;
};

struct disasm_type_struct
{
	struct parse_type_common_struct	head;
	ARM_CORE_WORD		pc;
	struct disasm_text_node	* disasm_list;
	ARM_CORE_WORD	start_addr;
	ARM_CORE_WORD	first_addr_past_disassembly;
	int		instruction_count;
};

#endif

