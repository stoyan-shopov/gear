/*!
 *	\file	scope.h
 *	\brief	debug information symbol scope related routines header file
 *	\author	shopov
 *
 *	Revision summary:
 *
 *	$Log: $
 */


/*
 *
 * exported types follow
 *
 */

/*! structure holding various flags directing the course of identifier scope resolution */
struct scope_resolution_flags
{
	/*! if nonzero, lookup the global scope only
	 * 
	 * this can be the case, for example, when the debugged program is not
	 * being run, so there is no other scope to look up other
	 * than the global one */
	int	global_scope_only	: 1;
};

/*
 *
 * exported function prototypes follow
 *
 */

struct dwarf_head_struct * deprecated_scope_locate_dobj(struct gear_engine_context * ctx, const char * name, const struct scope_resolution_flags flags);

