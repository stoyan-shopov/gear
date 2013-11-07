#ifndef __BKPT_TYPES_H__
#define __BKPT_TYPES_H__

#include <libtroll.h>

enum BKPT_RECORD_ENUM
{
	BKPT_RECORD_INVALID = 0,
	BKPT_RECORD_BKPT_ADDED,
	BKPT_RECORD_BKPT_REMOVED,
	BKPT_RECORD_BKPT_LIST,
};
struct bkpt_type_struct
{
	struct parse_type_common_struct	head;
	enum BKPT_RECORD_ENUM bkpt_record_type;

	struct
	{
		int is_addr_valid	: 1;
		int is_srcfile_valid	: 1;
		int is_srcline_valid	: 1;
		int is_stmt_flag_valid	: 1;
		int is_empty		: 1;
	}
	/* gcc cpp doesn't seem to support anonymous members... */
	f
	;
	ARM_CORE_WORD	addr;
	int	srcline;
	bool	is_at_start_of_stmt;
	struct	bkpt_type_struct * next;
	char	*srcfile;
};


#endif /* __BKPT_TYPES_H__ */

