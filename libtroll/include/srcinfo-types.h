#ifndef __SRCINFO_TYPES_H__
#define __SRCINFO_TYPES_H__

#include <libtroll.h>

struct subprogram_type_struct
{
	char	* name;
	int	srcline_nr;
};

struct srcline_addr_pair_struct
{
	ARM_CORE_WORD	addr;
	int		srcline_nr;
	/* index in the cunames array for the
	 * compilation unit name in which this
	 * address resides; zero is invalid and
	 * denotes that this information is
	 * unavailable */
	int		cuname_idx;
	/* index in the subprogram_arr array for the
	 * subprogram in which this
	 * address resides; zero is invalid and
	 * denotes that this information is
	 * unavailable */
	int		subarr_idx;
	/* generic extension field */
	int	iextend;
};

struct srclist_type_struct
{
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

	struct subprogram_type_struct * subprogram_arr;
	/* the size of the 'subprogram_arr' array above */
	int subprogram_arr_len;
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

struct cu_info_struct
{
	/* the name of the compilation unit (most commonly the primary source code file name for the compilation unit) */
	const char * name;
	/* the current working directory of the compilation command that produced this comilation unit
	 *
	 * this is the 'DW_AT_comp_dir' attribute value (a null terminated string) - if any - associated
	 * with this compilation unit */
	const char * compdir;
};

struct srcinfo_type_struct
{
	struct parse_type_common_struct	head;

	struct	srclist_type_struct	* srclist;
	/* an array of compilation units */
	struct cu_info_struct * comp_units;
	/* the number of elements in the 'comp_units' array above */
	int	nr_comp_units;
};

#endif /* __SRCINFO_TYPES_H__ */

