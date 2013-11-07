#ifndef __REGVIEW_H__
#define __REGVIEW_H__
#include <libtroll.h>

#if 0
struct regview_type_struct : public parse_type_common_struct
{
	regview_type_struct(void) { name = 0; next = 0; }
	~regview_type_struct(void) { if (name) free(name); }
	char		* name;
	ARM_CORE_WORD		val;
	struct regview_type_struct * next;
	void destroy_self(void)
	{
		struct regview_type_struct * p, * t;
		p = this;
		while (p)
		{
			t = p;
			p = p->next;
			delete t;
		}
	}

	void clone_to(struct regview_type_struct * p)
	{
		struct regview_type_struct * reg_node;

		parse_type_common_struct::clone_to((struct parse_type_common_struct *) p);

		reg_node = this;

		goto there;

		while (reg_node = reg_node->next)
		{
			p = p->next = new struct regview_type_struct;
there:
			p->val = reg_node->val;
			if (reg_node->name)
				p->name = strdup(reg_node->name);
			else
				p->name = 0;
		}
		p->next = 0;
	}
};
#endif

struct regview_type_struct
{
	/*! \todo	this should not be here, the whole structure must be reorganised */
	struct parse_type_common_struct	head;
	char		* name;
	ARM_CORE_WORD		val;
	struct regview_type_struct * next;
};

#endif /* __REGVIEW_H__ */
