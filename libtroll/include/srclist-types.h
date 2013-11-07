#ifndef __SRCLIST_TYPES_H__
#define __SRCLIST_TYPES_H__

#include <libtroll.h>

struct subprogram_type_struct
{
	struct subprogram_type_struct	* next;
	char	* name;
	int	srcline_nr;
};

struct srcline_addr_pair_struct
{
	ARM_CORE_WORD	addr;
	int		srcline_nr;
	/* generic extension fields */
	void	* pextend;
	int	iextend;
};

struct srclist_type_struct
{
	struct parse_type_common_struct	head;
	struct srclist_type_struct * next;
	char	* srcname;
	/* this is an array, denoting which source
	 * code line numbers in the file are breakpointable;
	 * line numbers start from one (therefore, bit 0, of
	 * byte 0 is always unused and ignored); a bit set means the
	 * source code line number is breakpointable */
	unsigned int		* bkpt_bmap;
	/* the number of the elements in the array above */
	unsigned int		bmap_size;
	ARM_CORE_WORD		low_pc;
	ARM_CORE_WORD		hi_pc;

	struct subprogram_type_struct * subprogram_list;
	/*! \todo	the list below *must* be sorted by core
	 *		address value, in ascending order - check this, as
	 *		this is crucial for proper printing of disassemblies
	 *		
	 * \note	the zeroth element is reserved/unused, this is
	 *		important for some consumers */
	struct srcline_addr_pair_struct * srcaddr_pairs;
	/* this holds the number of elements in the srcaddr_pairs array above */
	int srcaddr_len;
};

#endif /* __SRCLIST_TYPES_H__ */

