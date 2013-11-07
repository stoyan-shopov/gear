/*!
 *	\file	lexblock-access.h
 *	\brief	lexical blocks access header
 *	\author	shopov
 *
 *
 *	Revision summary:
 *
 *	$Log: $
 */

/*
 *
 * exported definitions follow
 *
 */

/*
 *
 * exported types follow
 *
 */

/*! the lexical block information holding structure */
struct lexblock_data
{
	/*! the head of this structure */
	struct dwarf_head_struct	head;
	/*! head of the list of local variables for the lexical block */
	struct dobj_data *	vars;
	/*! head of the list of nested lexical blocks */
	struct lexblock_data *	lexblocks;
	/*! next node pointer for the list of nested lexical blocks */
	struct lexblock_data *	sib_ptr;
	/*! the table of address range(s) (if any) that this liexical block spans */
	struct dwarf_ranges_struct * addr_ranges;
	/*! a list of subprograms inlined in this lexical block
	 *
	 *	\todo	also support nested subprograms */
	struct subprogram_data	* inlined_subprograms;
};

/*
 *
 * exported function prototypes follow
 *
 */
struct lexblock_data * lexblock_process(struct gear_engine_context * ctx, Dwarf_Die die);

