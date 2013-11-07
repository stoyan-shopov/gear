#ifndef __EXEC_TYPES_H__
#define __EXEC_TYPES_H__
#include <libtroll.h>

#if 0
struct exec_type_struct : public parse_type_common_struct
{
	exec_type_struct(void)
	{
		memset(&flags, 0, sizeof flags);
		target_state = TARGET_CORE_STATE_INVALID;
		halt_addr = 0;
		comp_unit_name = subprogram_name = srcfile_name = 0;
		srcline = 0;
	}
	~exec_type_struct(void)
	{
		if (flags.is_comp_unit_valid)
			free(comp_unit_name);
		if (flags.is_subprogram_valid)
			free(subprogram_name);
		if (flags.is_srcfile_valid)
			free(srcfile_name);
	}
	/* flags denoting applicability of various fields below */
	struct
	{
		int	is_halt_addr_valid	: 1;
		int	is_comp_unit_valid	: 1;
		int	is_subprogram_valid	: 1;
		int	is_srcfile_valid	: 1;
		int	is_srcline_valid	: 1;
	}
	flags;
	/* this should be always present, in any type of a target
	 * execution state change packet */
	/* this is declared in the gear engine sources */
	enum TARGET_CORE_STATE_ENUM	target_state;
	ARM_CORE_WORD	halt_addr;
	char	* comp_unit_name;
	char	* subprogram_name;
	char	* srcfile_name;
	int	srcline;

	void destroy_self(void)
	{
		delete this;
	}

	void clone_to(struct exec_type_struct * p)
	{
		parse_type_common_struct::clone_to((struct parse_type_common_struct *) p);

		p->flags = flags;
		p->target_state = target_state;
		p->halt_addr = halt_addr;
		p->srcline = srcline;

		p->comp_unit_name = 0;
		p->subprogram_name = 0;
		p->srcfile_name = 0;
		if (flags.is_comp_unit_valid)
			p->comp_unit_name = strdup(comp_unit_name);
		if (flags.is_subprogram_valid)
			p->subprogram_name = strdup(subprogram_name);
		if (flags.is_srcfile_valid)
			p->srcfile_name = strdup(srcfile_name);
	}
};
#endif

struct exec_type_struct
{
	struct parse_type_common_struct	head;
	/* flags denoting applicability of various fields below */
	struct
	{
		int	is_halt_addr_valid	: 1;
		int	is_comp_unit_valid	: 1;
		int	is_subprogram_valid	: 1;
		int	is_srcfile_valid	: 1;
		int	is_srcline_valid	: 1;
	}
	flags;
	/* this should be always present, in any type of a target
	 * execution state change packet */
	/* this is declared in the gear engine sources */
	enum TARGET_CORE_STATE_ENUM	target_state;
	ARM_CORE_WORD	halt_addr;
	char	* comp_unit_name;
	char	* subprogram_name;
	char	* srcfile_name;
	int	srcline;

};


#endif /* __EXEC_TYPES_H__ */
