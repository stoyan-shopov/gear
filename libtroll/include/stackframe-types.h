#ifndef __STACKFRAME_TYPES_H__
#define __STACKFRAME_TYPES_H__
#include <libtroll.h>


struct stackframe_struct
{
	/* flags denoting applicability of various fields below */
	struct
	{
		int	is_pc_addr_valid	: 1;
		int	is_comp_unit_valid	: 1;
		int	is_subprogram_valid	: 1;
		int	is_srcfile_valid	: 1;
		int	is_srcline_valid	: 1;
	}
	flags;
	ARM_CORE_WORD	pc_addr;
	char	* comp_unit_name;
	char	* subprogram_name;
	char	* srcfile_name;
	int	srcline;

	struct stackframe_struct	* older;
	struct stackframe_struct	* younger;
};

struct stackframe_type_struct
{
	struct parse_type_common_struct	head;
	enum
	{
		STACKFRAME_RECORD_TYPE_INVALID = 0,
		STACKFRAME_RECORD_TYPE_BACKTRACE,
		STACKFRAME_RECORD_TYPE_SELECTED_FRAME_NR,
	}
	record_type;

	union
	{
		struct
		{
			/* a pointer to the youngest (outermost) frame */
			struct stackframe_struct * youngest_frame;
			/* a pointer to the oldest (outermost) node in the
			 * list, used to traverse the list backwards */
			struct stackframe_struct * oldest_frame;
			/* the number of frames on the list */
			int	frame_cnt;
		};
		int		selected_frame_nr;
	};
};

#endif /* __STACKFRAME_TYPES_H__ */

