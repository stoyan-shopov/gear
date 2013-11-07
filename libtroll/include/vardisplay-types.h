#ifndef __VARDISPLAY_TYPES_H__
#define __VARDISPLAY_TYPES_H__
#include <libtroll.h>

struct var_typedef
{
	char	* name;
	char	* type_name;
	char	* rawtype;
	bool	is_deref_point;
	/* aggregate members list head */
	struct var_typedef * childlist;
	/* aggregate members sibling list */
	struct var_typedef * next;
	/* if nonzero, encodes array dimensions */
	int len_upper_bounds;
#define  MAX_ARR_BOUNDS		16
	unsigned int arr_upper_bounds[MAX_ARR_BOUNDS];

	int	nr_atoms;

};

/*! \todo	this must be optimized */
struct vardisplay_type_struct
{
	struct parse_type_common_struct	head;
	struct var_typedef * vartype;
	/* the number of elements in this array equals
	 * this->vartype->nr_atoms */
	const char ** valarray;
};

#endif /* __VARDISPLAY_TYPES_H__ */

