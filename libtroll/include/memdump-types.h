#ifndef __MEMDUMP_TYPES_H__
#define __MEMDUMP_TYPES_H__

#include <libtroll.h>

struct memdump_type_struct
{
	struct parse_type_common_struct	head;
	ARM_CORE_WORD	start_addr;
	int		buf_len;
	uint32_t	* buf;
};


#endif /* __MEMDUMP_TYPES_H__ */

