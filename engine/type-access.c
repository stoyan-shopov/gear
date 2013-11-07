/*!
 *	\file	type-access.c
 *	\brief	data type entries processing and access code
 *	\author	shopov
 *
 *	\todo	move tab formatting in the type/data printing
 *		code to a static function here (cosmetic)
 *	\todo	rename the exported functions to conform to
 *		the adopted naming convention
 *	\todo	must handle incomplete types
 *	\todo	handle function types
 *
 *	Revision summary:
 *
 *	$Log: $
 */

/*
 *
 * include section follows
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdarg.h>
#include <limits.h>

#include "dwarf-common.h"
#include "target-defs.h"
#include "gear-engine-context.h"
#include "core-access.h"
#include "dwarf-expr.h"
#include "type-access.h"
#include "dwarf-loc.h"
#include "subprogram-access.h"
#include "dobj-access.h"
#include "util.h"
#include "gprintf.h"
#include "miprintf.h"
#include "symtab.h"

#include "cxx-hacks.h"

/*
 *
 * local definitions follow
 *
 */
/*! maximum array dimensions supported when handling type/data dumping
 *
 * due to the method dwarf array types are handled, we need a stack when printing
 * them; this gives a maximum stack size - equal to the maximum number of array
 * dimensions supported; this here is arbitrary and should be enough for almost
 * everything you can think of */
#define MAX_ARRAY_DIMENSIONS	128

/*
 *
 * local data types follow
 *
 */

/*! type printer sequencer
 *
 * this is used for c type declaration printing and
 * printing ranges of aggregate type members (arrays,
 * structures/unions and combinations of these); for type
 * declaration printing, this is needed to properly print
 * pointer to arrays declarations - arrays are really the
 * only special case, because they come last, after the identifier
 * part, and pointers must be treated carefully in these
 * cases, because array declarators have a higher precedence
 * over pointer declarators, parentheses are needed to be placed
 * properly when printing such type declarations - but this cannot
 * be accomplished without some lookahead in the type
 * data lists used here - and this is the purpose this data
 * structure here serves
 *
 * as for aggregate range printing - this was considered
 * necessary and useful to relieve data traffic between
 * the gear engine and a corresponding frontend; the rationale
 * for this is as follows: when a debugger user decides to monitor
 * some (large) aggregate data structure (e.g. an array),
 * transferring all of the aggregate data type members
 * from the target to the engine and, subsequently, to the
 * frontend, could be potentially *very* slow (and it cant
 * be otherwise with dumb wiggler-type hardware, the only
 * one that i(sgs) have at my disposal right now); however,
 * moving all of the data is simply redundant in most of the
 * cases - with the typical scenario of the debugger user
 * being interested in a small amount of the members of the
 * aggregate; as the frontend knows exactly which data
 * members are visible, it can request just them, and
 * later request new ones, should the debugger user request
 * them; this has the additional virtue that the frontend
 * (and possible the gear engine) need not keep/cache large
 * amounts of data, but instead maintain a small cache
 * of the data that the debugger user decides to inspect,
 * throwing away data that hase become no longer visible to
 * the debugger user (when e.g. he/she scrolls a variable
 * display window/a memory dump, folds/unfolds an aggregate
 * data structure)
 * there is, however, more to the problem; having to support
 * a display which shows which data has been modified between
 * successive target halts implies that we should read
 * all of the monitored data each time the target halts
 * (which can obvously take *lots* of time); currently,
 * this is not handled, but a simple accuracy/speed tradeoff
 * could be this - the data that is to be monitored is not
 * actually read, but is checksummed by a simple piece of
 * code that the debugger injects in the target, then runs
 * it to checksum a portion of memory, then reads the result;
 * this is the basic idea - and some mechanism utilizing it
 * will have to be built into a subsequent release of the gear
 *
 * \note	what is called an 'atom' in the aggregate type
 *		member printing, can be viewed as a field in
 *		the aggregate that is an lvalue (however,
 *		aggregates themselves are not regarded as
 *		'atoms' here); an 'atom' is something that
 *		in order the front end to display, target
 *		memory must be accessed; this really amounts
 *		to some basic numeric type or a pointer
 *
 *	\todo	currently, for range printing, the target memory
 *		is still being read in its whole - fix this
 *	\todo	code the target-assisted checksumming
 */
struct type_dump_seq
{
	/*! output printing routine used by the internal type printer
	 *
	 * output in the type dumping routine is printed through
	 * this routine so that it can be easily redirected to different
	 * output streams; basically, this should be some 'printf'-kind
	 * routine (note that a pointer to 'printf' itself can
	 * legitimately be put here)
	 *
	 *	\todo	that does it... i(sgs) have absolutely no idea
	 *		when, how and why this has crept into the code...
	 *		this is utterly braindamaged, i suspect this has
	 *		crept in here before the ncurses user interface,
	 *		even before gprintf/miprintf were coded,
	 *		marvellous... this must be fixed!
	 */
	int (*type_printf)(const char * format, ...);
	/*! a flag denoting that we are currently printing the so-termed 'prefix' part of a declaration
	 *
	 * basically, a declaration is viewed as having three parts:
	 * a prefix part, an identifier part (can be missing), a suffix part
	 * (can be missing as well) - for example in the declaration:
	 * 'int (*arr_ptr)[3]', 'int (*' is the prefix part, 'arr_ptr' is
	 * the identifier part, and ')[3]' is the suffix part;
	 * as the one above this is really only used for proper printing
	 * of parentheses in 'pointers-to-arrays' declarations
	 *
	 * \todo	change this to bool */
	unsigned	is_prefix_printing	: 1;
	/*! indentation level used for prettyprinting
	 *
	 * this *must* be initialized to zero prior to the invocation of
	 * type_data_dump_int; prettyprinting basically causes the output to
	 * have $-s printed instead of tabs and @-s printed instead of newlines
	 * for type declarations containing structures and unions - this could be
	 * used for prettyprinting in a gear engine output consumer (e.g. a frontend);
	 * if such prettyprinting is not required in the consumer, $-s and @-s
	 * should simply be discarded from the type declaration string */
	int		indent_level;
	/* aggregate type range printing variables follow */

	/*! first atom index to print, indices start from zero */
	int		first_atom;
	/*! current atom index */
	int		cur_atom;
	/*! last atom index to print, inclusive */
	int		last_atom;
};
/*
 *
 * local data follows
 *
 */

/*
 *
 * local functions follow
 *
 */

/*!
 *	\fn	static unsigned int dwarf_get_childcnt(struct gear_engine_context * ctx, Dwarf_Die die)
 *	\brief	given a dwarf die, returns the number of its children
 *
 *	\note	it is noteworthy to say that, once upon a time,
 *		libdwarf actually did have a 'dwarf_childcnt()' routine
 *		which has been removed 'on grounds that no good
 *		use was apparent' - which is absolutely true; such
 *		information would not really be of use, and is
 *		currently used only by 'type_process' when handling
 *		enumerations, and is taken out here only for
 *		the purpose of clarity, it is not useful,
 *		and not used, anywhere else
 *
 *	\param	ctx	context to work in
 *	\param	die	the dwarf die for which to count
 *			the number of children
 *	\return	the number of children for the dwarf die given
 */
static unsigned int dwarf_get_childcnt(struct gear_engine_context * ctx, Dwarf_Die die)
{
int cnt, res;
Dwarf_Error err;
Dwarf_Die child_die, sib_die;

	res = dwarf_child(die, &child_die, &err);
	if (res == DW_DLV_NO_ENTRY)
		return 0;
	if (res != DW_DLV_OK)
		panic("");
	cnt = 0;
	do
	{
		cnt ++;
		res = dwarf_siblingof(ctx->dbg, child_die, &sib_die, &err);
		dwarf_dealloc(ctx->dbg, child_die, DW_DLA_DIE);
		child_die = sib_die;
	}
	while (res == DW_DLV_OK);
	if (res != DW_DLV_NO_ENTRY)
		panic("");
	return cnt;
}


/*
 *
 * type hash operations
 * these are just stubs for the time being...
 *
 *	\todo	actually implement this
 *
 */


/*! \todo document this */
struct type_hash_node
{
	/*! the die offset for the described type */
	Dwarf_Off		die_offset;
	/*! pointer to the data type tree built for the type */
	struct dtype_data	* dtype;
	/*! link pointer for the hash table */
	struct type_hash_node	* next;
};

/*! \todo document this */
struct type_hash
{
	int	tcache_nodes;
	struct dtype_data	* tcache;
	int	htab_size;
	struct type_hash_node	* htab[0];
};


/*!
 *	\fn	static struct dtype_data * hash_get_type(struct gear_engine_context * ctx, Dwarf_Off die_offset)
 *	\brief	retrieves a pointer to the data type described by the die at
 *		offset ::die_offset, if present
 *
 *	\todo	actually implement this
 *
 *	\param	ctx	context used to access the type hash table
 *	\param	die_offset	the offset of the type die of interest
 *	\return	pointer to the type data for the type die at offset ::die_offset, or
 *		null, if the type data is not available
 */

static struct dtype_data * hash_get_type(struct gear_engine_context * ctx, Dwarf_Off die_offset)
{
struct type_hash_node * p;

	p = ctx->types->htab[die_offset % ctx->types->htab_size];

	while (p)
	{
		if (p->die_offset == die_offset)
			break;
		p = p->next;
	}
	return p ? p->dtype : 0;
}

/*!
 * 	\fn	static void hash_put_type(struct gear_engine_context * ctx, Dwarf_Off die_offset, struct dtype_data * dtype)
 *	\brief	put a data type node in the hash table for the file being debugged
 *
 *	\todo	actually implement this
 *
 *	\param	ctx	gear engine context data structure, used to access
 *			the data types hash table
 *	\param	die_offset	the offset of the data type die of interest
 *	\param	dtype		the data node to put in the hash table
 *	\return	none
 */

static void hash_put_type(struct gear_engine_context * ctx, Dwarf_Off die_offset, struct dtype_data * dtype)
{
struct type_hash_node * p;
int idx;

	if (hash_get_type(ctx, die_offset))
	{
		Dwarf_Die die;
		Dwarf_Error err;
		Dwarf_Off offset;
		if (dwarf_offdie(ctx->dbg, die_offset, &die, &err) != DW_DLV_OK)
			panic("");
{ Dwarf_Off x; if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic(""); printf("cu relative offset: %i\n", (int) x); }
		gprintf("absolute die offset %i\n", (int) die_offset);
		if (dwarf_CU_dieoffset_given_die(die, &offset, &err) != DW_DLV_OK)
			panic("");

		gprintf("cu die offset %i\n", (int) offset);
		panic("data type already in hash table");
	}
	if (!(p = malloc(sizeof(struct type_hash_node))))
		panic("out of core");
	/* just put it in front of the list */
	p->dtype = dtype;
	p->die_offset = die_offset;
	idx = die_offset % ctx->types->htab_size;
	p->next = ctx->types->htab[idx];
	ctx->types->htab[idx] = p;

	/*! \todo	remove this, debug only */
	dtype->head.die_offset = die_offset;

}

/*!
 *	\fn	static struct dtype_data * get_dtype_node(struct gear_engine_context * ctx)
 *	\brief	allocates memory for a dtype_data structure and initializes it
 *
 *	\return	a pointer to the newly allocated dtype_data data structure
 */

static struct dtype_data * get_dtype_node(struct gear_engine_context * ctx)
{
#if 1
struct dtype_data * p;
#define TCACHE_GROW_STEP	256

	if (!ctx->types->tcache_nodes)
	{
		if (!(p = (struct dtype_data * ) calloc(TCACHE_GROW_STEP, sizeof * p)))
			panic("out of core");
		ctx->types->tcache = p;
		ctx->types->tcache_nodes = TCACHE_GROW_STEP;
	}
	p = ctx->types->tcache ++;
	ctx->types->tcache_nodes --;
	return p;
#else	
struct dtype_data * p;

	if (!(p = (struct dtype_data * ) calloc(1, sizeof * p)))
		panic("out of core");
	return p;
#endif	
}

/*!
 *	\deprecated	this function, in its current state, is deprecated,
 *			prettyprinting does not really
 *			belong to the gear debugger engine, it must
 *			be reworked and dispatched to the front end engines
 *			(type and data presentation logic)
 *
 *	\fn	static void type_data_dump_int(struct dtype_data * type, void * data, char * name, bool is_prev_deref, bool is_in_deref, struct type_dump_seq * seq)
 *	\brief	prints details on a data type or the contents of a variable of a given type
 *
 *	this one is long and ugly, but is otherwise simple and straightforward;
 *	this is the core c language type/data printer in the gear
 *
 *	\todo	move most of the parameters to the sequencer - they
 *		really belong there
 *
 *	\note	as mentioned in the comments about prettyprinting in the comments of
 *		type_dump_seq, $-s are used instead of tabs and @-s are used instead
 *		of newlines for prettyprinting purposes when printing declarations
 *		containing structures and unions
 *
 *	\todo	handle endianness here; right now only little endian machines are supported
 *	\todo	the base type code is too verbose (the 'signed' printing); when this
 *		is being fixed, the default signedness of unqualified types should be
 *		taken in account (e.g. a plain 'char' versus 'signed/unsigned char';
 *		actually, this should be the only case sign should matter - a 'plain'
 *		char on the arm with gcc is actually unsigned - on the installation i use,
 *		other unqualified integer types are signed according to the ansi c standard);
 *		*or*, alternatively, the name field in the type node should be printed
 *		verbatim and this is probably the best solution; note that there is no problem
 *		with data sign when interpreting data to print, because the sign presence/absence
 *		is given by the dwarf base type encoding; fix this appropriately when there is
 *		time
 *	\todo	function types are currently not supported
 *		and when support is being added for them, a
 *		major rework of the type string construction logic
 *		may (read, will) be necessary
 *
 *	\param	type	type data pointer to the data type to be processed
 *	\param	data	if null, only type information is printed, if non null - points to
 *			a block of data to be interpreted as a datum of the type supplied;
 *			it is caller's responsibility to ensure this is properly allocated
 *			and set up
 *	\param	name	name used when printing variables data
 *	\param	is_prev_deref	a flag, which, if non-zero, denotes
 *	 			that the data type currently processed
 *	 			is immediately preceded by a 'pointer to'
 *	 			modifier - this is needed to properly
 *	 			print parentheses in c data type declarations
 *				such as pointers to functions/arrays
 *	\param	is_in_deref	a flag, which, if non-zero, denotes
 *	 			that the data type currently processed
 *				contains a 'pointer to' modifier; this
 *				is currently used in printing non-anonymous
 *				(named) aggregate data type members
 *				which are themselves of an aggregate type,
 *				to avoid infinite recursion when printing
 *				data type details for data types which
 *				contain at some place a pointer to themselves
 *	\param	seq	a pointer to a printer sequencer data structure;
 *			this stores printing parameter flags for data/type
 *			printing; a pointer to a sequencer is passed
 *			as this is used both for argument passing and
 *			for result return; see the data structure comments
 *			for sequencer flag details
 *	\return	none; different flags are returned thru the ::seq pointer; see
 * 			the ::"struct type_dump_seq" data structure comments
 *			for flag details
 */
static void type_data_dump_int(struct dtype_data * type, void * data, char * name, bool is_prev_deref, bool is_in_deref, struct type_dump_seq * seq)
{
int i;
unsigned long long c;
/*! a helper array used for array dumping by ::type_data_dump_int */
int *index_stack;
unsigned char * cptr;
int is_name_printed;
/*! printing sequencer data structure */
struct type_dump_seq lseq;
/*! a helper for handling arrays/array subranges */
bool is_subrange;

	if (!type)
	{
		if (!data && seq->is_prefix_printing)
			seq->type_printf("void ");
		return;
	}
	/* initialize a local copy of the printing sequencer */
	lseq = *seq;
	is_name_printed = 0;
	cptr = (unsigned char *)data;

	is_subrange = false;

	switch (type->dtype_class)
	{
		case DTYPE_CLASS_BASE_TYPE:
			if (data)
			{
				/* handle range data */
				if (seq->cur_atom < seq->first_atom ||
						seq->cur_atom > seq->last_atom)
				{
					/* we are out of range */
					seq->cur_atom++;
					return;
				}
				if (name)
					seq->type_printf("%s = ", name);
			}
			else
			{
				if (!lseq.is_prefix_printing)
					return;
			}
			switch (type->base_type_encoding)
			{
				case DW_ATE_unsigned:
				case DW_ATE_unsigned_char:

					switch (type->data_size)
					{
						case 1:
							if (!data)
							{
								if (type->name)
									seq->type_printf("%s ", type->name);
								else
									seq->type_printf("unsigned char ");
							}
							else
							{
								c = *(unsigned char*)data;
								seq->type_printf("%hhx", (unsigned char)c);
							}
							break;
						case 2:
							if (!data)
							{
								if (type->name)
									seq->type_printf("%s ", type->name);
								else
									seq->type_printf("unsigned short ");
							}
							else
							{
								c = *(unsigned short*)data;
								seq->type_printf("%hx", (unsigned short)c);
							}
							break;
						case 4:
							if (!data)
							{
								if (type->name)
									seq->type_printf("%s ", type->name);
								else
									seq->type_printf("unsigned int ");
							}
							else
							{
								c = *(unsigned int*)data;
								seq->type_printf("%lx", (unsigned int)c);
							}
							break;
						case 8:
							if (!data)
							{
								if (type->name)
									seq->type_printf("%s ", type->name);
								else
									seq->type_printf("unsigned long long ");
							}
							else
							{
								c = *(unsigned long long *)data;
								seq->type_printf("%llu", c);
							}
							break;
						default:
							panic("");
					}
					break;
				case DW_ATE_signed:
				case DW_ATE_signed_char:
					switch (type->data_size)
					{
						case 1:
							if (!data)
							{
								if (type->name)
									seq->type_printf("%s ", type->name);
								else
									seq->type_printf("signed char ");
							}
							else
							{
								c = *(signed char*)data;
								seq->type_printf("%hhx", (signed char)c);
							}
							break;
						case 2:
							if (!data)
							{
								if (type->name)
									seq->type_printf("%s ", type->name);
								else
									seq->type_printf("signed short ");
							}
							else
							{
								c = *(signed short*)data;
								seq->type_printf("%hx", (signed short)c);
							}
							break;
						case 4:
							if (!data)
							{
								if (type->name)
									seq->type_printf("%s ", type->name);
								else
									seq->type_printf("signed int ");
							}
							else
							{
								c = *(signed int*)data;
								seq->type_printf("%lx", (signed int)c);
							}
							break;
						case 8:
							if (!data)
							{
								if (type->name)
									seq->type_printf("%s ", type->name);
								else
									seq->type_printf("signed long long ");
							}
							else
							{
								c = *(unsigned long long *)data;
								seq->type_printf("%lli", c);
							}
							break;
						default:
							panic("");
					}
					break;
				case DW_ATE_boolean:
					switch (type->data_size)
					{
						case 1:
							if (!data)
							{
								if (type->name)
									seq->type_printf("%s ", type->name);
								else
									seq->type_printf("bool");
							}
							else
							{
								c = *(char*)data;
								seq->type_printf("%s", c ? "true" : "false");
							}
							break;
						default:
							panic("");
							break;
					}
					break;
				case DW_ATE_float:
					switch (type->data_size)
					{
						float f;
						double d;
						long double ld;
						case sizeof(float):
							if (!data)
							{
								if (type->name)
									seq->type_printf("%s ", type->name);
								else
									seq->type_printf("float ");
							}
							else
							{
								f = *(float *)data;
								seq->type_printf("%f", (double) f);
							}
							break;
						case sizeof(double):
							if (!data)
							{
								if (type->name)
									seq->type_printf("%s ", type->name);
								else
									seq->type_printf("double ");
							}
							else
							{
								d = *(double *)data;
								seq->type_printf("%f", d);
							}
							break;
						case sizeof(long double):
							if (!data)
							{
								if (type->name)
									seq->type_printf("%s ", type->name);
								else
									seq->type_printf("long double ");
							}
							else
							{
								ld = *(signed int*)data;
								seq->type_printf("%Lf", ld);
							}
							break;
						default:
							panic("");
					}
					break;
				default:
					gprintf("base type encoding is %i\n", (int) type->base_type_encoding);
					panic("");
			}
			break;
		case DTYPE_CLASS_PTR:
		{
			if (!data)
			{
				/* initialize printing sequencer */
				if (lseq.is_prefix_printing)
				{
					type_data_dump_int(type->ptr_type, data, 0, true, true, &lseq);
					seq->type_printf("*");
				}
				else
				{
					type_data_dump_int(type->ptr_type, data, 0, true, true, &lseq);
				}
			}
			else
			{
				/* handle range data */
				if (seq->cur_atom < seq->first_atom ||
						seq->cur_atom > seq->last_atom)
				{
					/* we are out of range */
					seq->cur_atom++;
					return;
				}
				c = *(unsigned int*)data;
				if (name)
					seq->type_printf("%s = ", name);
				seq->type_printf("0x%08x", (unsigned int)c);
			}
			break;
		}
		case DTYPE_CLASS_ARRAY_SUBRANGE:
			is_subrange = true;
		case DTYPE_CLASS_ARRAY:
		{
			/* stack for array index storage */
			int * stack;
			/* used to iterate on the subranges */
			struct dtype_data *	p;
			/* number of array dimensions */
			int ndims;
			/*! an array storing the dimensions of an array that is being processed by ::type_data_dump_int */
			int *arrdims;
			struct dtype_data * arr_type;

			if (!is_subrange)
				arr_type = type->arr_data.element_type;
			else
				arr_type = type->arr_subrange_data.element_type;

			/* initialize printing sequencer */
			arrdims = (int *)malloc(MAX_ARRAY_DIMENSIONS * sizeof(*arrdims));
			index_stack= (int *)malloc(MAX_ARRAY_DIMENSIONS * sizeof(*index_stack));
			if (!(arrdims && index_stack))
				panic("out of core");

			/* count number of indices of the array */
			i = 0;
			if (!is_subrange)
				p = type->arr_data.subranges;
			else
				p = type;
			while (p)
			{
				/*! \todo	handle flexible array members */
				if (p->arr_subrange_data.upper_bound == UINT_MAX)
					panic("");
				arrdims[i++] = p->arr_subrange_data.upper_bound;
				if (i == MAX_ARRAY_DIMENSIONS)
					panic("array has too many dimensions");
				p = p->arr_subrange_data.sib_ptr;
			}
			if (!i)
				panic("bad dwarf array type description");
			ndims = i;

			if (!data)
			{
				/* just print type and dimensions */
				if (lseq.is_prefix_printing)
				{
					type_data_dump_int(arr_type, 0, 0, false, is_in_deref, &lseq);
					if (is_prev_deref)
						seq->type_printf("(");
				}
				else
				{
					if (is_prev_deref)
						seq->type_printf(")");
					for (i = 0; i < ndims; i++)
					{
						seq->type_printf("[%i]", arrdims[i] + 1);
					}
					type_data_dump_int(arr_type, 0, 0, false, is_in_deref, &lseq);
				}
			}
			else
			{
				/* prepare to loop on the array dimensions */
				stack = index_stack;
				i = 0;
				seq->type_printf("\n");

				while (1)
				{
					/* advance */
					while (i < ndims)
					{
						if (!is_name_printed)
						{
							is_name_printed = 1;
							if (name)
								seq->type_printf("%s = ", name);
						}
						seq->type_printf("{\n");
						*stack++ = arrdims[i++];
					}
					/* fix up */
					i--;
					stack--;
					(*stack)++;
					/* dump data */
					while (*stack)
					{
						(*stack)--;
						type_data_dump_int(arr_type, (void *)cptr, 0, false, is_in_deref, &lseq);
						seq->type_printf(",\n");
						cptr += arr_type->data_size;
					}
					/* retreat */
					while (!*stack)
					{
						seq->type_printf("},\n");
						if (!i)
							/* done */
							goto done_arr_dump;
						i--;
						stack--;
					}
					(*stack)--;
					/* fix up */
					stack++;
					i++;
				}
done_arr_dump:;
			}

			free(arrdims);
			free(index_stack);
			break;

		}

		case DTYPE_CLASS_CLASS:
		{
			if (data)
				panic("");
			if (lseq.is_prefix_printing)
			{
				/* print type */
				seq->type_printf("class ");
				/* if a pointer modifier has
				 * been detected, do not print
				 * type details for an aggregate;
				 * this is a simple heuristics to
				 * avoid infinite recursion in the
				 * case of aggregates containing
				 * as members pointers to themselves
				 *
				 * \todo	it may be considered
				 *		that all members
				 *		of a named aggregate
				 *		type be handled this
				 *		way (apparently, this
				 *		is what gdb does)
				 *
				 * an exception here is made for
				 * anonymous aggregate members, which
				 * are always printed */
				if (is_in_deref && type->name)
				{
					/* just print a short name and return */
					seq->type_printf("%s ", type->name);
					/* done printing type data */
					break;
				}
				if (type->name)
					seq->type_printf("%s ", type->name);
				else
					seq->type_printf("<anonymous>");

			}

			break;


		}

		case DTYPE_CLASS_STRUCT:
		case DTYPE_CLASS_UNION:
		{
			struct dtype_data * member;

			if (!data)
			{
				if (lseq.is_prefix_printing)
				{
					/* print type */
					if (type->dtype_class == DTYPE_CLASS_STRUCT)
						seq->type_printf("struct ");
					else
						seq->type_printf("union ");
					/* if a pointer modifier has
					 * been detected, do not print
				         * type details for an aggregate;
					 * this is a simple heuristics to
					 * avoid infinite recursion in the
					 * case of aggregates containing
					 * as members pointers to themselves
					 *
					 * \todo	it may be considered
					 *		that all members
					 *		of a named aggregate
					 *		type be handled this
					 *		way (apparently, this
					 *		is what gdb does)
					 *
					 * an exception here is made for
					 * anonymous aggregate members, which
					 * are always printed */
					if (is_in_deref && type->name)
					{
						/* just print a short name and return */
						seq->type_printf("%s ", type->name);
						/* done printing type data */
						break;
					}
					if (type->name)
						seq->type_printf("%s ", type->name);
					else
						seq->type_printf("<anonymous>");

					seq->type_printf("@");
					for (i = 0; i < lseq.indent_level; i++, seq->type_printf("$"));
					seq->type_printf("{@");
					/* increase indentation level for printing the members */
					lseq.indent_level++;
					for (member = type->struct_data.data_members;
						member;
							member = member->member_data.sib_ptr)
					{
						lseq.is_prefix_printing = 1;
						for (i = 0; i < lseq.indent_level; i++, seq->type_printf("$"));
						type_data_dump_int(member, 0, 0, false, is_in_deref, &lseq);
						lseq.is_prefix_printing = 0;
						type_data_dump_int(member, 0, 0, false, is_in_deref, &lseq);
						seq->type_printf("@");
					}
					/* done printing members - go back one indentation level */
					lseq.indent_level--;
					for (i = 0; i < lseq.indent_level; i++, seq->type_printf("$"));
					seq->type_printf("} ");
				}
			}
			else
			{
				/* dump data */
				if (name)
					seq->type_printf("%s = ", name);
				seq->type_printf("{ ");
				for (member = type->struct_data.data_members;
					member;
						member = member->member_data.sib_ptr)
				{
					if (member->is_node_a_static_cxx_struct_member)
						panic("");
					if (member->member_data.member_location == UINT_MAX)
						panic("");
					type_data_dump_int(member,
						(void *)(cptr + member->member_data.member_location),
							0, false, is_in_deref, &lseq);
				}
				seq->type_printf("}");
			}
			break;
		}
		case DTYPE_CLASS_MEMBER:
		{
			if (!data)
			{
				if (lseq.is_prefix_printing)
				{
					type_data_dump_int(type->member_data.member_type, 0, name, false, is_in_deref, &lseq);
				}
				else
				{
					seq->type_printf("%s", type->name);
					type_data_dump_int(type->member_data.member_type, 0, name, false, is_in_deref, &lseq);
					seq->type_printf(";");
				}
			}
			else
			{
				type_data_dump_int(type->member_data.member_type, data, type->name, false, is_in_deref, &lseq);
				seq->type_printf(",\n");
			}
			break;
		}

		case DTYPE_CLASS_SUBROUTINE:
			if (!data)
			{
				if (lseq.is_prefix_printing)
				{
					type_data_dump_int(type->subroutine_data.sub->type, 0, name, false, is_in_deref, &lseq);
					if (is_prev_deref)
						seq->type_printf("(");
				}
				else
				{
					struct dobj_data * param;
					struct type_dump_seq tseq;

					if (is_prev_deref)
						seq->type_printf(")");

					param = type->subroutine_data.sub->params;
					seq->type_printf("(");
					tseq.type_printf = seq->type_printf;
					if (!param)
						seq->type_printf("void");
					else while (param)
					{
						tseq.is_prefix_printing = 1;

						type_data_dump_int(param->type, 0, 0, false, false, &tseq);
						tseq.is_prefix_printing = 0;
						type_data_dump_int(param->type, 0, 0, false, false, &tseq);

						param = param->sib_ptr;
						if (param)
							tseq.type_printf(", ");
					}

					seq->type_printf(")");
					type_data_dump_int(type->subroutine_data.sub->type, 0, 0, false, is_in_deref, &lseq);
				}
			}
			else
			{
				panic("");
			}
			break;

		case DTYPE_CLASS_TYPE_QUALIFIER:
		{
			if (!data)
			{
				if (lseq.is_prefix_printing)
				{
					switch (type->type_qualifier_data.qualifier_id)
					{
						default:
						case TYPE_QUALIFIER_INVALID:
							seq->type_printf("<unknown type qualifier> ");
							break;
						case TYPE_QUALIFIER_CONST:
							seq->type_printf("const ");
							break;
						case TYPE_QUALIFIER_RESTRICT:
							seq->type_printf("restrict ");
							break;
						case TYPE_QUALIFIER_VOLATILE:
							seq->type_printf("volatile ");
							break;
					}
					type_data_dump_int(type->type_qualifier_data.type, 0, name, false, is_in_deref, &lseq);
				}
				else
				{
					type_data_dump_int(type->type_qualifier_data.type, 0, name, false, is_in_deref, &lseq);
				}
			}
			else
			{
				panic("");
			}
			break;
		}

		case DTYPE_CLASS_TYPEDEF:
			if (!data)
			{
				if (lseq.is_prefix_printing)
				{
					seq->type_printf("typedef %s ", type->name);
				}
			}
			else
				panic("");
			break;
		case DTYPE_CLASS_ENUMERATION:
			if (!data)
			{
				if (lseq.is_prefix_printing)
				{
					seq->type_printf("enum %s", type->name);
				}
			}
			else
				panic("");
			break;
		default:
			gprintf("unknown data type found: %i\n", type->dtype_class);
			panic("");
	}
}

/*
 *
 * exported functions follow
 *
 */


/*
 *	\fn	int dtype_access_sizeof(struct dwarf_head_struct * item)
 *	\brief	used to perform a bytes size query on an object or a data type
 *
 *	\bug	this function is incomplete
 *	\todo	finish this; also, add and document useful extensions
 *		such as obtaining the compiled size of functions
 *	\todo	maybe return zero for debug information
 *		tree nodes that dont have a well defined meaning
 *		of 'size' instead of panicking
 *
 *	\param	item	a pointer to a debug information tree node
 *			on which to perform the sizeof query; not
 *			all debug information tree nodes actually
 *			have a well defined meaning of 'size', so
 *			this function panics on such a request, but
 *			it may be preferable to just return zero in
 *			some cases; this can be null, and zero is
 *			returned in this case; basically,
 *			item is a pointer to a data object or
 *			some type in the debug information tree
 *			for the program being debugged
 *	\return	the number of bytes the entity described by item
 *		occupies; zero is returned if the passed item
 *		pointer equals null
 */
int dtype_access_sizeof(struct dwarf_head_struct * item)
{
/* helpers to avoid cluttering */
struct dtype_data * dptr;
int i;

	if (!item)
		/* either a void type or an error */
		return 0;

	dptr = (struct dtype_data *) item;

	switch (item->tag)
	{
		case DW_TAG_array_type:
			i = dtype_access_sizeof(
					(struct dwarf_head_struct *) dptr->arr_data.element_type);
			dptr = dptr->arr_data.subranges;
			goto handle_array_subranges;
		case DW_TAG_subrange_type:
			i = dtype_access_sizeof(
					(struct dwarf_head_struct *) dptr->arr_subrange_data.element_type);
handle_array_subranges:
			while (dptr)
			{
				/*! \todo	handle flexible array members */
				if (dptr->arr_subrange_data.upper_bound == UINT_MAX)
					panic("");
				i *= dptr->arr_subrange_data.upper_bound + 1;
				dptr = dptr->arr_subrange_data.sib_ptr;
			}

			return i;

		case DW_TAG_class_type:
		case DW_TAG_base_type:
		case DW_TAG_enumeration_type:
		case DW_TAG_pointer_type:
		case DW_TAG_structure_type:
		case DW_TAG_union_type:
			return dptr->data_size;

		case DW_TAG_volatile_type:
		case DW_TAG_const_type:
			return dtype_access_sizeof(dptr->type_qualifier_data.type);
		case DW_TAG_typedef:
			return dtype_access_sizeof(dptr->typedef_data.type);
		case DW_TAG_formal_parameter:
		case DW_TAG_member:
		case DW_TAG_reference_type:
		case DW_TAG_subroutine_type:
		case DW_TAG_ptr_to_member_type:
		case DW_TAG_access_declaration:
		case DW_TAG_constant:
		case DW_TAG_enumerator:
		case DW_TAG_variable:
		case DW_TAG_restrict_type:
		case DW_TAG_unspecified_type:
		case DW_TAG_mutable_type:
		case DW_TAG_shared_type:
		default:
			gprintf("%s(): unsupported dwarf tag: %s(%i)\n", __func__,
					dwarf_util_get_tag_name(item->tag),
					(int) item->tag);
			panic("");

	}
}

/*!
 *	\fn	int dtype_access_get_type_die(struct gear_engine_context * ctx, Dwarf_Die die, Dwarf_Die * type_die);
 *	\brief	for a dwarf die, retrieves its data type die, if present
 *
 *	given a dwarf die, retrieves its type die in *type_die;
 *	if the input die doesn't have a type, returns zero
 *
 *	\param	ctx	context used to access debuggee data types
 *	\param	die	the die of interest
 *	\param	type_die	the type die of the input die
 *	\return	non-zero, if the input die has a type, zero otherwise
 */
int dtype_access_get_type_die(struct gear_engine_context * ctx, Dwarf_Die die, Dwarf_Die * type_die)
{
Dwarf_Error err;
Dwarf_Bool flag;
Dwarf_Attribute attr;
Dwarf_Off offset;

	/* skip to the die containing the type of the data member */
	if (dwarf_hasattr(die, DW_AT_type, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag == 0)
		/* type data not present */
		return 0;
	if (dwarf_attr(die, DW_AT_type, &attr, &err) != DW_DLV_OK)
		panic("dwarf_attr()");
	if (dwarf_global_formref(attr, &offset, &err) != DW_DLV_OK)
	{
{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("\n");
	printf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}
		panic("dwarf_global_formref()");
	}
	if (dwarf_offdie(ctx->dbg, offset, type_die, &err) != DW_DLV_OK)
		panic("dwarf_offdie()");
	dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
	return !0;
}


/*!
 *	\fn	struct dtype_data * type_process(struct gear_engine_context * ctx, Dwarf_Die die);
 *	\brief	given a dwarf die, builds the information tree for the type of the die
 *
 *	\param	ctx	context used to access debuggee data type innformation
 *	\param	die	the die of interest
 *	\return	pointer to the root of the debug information tree built for the type of die
 */
struct dtype_data * type_process(struct gear_engine_context * ctx, Dwarf_Die die)
{
struct dtype_data * p;
Dwarf_Error err;
Dwarf_Half tagval;
Dwarf_Half form;
Dwarf_Bool flag;
Dwarf_Attribute attr;
Dwarf_Off type_die_offset;
Dwarf_Unsigned return_uvalue;
Dwarf_Signed return_svalue;
Dwarf_Die type_die;
int res;

	/*! \todo	maybe validate the die tag here */
	/* first, see if the type die tree is already in the hash table */
	if (dwarf_dieoffset(die, &type_die_offset, &err) != DW_DLV_OK)
		panic("dwarf_dieoffset()");

	if ((p = hash_get_type(ctx, type_die_offset)))
	{
		return p;
	}

	/* check for unsupported attributes */
	if (dwarf_hasattr(die, DW_AT_artificial, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		gprintf("%s(): handle DW_AT_artificial\n", __func__);
		//panic("");
	}
	if (dwarf_hasattr(die, DW_AT_accessibility, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		gprintf("%s(): add support for DW_AT_accessibility\n", __func__);
	}
	if (dwarf_hasattr(die, DW_AT_allocated, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
		panic("");
	if (dwarf_hasattr(die, DW_AT_associated, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
		panic("");
	if (dwarf_hasattr(die, DW_AT_description, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
		panic("");
	if (dwarf_hasattr(die, DW_AT_start_scope, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
		panic("");


	/* get core for the new node */
	p = get_dtype_node(ctx);

	/* get the type of the die and process it */
	if (dwarf_tag(die, &tagval, &err) != DW_DLV_OK)
		panic("dwarf_tag()");

	p->head.tag = tagval;


#if 1
	if (dwarf_hasattr(die, DW_AT_MIPS_linkage_name, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		if (dwarf_attr(die, DW_AT_MIPS_linkage_name, &attr, &err) != DW_DLV_OK)
			panic("dwarf_attr()");
		/* retrieve the flag's value */
		if (dwarf_formstring(attr, &p->linkage_name, &err) != DW_DLV_OK)
			panic("dwarf_formflag()");
		dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
	}
#endif	

	/* the dwarf standard, paragraph 2.13.2 says:
	 *
	 * 2.13.2 Declarations Completing Non-Defining Declarations
	 * Debugging information entries that represent a
	 * declaration that completes another (earlier)
	 * non-defining declaration, may have a DW_AT_specification
	 * attribute whose value is a reference to the debugging
	 * information entry representing the non-defining declaration.
	 * Debugging information entries with a DW_AT_specification attribute
	 * do not need to duplicate information provided by the debugging
	 * information entry referenced by that specification attribute.
	 *
	 * do just that here - see if the current node completes
	 * another (non-defining) declaration, and if so - copy all
	 * of the fields of the other (non-defining) declaration
	 * into this new node, and then process the attributes
	 * of this new node, thus complementing and/or overriding
	 * the fields copied from the other (non-defining) declaration */

	/* see if the data type completes another,
	 * non-defining declaration */
	if (dwarf_hasattr(die, DW_AT_specification, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		Dwarf_Off offset;
		Dwarf_Die decl_die;
		struct dtype_data * decl_node;
		/* sanity check */
		if (dwarf_hasattr(die, DW_AT_declaration, &flag, &err) != DW_DLV_OK)
			panic("dwarf_hasattr()");
		if (flag)
			panic("");

		if (dwarf_attr(die, DW_AT_specification, &attr, &err) != DW_DLV_OK)
			panic("dwarf_attr()");
		/* retrieve the associated non-defining declaration
		 * die offset */
		if (dwarf_global_formref(attr, &offset, &err) != DW_DLV_OK)
			panic("dwarf_global_formref()");
		dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
		if (dwarf_offdie(ctx->dbg, offset, &decl_die, &err) != DW_DLV_OK)
			panic("dwarf_offdie()");
		if (!(decl_node = type_process(ctx, decl_die)))
			panic("");
		dwarf_dealloc(ctx->dbg, decl_die, DW_DLA_DIE);
		/* see if the referenced die is well-formed */
		if (!decl_node->is_node_a_declaration
				|| decl_node->head.tag != tagval)
		{
{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}
		gprintf("panic() downgrade: buggy gcc debug information!!!\n");
		return p;
			panic("");
		}
		/* ok, copy the fields of the associated
		 * non-defining declaration into the current node */
		*p = *decl_node;
		/* only this field should be fixed */
		p->is_node_a_declaration = false;
	}

	/* see if this is a declaration */
	if (dwarf_hasattr(die, DW_AT_declaration, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		if (dwarf_attr(die, DW_AT_declaration, &attr, &err) != DW_DLV_OK)
			panic("dwarf_attr()");
		/* retrieve the flag's value */
		if (dwarf_formflag(attr, &flag, &err) != DW_DLV_OK)
			panic("dwarf_formflag()");
		dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
		if (flag)
			p->is_node_a_declaration = 1;
	}

	/*! \todo	should this really be here... maybe move it somehwere above... */
	if (dwarf_hasattr(die, DW_AT_abstract_origin, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		Dwarf_Off offset;
		Dwarf_Die abstract_die;
		struct dtype_data * abstract_node;

		/* retrieve the associated abstract origin
		 * die offset */
		if (dwarf_attr(die, DW_AT_abstract_origin, &attr, &err) != DW_DLV_OK)
			panic("dwarf_attr()");
		if (dwarf_global_formref(attr, &offset, &err) != DW_DLV_OK)
			panic("dwarf_global_formref()");
		dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
		if (dwarf_offdie(ctx->dbg, offset, &abstract_die, &err) != DW_DLV_OK)
			panic("dwarf_offdie()");
		if (!(abstract_node = type_process(ctx, abstract_die)))
			panic("");
		dwarf_dealloc(ctx->dbg, abstract_die, DW_DLA_DIE);
		/* copy the fields of interest */
		/*! \todo	this is incorrect/incomplete - other fields;
		 *		also, see how this should work along with
		 *		the DW_AT_specification case above, and
		 *		look in the dwarf4 standard if this is at
		 *		all possible */
		*p = *abstract_node;
		p->head.die_offset = type_die_offset;
		/*! \todo	depending on the type class, wipe any
		 *		sibling fields here... */
		//p->sib_ptr = 0;

		if (0) free(abstract_node);
	}

	/* retrieve the type name, if any */
	if (dwarf_hasattr(die, DW_AT_name, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		if (dwarf_diename(die, &p->name, &err) != DW_DLV_OK)
			panic("dwarf_diename()");
	}
	/* read data size if available */
	if (dwarf_hasattr(die, DW_AT_byte_size, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		if (dwarf_attr(die, DW_AT_byte_size, &attr, &err) != DW_DLV_OK)
			panic("dwarf_attr()");
		if (dwarf_formudata(attr, &return_uvalue, &err) != DW_DLV_OK)
			panic("dwarf_formudata()");
		p->data_size = return_uvalue;
		if (return_uvalue == 0)
		{
{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("\n");
	printf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}
printf("warning: zero-sized data object\n");
		}
		dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
	}

	if (dwarf_hasattr(die, DW_AT_bit_size, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		/* sanity checks */
		if (p->data_size == 0)
			panic("");
		/* make sure the type node being built
		 * is some aggregate member */
		/*! \todo	is it possible that DW_AT_bit_size attributes
		 *		are seen somewhere else? */
		if (tagval != DW_TAG_member)
			panic("");
		if (dwarf_attr(die, DW_AT_bit_size, &attr, &err) != DW_DLV_OK)
			panic("dwarf_attr()");
		if (dwarf_formudata(attr, &return_uvalue, &err) != DW_DLV_OK)
			panic("dwarf_formudata()");
		p->member_data.bit_size = return_uvalue;
		dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
	}
	if (dwarf_hasattr(die, DW_AT_bit_offset, &flag, &err) != DW_DLV_OK)
		panic("dwarf_hasattr()");
	if (flag)
	{
		/* sanity checks */
		if (p->data_size == 0 || p->member_data.bit_size == 0)
			panic("");
		/* make sure the type node being built
		 * is some aggregate member */
		/*! \todo	is it possible that DW_AT_bit_offset attributes
		 *		are seen somewhere else? */
		if (tagval != DW_TAG_member)
			panic("");
		if (dwarf_attr(die, DW_AT_bit_offset, &attr, &err) != DW_DLV_OK)
			panic("dwarf_attr()");
		if (dwarf_formudata(attr, &return_uvalue, &err) != DW_DLV_OK)
			panic("dwarf_formudata()");
		p->member_data.bit_offset = return_uvalue;
		dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
	}


	/* save type data into hash table */
	/*! \todo	fix comments here */
	if (tagval != DW_TAG_enumeration_type)
		hash_put_type(ctx, type_die_offset, p);

	switch (tagval)
	{
		case DW_TAG_array_type:
		{
 			/* a special case here; this is similar to the
			 * handling of the DW_TAG_pointer_type below
 			 *
			 * in a declaration of the form:
			 *
 			 * struct s { struct s * s[5]; } * s [5];
			 *
			 * it is possible to recurse back to here -
			 * so we check if after the recursive call
			 * to type_process() here, the currently
			 * processed array type has already
			 * been added to the dwarf type information
			 * being built */
			struct dtype_data **	subrange;
			Dwarf_Die child_die;

			p->dtype_class = DTYPE_CLASS_ARRAY;
			/* read the type die of the elements */
			if (!dtype_access_get_type_die(ctx, die, &type_die))
				panic("aggregate member doesnt have a type");
			p->arr_data.element_type = type_process(ctx, type_die);
			dwarf_dealloc(ctx->dbg, type_die, DW_DLA_DIE);

			/* process children of this node */
			/* read the first child */
			if (dwarf_child(die, &child_die, &err) != DW_DLV_OK)
				panic("dwarf_child()");
			subrange = &p->arr_data.subranges;
			do
			{
			Dwarf_Die sib_die;

				*subrange = type_process(ctx, child_die);
				/* duplicate array element type in the subrange child */
				(*subrange)->arr_subrange_data.element_type =
					p->arr_data.element_type;
				res = dwarf_siblingof(ctx->dbg, child_die, &sib_die, &err);
				dwarf_dealloc(ctx->dbg, child_die, DW_DLA_DIE);
				child_die = sib_die;
				subrange = &(*subrange)->arr_subrange_data.sib_ptr;
				if (res == DW_DLV_ERROR)
					panic("dwarf_siblingof()");
			}
			while (res != DW_DLV_NO_ENTRY);

type_access_stats.nr_arr_type_nodes ++;			

			break;
		}

		case DW_TAG_subrange_type:
		{
			p->dtype_class = DTYPE_CLASS_ARRAY_SUBRANGE;
			/* just read the upper bound attribute of the die */
			if (dwarf_hasattr(die, DW_AT_upper_bound, &flag, &err) != DW_DLV_OK)
				panic("dwarf_hasattr()");
			if (flag == 0)
			{
				/* the dwarf standard, paragraph 5.12, says:
				 * If the upper bound and count are missing, then the upper bound value is unknown. */
				/* upper bound not present - size is unknown - 
				 * one such case in c are flexible array
				 * members; also read the comments
				 * about the arr_subrange_data.upper_bound data
				 * field in struct dtype_data - in header file
				 * type-access.h */
				p->arr_subrange_data.upper_bound = UINT_MAX;
			}
			else
			{
				if (dwarf_attr(die, DW_AT_upper_bound, &attr, &err) != DW_DLV_OK)
					panic("dwarf_attr()");
				/* make sure the upper bound is a constant */
				if (dwarf_whatform(attr, &form, &err) != DW_DLV_OK)
					panic("dwarf_whatform()");
				if (form == DW_FORM_ref4 || form == DW_FORM_block1)
				{
					/* this is possible with c variable
					 * length arrays (see the c standard
					 * for details) - in such cases
					 * the upper array bound can be a
					 * reference to some (probably
					 * artificially generated by the
					 * compiler) variable that contains
					 * the actual array upper bound
					 * value (this is the case when the
					 * dwarf form for this attribute equals
					 * DW_FORM_ref*), or this could be some
					 * location expression (this is
					 * the case when the dwarf form for
					 * this attribute equals
					 * DW_FORM_block*) */
					gprintf("array upper bound block/reference forms not currently handled; support these\n");
					break;
				}
				if (form != DW_FORM_data1 &&
					form != DW_FORM_data2 &&
					form != DW_FORM_data4 &&
					form != DW_FORM_data8 &&
					form != DW_FORM_sdata &&
					form != DW_FORM_udata
				)
				{
{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic("");
	gprintf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(die, &x, &err) != DW_DLV_OK) panic("");
	gprintf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}
					gprintf("unsupported dwarf upper aray form: %i\n", form);
					panic("array upper bound is not a constant");
				}
				if (dwarf_formudata(attr, &return_uvalue, &err) != DW_DLV_OK)
					panic("dwarf_formudata()");
				dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
				p->arr_subrange_data.upper_bound = return_uvalue;
			}

type_access_stats.nr_arr_type_nodes ++;			

			break;
		}

		case DW_TAG_enumeration_type:
		{
			Dwarf_Die child_die;
			int i;

			p->dtype_class = DTYPE_CLASS_ENUMERATION;
			i = p->enum_data.nr_enumerators = dwarf_get_childcnt(ctx, die);
			if (i == 0)
			{
				/* an empty enumeration, or an enum declaration */
				hash_put_type(ctx, type_die_offset, p);
				break;
				if (p->is_node_a_declaration)
					gprintf("ok, enum is just a declaration...\n");
{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}
				panic("");
			}
			/* sanity check */
			if (p->data_size == 0)
				panic("");
			if (!(p->enum_data.enumerators = calloc(i, sizeof * p->enum_data.enumerators)))
				panic("");
			/* process children of this node */
			/* read the first child */
			if (dwarf_child(die, &child_die, &err) != DW_DLV_OK)
				panic("dwarf_child()");
			i = 0;
			do
			{
			Dwarf_Die sib_die;
				/* retrieve the enumerator symbollic name */
				if (dwarf_diename(child_die, &p->enum_data.enumerators[i].name, &err) != DW_DLV_OK)
					panic("dwarf_diename()");
				/* read the constant value */
				if (dwarf_hasattr(child_die, DW_AT_const_value, &flag, &err) != DW_DLV_OK)
					panic("dwarf_hasattr()");
				if (flag == 0)
					/* value not present */
					panic("enumerator has no value");
				if (dwarf_attr(child_die, DW_AT_const_value, &attr, &err) != DW_DLV_OK)
					panic("dwarf_attr()");
				/* make sure the enumerator value is a constant */
				if (dwarf_whatform(attr, &form, &err) != DW_DLV_OK)
					panic("dwarf_whatform()");
				if (form != DW_FORM_data1 &&
						form != DW_FORM_data2 &&
						form != DW_FORM_data4 &&
						form != DW_FORM_data8 &&
						form != DW_FORM_sdata &&
						form != DW_FORM_udata
				   )
					panic("enumerator value is not a constant");

				if (dwarf_formsdata(attr, &return_svalue, &err) != DW_DLV_OK)
					panic("dwarf_formsdata()");
				dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
				p->enum_data.enumerators[i].enum_const = return_svalue;

				type_access_stats.nr_enumerator_nodes ++;			

				/* move to the next enumerator */
				res = dwarf_siblingof(ctx->dbg, child_die, &sib_die, &err);
				if (res == DW_DLV_ERROR)
					panic("dwarf_siblingof()");
				dwarf_dealloc(ctx->dbg, child_die, DW_DLA_DIE);
				child_die = sib_die;

				i ++;
			}
			while (res != DW_DLV_NO_ENTRY);
			/*! \todo	comment what is being done here */
			if (1 && p->name)
			{
				struct sym_struct	* sym;
				sym = symtab_find_sym(ctx, p->name);
				while (sym)
				{
					if (sym->symclass == SYM_TYPE
							&& dtype_access_are_types_compatible(sym->dtype, p))
					{
						for (i = 0; i < p->enum_data.nr_enumerators; i++)
							dwarf_dealloc(ctx->dbg,
									p->enum_data.enumerators[i].name,
									DW_DLA_STRING);
						free(p->enum_data.enumerators);
						//free(p);
						memset(p, 0, sizeof * p);
						ctx->types->tcache --;
						ctx->types->tcache_nodes ++;

						p = sym->dtype;
						break;
					}

					sym = sym->next;
				}
				if (!sym)
				{
					symtab_store_sym(ctx, p->name, SYM_TYPE, p);
					type_access_stats.nr_enumeration_nodes ++;
				}
			}

			hash_put_type(ctx, type_die_offset, p);

			break;
		}

		case DW_TAG_pointer_type:
		{
			/* sanity check */
#if 0
// arm cc dwarf generates this
			if (p->data_size == 0)
			{
{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("\n");
	printf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}
				panic("");
			}
#endif

			p->dtype_class = DTYPE_CLASS_PTR;
			if (!dtype_access_get_type_die(ctx, die, &type_die))
				/* void pointer found */
				p->ptr_type = 0;
			else
			{
				p->ptr_type = type_process(ctx, type_die);
				dwarf_dealloc(ctx->dbg, type_die, DW_DLA_DIE);
			}

type_access_stats.nr_ptr_nodes ++;			


			break;
		}
		case DW_TAG_reference_type:
			gprintf("%s(): handle reference type properly\n", __func__);
		{
			/* sanity check */
			if (p->data_size == 0)
				panic("");

			p->dtype_class = DTYPE_CLASS_PTR;
			if (!dtype_access_get_type_die(ctx, die, &type_die))
				/* void pointer found */
				p->ptr_type = 0;
			else
			{
				p->ptr_type = type_process(ctx, type_die);
				dwarf_dealloc(ctx->dbg, type_die, DW_DLA_DIE);
			}
		}
			break;


		case DW_TAG_typedef:
		{
			p->dtype_class = DTYPE_CLASS_TYPEDEF;
			if (!dtype_access_get_type_die(ctx, die, &type_die))
			{
				/*! \todo	the typedef is actually just a
				 *		declaration - not a definition - maybe
				 *		discard such die-s altogether */
				gprintf("die offset: %i\n", (int) type_die_offset);
				gprintf("downgraded from panic(): typedef entry doesnt have a type\n");
				break;
			}
			p->typedef_data.type = type_process(ctx, type_die);
			dwarf_dealloc(ctx->dbg, type_die, DW_DLA_DIE);

type_access_stats.nr_typedef_nodes ++;			

			break;
		}

		case DW_TAG_string_type:
			panic("handle string type");
			break;


		case DW_TAG_class_type:
		{
			Dwarf_Die child_die;
			struct dtype_data		** data_members;
			struct dtype_data		** base_classes;
			struct subprogram_data		** member_functions;
			Dwarf_Half	child_tag;

			p->dtype_class = DTYPE_CLASS_CLASS;

			/* sanity check */
			if (p->data_size == 0 && !p->is_node_a_declaration)
				panic("");

			/* if this is just a declaration, moan we cant handle it right now */
			if (p->is_node_a_declaration)
			{
				gprintf("panic downgrade: handle class declarations");
				break;
				panic("");
			}
			/* ok, read the children of the class (the die children) and put them on the member lists */
			/* read the first child */
			if (dwarf_child(die, &child_die, &err) != DW_DLV_OK)
			{
{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("\n");
	printf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}
				gprintf("panic downgrade\n");
				break;
				panic("dwarf_child()");
			}

			data_members = &p->struct_data.data_members;
			member_functions = &p->struct_data.member_functions;
			base_classes = &p->struct_data.inherited_bases;

			do
			{
			if (dwarf_tag(child_die, &child_tag, &err) != DW_DLV_OK)
				panic("dwarf_tag()");

				switch (child_tag)
				{
					default:
{ Dwarf_Off x; if (dwarf_die_CU_offset(child_die, &x, &err) != DW_DLV_OK) panic(""); gprintf("cu relative offset: %i\n", (int) x); }
						//panic("");
						gprintf("skipping c++ panic");
						break;
					case DW_TAG_inheritance:
{
	Dwarf_Die base_class_die;
/*! \todo	HACK!!! HACK!!! HACK!!! HACK!!! HACK!!! HACK!!! HACK!!! HACK!!!
 *		fix this ugly hack */

					if (!dtype_access_get_type_die(ctx, child_die, &base_class_die))
							panic("");
						*base_classes = get_dtype_node(ctx);
						(*base_classes)->inheritance_data.class_type = type_process(ctx, base_class_die);






			if (dwarf_hasattr(child_die, DW_AT_data_member_location, &flag, &err) != DW_DLV_OK)
				panic("dwarf_hasattr()");
			if (flag == 0)
				/* should not happen */
				panic("");
			else
			{
				/* data member location information
				 * present - process it */
				/* ok, compute member offset from the type base */
				if (dwarf_attr(child_die, DW_AT_data_member_location, &attr, &err) != DW_DLV_OK)
					panic("dwarf_attr()");

				if (dwarf_whatform(attr, &form, &err) != DW_DLV_OK)
					panic("dwarf_whatform");
				switch (form)
				{
					case DW_FORM_block1:
					{
						/*! \todo	maybe use support code from
						 *		dwarf-expr.c/dwarf-loc.c to compute
						 *		the base class offsets */
						struct dwarf_location	loc;

						if (dwarf_loclist_n(attr, &loc.llbuf, &loc.listlen, &err) != DW_DLV_OK)
							panic("dwarf_loclist_n()");
						if (loc.listlen != 1)
							panic("weird case of an aggregate data type member location description");
#if 0
						if (dwarf_loc_eval_loc_from_list(&p->member_data.member_location, &is_in_reg, &is_fbreg_needed,
							&loc, 0, 0, 0, 0))
							panic("could not evaluate aggregate data type member location");
						/* basic sanity check */
						if (is_in_reg || is_fbreg_needed)
							panic("bad aggregate data type member location");
						break;
#endif
						/* shortcut this */
						if (loc.llbuf[0]->ld_cents != 1)
							panic("");
						if (loc.llbuf[0]->ld_s[0].lr_atom != DW_OP_plus_uconst)
							panic("");
						(*base_classes)->inheritance_data.location_in_derived_class = loc.llbuf[0]->ld_s[0].lr_number;
						/* these are not needed any more */	 
						dwarf_dealloc(ctx->dbg, loc.llbuf[0]->ld_s, DW_DLA_LOC_BLOCK);
						dwarf_dealloc(ctx->dbg, loc.llbuf[0], DW_DLA_LOCDESC);
						dwarf_dealloc(ctx->dbg, loc.llbuf, DW_DLA_LIST);
						break;
					}

					default:
						gprintf("unknown dwarf form for dwarf member data location attribute: 0x%hhx\n", form);
						panic("");
				}
			}




						base_classes = &(*base_classes)->inheritance_data.sib_ptr;






						break;
}
					case DW_TAG_subprogram:
						*member_functions = subprogram_process(ctx, child_die);
						member_functions = &(*member_functions)->sib_ptr;
						break;
					case DW_TAG_member:
						*data_members = type_process(ctx, child_die);
						/* make sure the data member location
						 * description is valid - also read the comments
						 * about the member_data.member_location data
						 * field in struct dtype_data - in header file
						 * type-access.h */
						if ((*data_members)->member_data.member_location == UINT_MAX
								&& !(*data_members)->is_node_a_declaration)
						{
{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(child_die, &x, &err) != DW_DLV_OK) panic("");
	printf("\n");
	printf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(child_die, &x, &err) != DW_DLV_OK) panic("");
	printf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}
panic("");
						}
						if ((*data_members)->linkage_name)
							cxx_hacks_add_linkage_node(ctx, *data_members);
						data_members = &(*data_members)->member_data.sib_ptr;
						break;
				}
				res = dwarf_siblingof(ctx->dbg, child_die, &child_die, &err);
				if (res == DW_DLV_ERROR)
					panic("dwarf_siblingof()");

			}
			while (res != DW_DLV_NO_ENTRY);

			break;
		}

		{
			Dwarf_Die child_die;
			struct dtype_data **	member;

		case DW_TAG_union_type:
			p->dtype_class = DTYPE_CLASS_UNION;

type_access_stats.nr_union_nodes ++;			

			if (/* skip next statement */
					0)
		case DW_TAG_structure_type:
			{
			p->dtype_class = DTYPE_CLASS_STRUCT;

type_access_stats.nr_struct_nodes ++;			

			}
			/* if this is just a declaration, moan we cant handle it right now */
			if (p->is_node_a_declaration)
			{
					gprintf("downgraded from panic(), incomplete type found, DW_AT_declaration is set; doesnt know what to do");
					//panic("incomplete type found, DW_AT_declaration is set; doesnt know what to do");
{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("\n");
	printf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}
			}
			/* sanity check */
			else if (p->data_size == 0)
			{
{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}
printf("warning: zero-sized data object\n");
				}
			/* ok, read the members of the structure (the die children) and put them on the member list */
			/* read the first child */
			if ((res = dwarf_child(die, &child_die, &err)) == DW_DLV_ERROR)
				panic("dwarf_child()");
			member = &p->struct_data.data_members;
			while (res != DW_DLV_NO_ENTRY)
			{
			Dwarf_Die sib_die;

				*member = type_process(ctx, child_die);
				dwarf_util_set_parent(*member, p);
				/* make sure the data member location
				 * description is valid - also read the comments
				 * about the member_data.member_location data
				 * field in struct dtype_data - in header file
				 * type-access.h */
				if ((*member)->member_data.member_location == UINT_MAX)
				{
					/* make sure a union type is being
					 * built */
					if (tagval != DW_TAG_union_type)
					{
						gprintf("die offset: %i\n", (int) type_die_offset);
						gprintf("handle here c++ DW_TAG_subprograms, skipping panic now\n");
						//panic("");
					}
					/* fix up data member location */
					(*member)->member_data.member_location = 0;
				}
				res = dwarf_siblingof(ctx->dbg, child_die, &sib_die, &err);
				member = &(*member)->member_data.sib_ptr;
				if (res == DW_DLV_ERROR)
					panic("dwarf_siblingof()");
				dwarf_dealloc(ctx->dbg, child_die, DW_DLA_DIE);
				child_die = sib_die;
			}

			break;
		}
		case DW_TAG_subroutine_type:
			p->dtype_class = DTYPE_CLASS_SUBROUTINE;
			p->subroutine_data.sub = subprogram_process(ctx, die);
			break;

		case DW_TAG_unspecified_parameters:
			panic("handle unspecified parameters type");
			break;

		case DW_TAG_variant:
			panic("handle variant type");
			break;

		case DW_TAG_ptr_to_member_type:
			if (0) panic("handle pointer to member type");
			break;

		case DW_TAG_member:
		{
			p->dtype_class = DTYPE_CLASS_MEMBER;

			if (dwarf_hasattr(die, DW_AT_data_member_location, &flag, &err) != DW_DLV_OK)
				panic("dwarf_hasattr()");
			if (flag == 0)
			{
				/* data member location information not present */
				/* the dwarf 3 standard says in paragraph 5.6.6:
				 * The location description for a data member of a union may be omitted, since all data members of a union begin at the same address.
				 *
				 * i.e., this is a normal case for c union data
				 * members (and is an error for c structure
				 * data members) - but this cannot be checked
				 * here - it is this routine caller's responsibility
				 * to perform this check (also read the comments
				 * about the member_data.member_location data
				 * field in struct dtype_data - in header file
				 * type-access.h) */
				p->member_data.member_location = UINT_MAX;
			}
			else
			{
				/* data member location information
				 * present - process it */
				/* ok, compute member offset from the type base */
				if (dwarf_attr(die, DW_AT_data_member_location, &attr, &err) != DW_DLV_OK)
					panic("dwarf_attr()");

				if (dwarf_whatform(attr, &form, &err) != DW_DLV_OK)
					panic("dwarf_whatform");
				switch (form)
				{
					case DW_FORM_block:
					case DW_FORM_block1:
					{
						/*! \todo	maybe use support code from
						 *		dwarf-expr.c/dwarf-loc.c to compute
						 *		the member offsets */
						struct dwarf_location	loc;

						if (dwarf_loclist_n(attr, &loc.llbuf, &loc.listlen, &err) != DW_DLV_OK)
							panic("dwarf_loclist_n()");
						if (loc.listlen != 1)
							panic("weird case of an aggregate data type member location description");
#if 0
						if (dwarf_loc_eval_loc_from_list(&p->member_data.member_location, &is_in_reg, &is_fbreg_needed,
							&loc, 0, 0, 0, 0))
							panic("could not evaluate aggregate data type member location");
						/* basic sanity check */
						if (is_in_reg || is_fbreg_needed)
							panic("bad aggregate data type member location");
						break;
#endif
						/* shortcut this */
						if (loc.llbuf[0]->ld_cents != 1)
							panic("");
						if (loc.llbuf[0]->ld_s[0].lr_atom != DW_OP_plus_uconst)
							panic("");
						p->member_data.member_location = loc.llbuf[0]->ld_s[0].lr_number;
						/* these are not needed any more */	 
						dwarf_dealloc(ctx->dbg, loc.llbuf[0]->ld_s, DW_DLA_LOC_BLOCK);
						dwarf_dealloc(ctx->dbg, loc.llbuf[0], DW_DLA_LOCDESC);
						dwarf_dealloc(ctx->dbg, loc.llbuf, DW_DLA_LIST);
						break;
					}

					default:
						gprintf("unknown dwarf form for dwarf member data location attribute: 0x%hhx\n", form);
						{
{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("\n");
	printf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}
						panic("");
						}
				}
				/* alsoxxxx */
				dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
			}

			/* read the type die for the member */
			if (!dtype_access_get_type_die(ctx, die, &type_die))
			{
				gprintf("panic downgrade: aggregate member doesnt have a type");
				break;
				//panic("aggregate member doesnt have a type");
			}
			p->member_data.member_type = type_process(ctx, type_die);
			dwarf_dealloc(ctx->dbg, type_die, DW_DLA_DIE);

type_access_stats.nr_member_nodes ++;

			if (p->linkage_name)
				cxx_hacks_add_linkage_node(ctx, p);

			break;
		}
		case DW_TAG_set_type:
			panic("handle set type");
			break;

		case DW_TAG_access_declaration:
			panic("handle access declaration type");
			break;

		case DW_TAG_base_type:
		{
			/* sanity check */
			if (p->data_size == 0)
				panic("");
			p->dtype_class = DTYPE_CLASS_BASE_TYPE;
			/* read encoding */
			if (dwarf_attr(die, DW_AT_encoding, &attr, &err) != DW_DLV_OK)
				panic("dwarf_attr()");
			if (dwarf_formudata(attr, &return_uvalue, &err) != DW_DLV_OK)
				panic("dwarf_formudata()");
			dwarf_dealloc(ctx->dbg, attr, DW_DLA_ATTR);
			p->base_type_encoding = return_uvalue;
			/* see if the type already is present in the 
			 * symbol table */
			/* HACKS HACKS HACKS */
			if (0) {
				struct sym_struct	* sym;
				sym = symtab_find_sym(ctx, p->name);
				while (sym)
				{
					if (sym->symclass == SYM_TYPE)
					{
					struct dtype_data * p1;
					p1 = sym->dtype;
						if (p1->dtype_class == p->dtype_class
								&& p1->data_size == p->data_size
								&& !strcmp(p1->name, p->name))
						{


{
struct type_hash_node * px;

	px = ctx->types->htab[type_die_offset % ctx->types->htab_size];

	while (px)
	{
		if (px->die_offset == type_die_offset)
			break;
		px = px->next;
	}
	if (!px)
		panic("");
	px->dtype = p1;
}

							ctx->types->tcache --;
							ctx->types->tcache_nodes ++;
							p = p1;
							break;
						}
					}
					sym = sym->next;
				}
				if (!sym)
{

type_access_stats.nr_base_type_nodes ++;

					symtab_store_sym(ctx, p->name, SYM_TYPE, p);
}
			}
		else
type_access_stats.nr_base_type_nodes ++;
		}
type_access_stats.nr_base_type_nodes ++;			
			break;

		case DW_TAG_volatile_type:
			p->type_qualifier_data.qualifier_id = TYPE_QUALIFIER_VOLATILE;

type_access_stats.nr_tqual_nodes ++;			

			if (0)
				/* skip next statement */
		case DW_TAG_const_type:
{
			p->type_qualifier_data.qualifier_id = TYPE_QUALIFIER_CONST;

type_access_stats.nr_tqual_nodes ++;			
}
			p->dtype_class = DTYPE_CLASS_TYPE_QUALIFIER;
			if (!dtype_access_get_type_die(ctx, die, &type_die))
			{
				/* possible - this is a possible case when
				 * declaring untyped (void) pointers
				 * to constant memory (e.g. - in c:
				 * 'const void * ptr') */
			}
			else
			{
				p->type_qualifier_data.type = type_process(ctx, type_die);
				dwarf_dealloc(ctx->dbg, type_die, DW_DLA_DIE);
			}
			break;

		case DW_TAG_constant:
			panic("handle constant");
			break;

		case DW_TAG_file_type:
			panic("handle file type");
			break;
		case DW_TAG_inheritance:	
		case DW_TAG_subprogram:	
		case DW_TAG_template_type_parameter:	
			gprintf("dwarf tag not handled: %s\n", dwarf_util_get_tag_name(tagval));
			break;

		default:
			gprintf("dwarf tag not handled: %s\n", dwarf_util_get_tag_name(tagval));
			gprintf("skipping panic() now, downgraded from panic()\n");
{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}
			//panic("");
			break;
	}

	return p;
}


/*! \todo	document this */
void type_data_dump(struct dtype_data * type, void * data, char * name, int first_atom, int last_atom)
{
struct type_dump_seq seq;

	seq.type_printf = gprintf;
	seq.is_prefix_printing = 1;
	seq.first_atom = first_atom;
	seq.last_atom = last_atom;
	seq.cur_atom = 0;
	seq.indent_level = 0;

	type_data_dump_int(type, data, name, false, false, &seq);
	/* if this has been a data dump request, return */
	if (data)
		return;
	/* otherwise, this is a type dump request - proceed
	 * with type prefix printing */
	if (name)
		gprintf("%s", name);
	seq.is_prefix_printing = 0;
	type_data_dump_int(type, data, name, false, false, &seq);

}


/*! \todo	document this */
/*! \todo	explain and resolve the hacks about RAWTYPE printing
 *		and arrays - RAWTYPE IS CURRENTLY BROKEN FOR ARRAYS -
 *		there are two rawtype output records generated for
 *		arrays, of which only the second one is valid */
void type_dump_mi(struct dtype_data * type, const char * name, bool is_in_deref)
{
int i;
bool is_subrange;
struct type_dump_seq seq;
bool must_print_rawtype;

	/*! \todo	handle void here */
	if (!type)
	{
		panic("");
	}

	/* initialize the default machine interface printing routine */
	seq.type_printf = miprintf;

	/* by default, dump the rawtype description (e.g., useful for
	 * prettyprining in the console frontend) at the end of
	 * this function */
	must_print_rawtype = true;
	is_subrange = false;

	if (name)
		miprintf("NAME = \"%s\",", name);
	switch (type->dtype_class)
	{
		case DTYPE_CLASS_BASE_TYPE:
		{
			miprintf("TYPENAME = \"");
			/* print the base type using the internal printer */
			seq.is_prefix_printing = 1;
			seq.indent_level = 0;

			type_data_dump_int(type, 0, 0, false, false, &seq);
			seq.is_prefix_printing = 0;
			type_data_dump_int(type, 0, 0, false, false, &seq);

			miprintf("\"");
			miprintf(", ");
		}
			break;
		case DTYPE_CLASS_PTR:
		{
			if (!is_in_deref)
			{
				miprintf("DEREF_POINT, ");
				miprintf("TYPENAME = \"");

				/* trigger the typeprinter */
				seq.is_prefix_printing = 1;
				seq.indent_level = 0;

				type_data_dump_int(type/*->ptr_type*/, 0, 0, true, true, &seq);
				seq.is_prefix_printing = 0;
				type_data_dump_int(type/*->ptr_type*/, 0, 0, true, true, &seq);

				miprintf("\"");
				miprintf(", ");
			}
			else
				/* should never happen */
				panic("");
			break;
		}
		case DTYPE_CLASS_ARRAY_SUBRANGE:
			is_subrange = true;
		case DTYPE_CLASS_ARRAY:
		{
			struct dtype_data * arr_type;

			if (!is_in_deref)
			{
				/* used to iterate on the subranges */
				struct dtype_data *	p;
				/* number of array dimensions */
				int ndims;
				/*! an array storing the dimensions of an array that is being processed by ::type_dump_mi */
				int *arrdims;

				arrdims = (int *)malloc(MAX_ARRAY_DIMENSIONS * sizeof(*arrdims));
				if (!arrdims)
					panic("out of core");

				/* count number of indices of the array */
				i = 0;
				if (type->dtype_class == DTYPE_CLASS_ARRAY)
				{
					p = type->arr_data.subranges;
					arr_type = type->arr_data.element_type;
				}
				else
				{
					p = type;
					arr_type = type->arr_subrange_data.element_type;
				}
				while (p)
				{
					/*! \todo	handle flexible array members */
					if (p->arr_subrange_data.upper_bound == UINT_MAX)
						panic("");
					arrdims[i++] = p->arr_subrange_data.upper_bound;
					if (i == MAX_ARRAY_DIMENSIONS)
						panic("array has too many dimensions");
					p = p->arr_subrange_data.sib_ptr;
				}
				if (!i)
					panic("bad dwarf array type description");
				ndims = i;
				miprintf("ARRDIMS = ");
				for (i = 0; i < ndims; i++)
					miprintf("%i $ ", arrdims[i]);
				miprintf(", ");

				free(arrdims);
			}
			else
				/* should never happen */
				panic("");
			type_dump_mi(arr_type, 0, is_in_deref);
			break;
		}
		case DTYPE_CLASS_CLASS:
		{
			struct dtype_data * member;

			/*! \todo	remove this, debug only */
			gprintf("\ndumping class type for die at offset %i (0x%x)\n", (int) type->head.die_offset, (int) type->head.die_offset);

			miprintf("TYPENAME = \"");
			miprintf("class");

			if (type->name)
				miprintf(" %s", type->name);
			else
				miprintf(" <anonymous>");

			miprintf("\"");
			miprintf(", ");
			if (!is_in_deref)
			{
				miprintf("CHILDLIST = (");
				/* print members */
				for (member = type->struct_data.data_members;
					member;
						member = member->member_data.sib_ptr)
				{
					miprintf("[");
					type_dump_mi(member, member->name, is_in_deref);
					miprintf("]");
					miprintf(",");
				}
				miprintf(")");
				miprintf(", ");
			}
			break;
		}
		case DTYPE_CLASS_STRUCT:
		case DTYPE_CLASS_UNION:
		{
			struct dtype_data * member;
			miprintf("TYPENAME = \"");
			if (type->dtype_class == DTYPE_CLASS_STRUCT)
				miprintf("struct");
			else
				miprintf("union");

			if (type->name)
				miprintf(" %s", type->name);
			else
				miprintf(" <anonymous>");

			miprintf("\"");
			miprintf(", ");
			if (!is_in_deref)
			{
				miprintf("CHILDLIST = (");
				/* print members */
				for (member = type->struct_data.data_members;
					member;
						member = member->member_data.sib_ptr)
				{
					miprintf("[");
					type_dump_mi(member, member->name, is_in_deref);
					miprintf("]");
					miprintf(",");
				}
				miprintf(")");
				miprintf(", ");
			}
			break;
		}
		case DTYPE_CLASS_MEMBER:
			type_dump_mi(type->member_data.member_type, 0, is_in_deref);
			/*! \todo	??? why is this here ??? */
			must_print_rawtype = false;

			break;

		case DTYPE_CLASS_TYPEDEF:
			/*! \todo	this really needs careful handling... */
			/* hmm, this seems to be getting too far... it seems
			 * that because of buggy output generation here, and
			 * buggy handling in the troll frontend, the bugs somehow
			 * neutralize themselves and, in effect, the display
			 * in the troll frontend seems to be perfect...
			 * truly fascinating... anyway, don't tell anyone...
			 *
			 * maybe i should rename the engine from 'gear',
			 * to the more appopriate 'bugalore' */
#if 0
		{
			miprintf("TYPENAME = \"");
			/* print the typedef using the internal printer */
			seq.is_prefix_printing = 1;
			seq.indent_level = 0;

			type_data_dump_int(type, 0, 0, false, false, &seq);
			seq.is_prefix_printing = 0;
			type_data_dump_int(type, 0, 0, false, false, &seq);

			miprintf("\"");
			miprintf(", ");

		}
#elif 0
		{
			miprintf("TYPENAME = \"");

			miprintf("%s", type->name);

			miprintf("\"");
			miprintf(", ");

		}
#else
			type_dump_mi(type->typedef_data.type, 0, is_in_deref);
#endif			
			break;
		case DTYPE_CLASS_ENUMERATION:
		{
			miprintf("TYPENAME = \"");
			/* print the enumeration information using the internal printer */
			seq.is_prefix_printing = 1;
			seq.indent_level = 0;

			type_data_dump_int(type, 0, 0, false, false, &seq);
			seq.is_prefix_printing = 0;
			type_data_dump_int(type, 0, 0, false, false, &seq);

			miprintf("\"");
			miprintf(", ");
		}
			break;
		case DTYPE_CLASS_SUBROUTINE:
		{
			miprintf("TYPENAME = \"");
			/* print the base type using the internal printer */
			seq.is_prefix_printing = 1;
			seq.indent_level = 0;

			type_data_dump_int(type, 0, name, false, false, &seq);
			if (name)
				miprintf("%s", name);
			seq.is_prefix_printing = 0;
			type_data_dump_int(type, 0, name, false, false, &seq);

			miprintf("\"");
			miprintf(", ");
			break;
		}
		case DTYPE_CLASS_TYPE_QUALIFIER:
			type_dump_mi(type->type_qualifier_data.type, 0, is_in_deref);
			must_print_rawtype = false;
			break;
		default:
			miprintf("unknown data type found: %i\n", type->dtype_class);
			panic("");
	}

	/* see if a raw type declaration string should be printed */
	if (must_print_rawtype)
	{
		miprintf("RAWTYPE = \"");
		seq.is_prefix_printing = 1;
		seq.indent_level = 0;

		type_data_dump_int(type, 0, name, false, false, &seq);
		/*! \todo	check if this is ok */
		if (name)
			miprintf("%s", name);
		seq.is_prefix_printing = 0;
		type_data_dump_int(type, 0, name, false, false, &seq);
		miprintf("\", ");
	}
}


/*! \todo	document this */
void type_dump_data_mi(struct gear_engine_context * ctx, struct dtype_data * type, void * data)
{
int i;
unsigned long long c;
/*! a helper array used for array dumping by ::type_data_dump_int */
int *index_stack;
unsigned char * cptr;
/*! a helper for handling arrays/array subranges */
bool is_subrange;

	/*! \todo	handle void here */
	if (!type)
	{
		panic("");
	}
	is_subrange = false;
	cptr = (unsigned char *)data;

	if (type->is_node_a_declaration)
		panic("");

	switch (type->dtype_class)
	{
		case DTYPE_CLASS_BASE_TYPE:
			switch (type->base_type_encoding)
			{
				case DW_ATE_unsigned:
				case DW_ATE_unsigned_char:

					switch (type->data_size)
					{
						case 1:
							/* for bytes, if the character is
							 * a printable, dump it as a character too */
							c = *(unsigned char*)data;
							miprintf("\"0x%hhx", (unsigned char)c);
							if (isprint((unsigned char) c))
							{
								/* an ordinary character - see if escaping
								 * is necessary */
								miprintf("'");
								switch (c)
								{
									case '"': miprintf("\\\""); break;
									case '\\': miprintf("\\\\"); break;
									case '\a': miprintf("\\a"); break;
									case '\b': miprintf("\\b"); break;
									case '\f': miprintf("\\f"); break;
									case '\n': miprintf("\\n"); break;
									case '\r': miprintf("\\r"); break;
									case '\t': miprintf("\\t"); break;
									case '\v': miprintf("\\v"); break;
									default: miprintf("%c", c); break;
								}
								miprintf("'");
							}
							miprintf("\",", (unsigned char)c);
							break;
						case 2:
							c = *(unsigned short*)data;
							miprintf("\"0x%hx\",", (unsigned short)c);
							break;
						case 4:
							c = *(unsigned int*)data;
							miprintf("\"0x%lx\",", (unsigned int)c);
							break;
						case 8:
							c = *(unsigned long long *)data;
							miprintf("\"0x%llu\",", c);
							break;
						default:
							panic("");
					}
					break;
				case DW_ATE_signed:
				case DW_ATE_signed_char:
					switch (type->data_size)
					{
						case 1:
							/* for bytes, if the character is
							 * a printable, dump it as a character too */
							c = *(signed char*)data;
							miprintf("\"0x%hhx", (signed char)c);
							if (isprint((signed char) c))
							{
								/* an ordinary character - see if escaping
								 * is necessary */
								miprintf(" ('");
								switch (c)
								{
									case '"': miprintf("\\\""); break;
									case '\\': miprintf("\\\\"); break;
									case '\a': miprintf("\\a"); break;
									case '\b': miprintf("\\b"); break;
									case '\f': miprintf("\\f"); break;
									case '\n': miprintf("\\n"); break;
									case '\r': miprintf("\\r"); break;
									case '\t': miprintf("\\t"); break;
									case '\v': miprintf("\\v"); break;
									default: miprintf("%c", c); break;
								}
								miprintf("')");
							}
							miprintf("\",", (signed char)c);
							break;
						case 2:
							c = *(signed short*)data;
							miprintf("\"0x%hx\",", (signed short)c);
							break;
						case 4:
							c = *(signed int*)data;
							miprintf("\"0x%lx\",", (signed int)c);
							break;
						case 8:
							c = *(unsigned long long *)data;
							miprintf("\"0x%lli\",", c);
							break;
						default:
							panic("");
					}
					break;
				case DW_ATE_boolean:
					switch (type->data_size)
					{
						case 1:
							c = *(unsigned char*)data;
							miprintf("\"%s(%i)\",", c ? "true" : "false", (unsigned char)c);
							break;
						default:
							panic("");
					}
					break;
				default:
					panic("");
			}
			break;
		case DTYPE_CLASS_PTR:
		{
			/* as a special case here, if this looks like a c-like
			 * string (i.e. a pointer to 1-byte sized data),
			 * also try dumping the string (by calling the
			 * gear_engine_context.dump_cstring_from_target_mem()
			 * routine, if available), so that the data displayed
			 * is hopefully more useful to a human user */
			struct dtype_data * deref_type;
			miprintf("\"0x%08x", *(unsigned int*)data);
			/* see if this looks like a c-like string and if so,
			 * try dumping the string as well */
			if ((deref_type = dtype_access_get_unqualified_base_type(type->ptr_type))
					//&& deref_type->dtype_class == DTYPE_CLASS_BASE_TYPE
					&& deref_type->data_size == /* one byte */ 1
					&& ctx->dump_cstring_from_target_mem)
			{
				miprintf(" '");
				ctx->dump_cstring_from_target_mem(ctx, * (ARM_CORE_WORD *) data);
				miprintf("'");
			}	
			miprintf("\",", *(unsigned int*)data);
			break;
		}
		case DTYPE_CLASS_ARRAY_SUBRANGE:
			is_subrange = true;
		case DTYPE_CLASS_ARRAY:
		{
			/* used to iterate on the subranges */
			struct dtype_data *	p;
			int nr_elements;
			struct dtype_data * arr_type;
			int arr_element_size;

			if (!is_subrange)
				arr_type = type->arr_data.element_type;
			else
				arr_type = type->arr_subrange_data.element_type;

			if (!is_subrange)
				p = type->arr_data.subranges;
			else
				p = type;
			/* count total number of elements (all dimensions combined) */
			nr_elements = 1;
			while (p)
			{
				/*! \todo	handle flexible array members */
				if (p->arr_subrange_data.upper_bound == UINT_MAX)
					panic("");
				nr_elements *= p->arr_subrange_data.upper_bound + 1;
				p = p->arr_subrange_data.sib_ptr;
			}
			if (!nr_elements)
				panic("bad dwarf array type description");

			/* dump data */
			arr_element_size = dtype_access_sizeof(arr_type);
			while (nr_elements--)
			{
				type_dump_data_mi(ctx, arr_type, (void *)cptr);
				cptr += arr_element_size;
			}

			break;

		}
		case DTYPE_CLASS_CLASS:
		case DTYPE_CLASS_STRUCT:
		case DTYPE_CLASS_UNION:
		{
			struct dtype_data * member;

			/* dump data */
			for (member = type->struct_data.data_members;
				member;
					member = member->member_data.sib_ptr)
			{
				if (member->is_node_a_static_cxx_struct_member)
					panic("");
				if (member->member_data.member_location == UINT_MAX)
					panic("");
				type_dump_data_mi(ctx, member,
						(void *)(cptr + member->member_data.member_location));
			}
			break;
		}

		case DTYPE_CLASS_MEMBER:
		{
			/* see if this is a bit field */
			if (type->member_data.bit_size)
			{
				uint64_t u;
				int t;
				/*! \todo	properly handle endianness and sizes here */
				switch (type->data_size)
				{
					default:
						panic("");
						break;
					case 1:
						u = *(uint8_t *) data;
						t = sizeof(uint8_t);
						break;	
					case 2:
						u = *(uint16_t *) data;
						t = sizeof(uint16_t);
						break;	
					case 4:
						u = *(uint32_t *) data;
						t = sizeof(uint32_t);
						break;	
					case 8:
						u = *(uint64_t *) data;
						t = sizeof(uint64_t);
						break;	
				}
				t *= 8 /* bits per byte */;
				u >>= t - (type->member_data.bit_offset + type->member_data.bit_size);
				/* mask out unused bits */
				u &= (1 << type->member_data.bit_size) - 1;

				type_dump_data_mi(ctx, type->member_data.member_type, &u);
			}
			else
				type_dump_data_mi(ctx, type->member_data.member_type, data);
			break;
		}

		case DTYPE_CLASS_TYPE_QUALIFIER:
		{
			type_dump_data_mi(ctx, type->type_qualifier_data.type, data);
			break;
		}

		case DTYPE_CLASS_TYPEDEF:
			type_dump_data_mi(ctx, type->typedef_data.type, data);
			break;

		case DTYPE_CLASS_ENUMERATION:
			{
			unsigned int t;

				i = type->data_size;
				if (i <= 0)
					panic("");
				t = 0;
				/*! \todo	properly handle the enumerator
				 *		endianness, size and sign here...
				 *		this below is only for low endian
				 *		data, and does not handle the
				 *		enumerator size properly */
				gprintf("enumerator size is %i:\n", i);
				/*! \todo	this is for low endian machines */
				while (i--)
					t |= cptr[i] << (8 /* bits in a byte*/ * i);
				/* scan the enumerator chidren of the enumeration
				 * hoping to find a symbolic name for the enumerator
			         * value computed */
				for (i = 0; i < type->enum_data.nr_enumerators; i ++)
					if (type->enum_data.enumerators[i].enum_const == t)
						/* match found */
						break;
				miprintf("\"%i (%s)\",", t,
						(i != type->enum_data.nr_enumerators)
						? type->enum_data.enumerators[i].name : "<<< unknown enumerator value >>>");
				break;
			}
		default:
			gprintf("unknown data type found: %i\n", type->dtype_class);
			panic("");
	}
}

/*!
 *	\fn	bool dtype_access_are_types_compatible(struct dtype_data * t1, struct dtype_data * t2)
 *	\brief	determines if two data types are compatible
 *
 *	\note	quoting the ISO/IEC 9899 technical corrigendum 2 (TC2):
 *
 * Two types have compatible type if their types are the same. Additional rules for
 * determining whether two types are compatible are described in 6.7.2 for type specifiers,
 * in 6.7.3 for type qualifiers, and in 6.7.5 for declarators.46)
 *
 * (46) above is a footnote, saying:
 * 46) Two types need not be identical to be compatible.
 *
 *	this routine basically makes sure the types to compare
 *	are identical (recursing in the process)
 *
 *	\todo	type qualifiers are currently not checked
 *	\todo	enumeration compatibility checking is currently
 *		not done properly; again, quoting the forementioned
 *		document:
 * Each enumerated type shall be compatible with char, a signed integer type, or an
 * unsigned integer type. The choice of type is implementation-defined,108) but shall be
 * capable of representing the values of all the members of the enumeration.
 *
 * (108) above is a footnote number, saying:
 * 108) An implementation may delay the choice of which integer type until all enumeration constants have
 * been seen.
 *
 *
 *	\param t1	a pointer to the first type to be used in the
 *			compatibility comparison
 *	\param t2	a pointer to the second type to be used in the
 *			compatibility comparison
 *	\return	true, if the types t1 and t2 are compatible in
 *		the terms mentioned above, false otherwise
 */
bool dtype_access_are_types_compatible(struct dtype_data * t1, struct dtype_data * t2)
{

	if (!t1 && !t2)
		/* both types are void - consider them compatible */
		return true;
	if (!(t1 && t2))
		/* one type is void - other is different from void;
		 * consider them incompatible */
		return false;
	switch (t1->dtype_class)
	{
		case DTYPE_CLASS_TYPEDEF:
		case DTYPE_CLASS_INVALID:
			panic("");
			break;
		case DTYPE_CLASS_BASE_TYPE:
			switch (t2->dtype_class)
			{
				case DTYPE_CLASS_ENUMERATION:
				case DTYPE_CLASS_TYPEDEF:
				case DTYPE_CLASS_INVALID:
					panic("");
					return false;
				case DTYPE_CLASS_ARRAY:
				case DTYPE_CLASS_ARRAY_SUBRANGE:
				case DTYPE_CLASS_STRUCT:
				case DTYPE_CLASS_UNION:
					return false;
				case DTYPE_CLASS_MEMBER:
					return dtype_access_are_types_compatible(t1, t2->member_data.member_type);
				case DTYPE_CLASS_PTR:
					return dtype_access_are_types_compatible(t1, t2->ptr_type);
				case DTYPE_CLASS_BASE_TYPE:
					if (t1->data_size == t2->data_size
							&& t1->base_type_encoding == t2->base_type_encoding)
						return true;
					else
						return false;

			}
		case DTYPE_CLASS_PTR:
			switch (t2->dtype_class)
			{
				case DTYPE_CLASS_TYPEDEF:
				case DTYPE_CLASS_INVALID:
					panic("");
					return false;
				case DTYPE_CLASS_ENUMERATION:
				case DTYPE_CLASS_ARRAY:
				/*! \todo	move this and process it
				 *		along with DTYPE_CLASS_ARRAY */
				case DTYPE_CLASS_ARRAY_SUBRANGE:
				case DTYPE_CLASS_STRUCT:
				case DTYPE_CLASS_UNION:
				case DTYPE_CLASS_BASE_TYPE:
					return false;
				case DTYPE_CLASS_MEMBER:
					return dtype_access_are_types_compatible(t1, t2->member_data.member_type);
				case DTYPE_CLASS_PTR:
					return dtype_access_are_types_compatible(
							dtype_access_get_unqualified_base_type(t1->ptr_type),
							dtype_access_get_unqualified_base_type(t2->ptr_type));
			}

		case DTYPE_CLASS_ARRAY_SUBRANGE:
		{
			/* temporary storage */
			struct dtype_data * tt1, * tt2;
			tt1 = t1;
			tt2 = t2;
			if (t2->dtype_class != DTYPE_CLASS_ARRAY_SUBRANGE)
				return false;
			/* ok, see if the dimensions match */
			while (t1)
			{
				if (!t2)
					return false;
				/*! \todo	handle flexible array members */
				if (t1->arr_subrange_data.upper_bound == UINT_MAX)
					panic("");
				/*! \todo	handle flexible array members */
				if (t2->arr_subrange_data.upper_bound == UINT_MAX)
					panic("");
				if (t1->arr_subrange_data.upper_bound !=
						t2->arr_subrange_data.upper_bound)
					return false;
				t1 = t1->arr_subrange_data.sib_ptr;
				t2 = t2->arr_subrange_data.sib_ptr;
			}
			if (t2)
				return false;
			/* dimensions match - check element types */
			return dtype_access_are_types_compatible(tt1->arr_subrange_data.element_type, tt2->arr_subrange_data.element_type);
		}

		case DTYPE_CLASS_ARRAY:
		{
			/* temporary storage */
			struct dtype_data * tt1, * tt2;
			tt1 = t1;
			tt2 = t2;
			if (t2->dtype_class != DTYPE_CLASS_ARRAY)
				return false;
			/* ok, see if the dimensions match */
			t1 = t1->arr_data.subranges;
			t2 = t2->arr_data.subranges;
			while (t1)
			{
				if (!t2)
					return false;
				/*! \todo	handle flexible array members */
				if (t1->arr_subrange_data.upper_bound == UINT_MAX)
					panic("");
				/*! \todo	handle flexible array members */
				if (t2->arr_subrange_data.upper_bound == UINT_MAX)
					panic("");
				if (t1->arr_subrange_data.upper_bound !=
						t2->arr_subrange_data.upper_bound)
					return false;
				t1 = t1->arr_subrange_data.sib_ptr;
				t2 = t2->arr_subrange_data.sib_ptr;
			}
			if (t2)
				return false;
			/* dimensions match - check element types */
			return dtype_access_are_types_compatible(tt1->arr_data.element_type, tt2->arr_data.element_type);
		}
		case DTYPE_CLASS_STRUCT:
		case DTYPE_CLASS_UNION:
			if (t2->dtype_class != t1->dtype_class)
				return false;
			/* see if the tags (if present) match */
			if (strcmp(t1->name, t2->name))
				return false;
			/* compare types member-wise */
			t1 = t1->struct_data.data_members;
			t2 = t2->struct_data.data_members;
			while (t1)
			{
				if (!t2)
					return false;
				/*! \todo	this here (possibly?)
				 *		invalidates the tail recursion
				 *		that the compiler infers
				 *		for this function;
				 *		fix this */
				if (!dtype_access_are_types_compatible(t1, t2))
					return false;
				t1 = t1->member_data.sib_ptr;
				t2 = t2->member_data.sib_ptr;
			}
			if (t2)
				return false;
			/* types match - render them compatible */
			return true;
		case DTYPE_CLASS_ENUMERATION:
		{
			int i1, i2;
			/*! \todo	this is incomplete; see the
			 *		enumeration compatibility
			 *		notes in the function header
			 * 		comments */
			if (t2->dtype_class != DTYPE_CLASS_ENUMERATION)
				return false;
			/* see if the tags (if present) match */
			if (strcmp(t1->name, t2->name))
				return false;

			/* make sure the number of elements match */
			i1 = t1->enum_data.nr_enumerators;
			i2 = t2->enum_data.nr_enumerators;
			if (i1 != i2)
				return false;
			/* compare types member-wise */
			/*! \todo	the comparison here is incorrect;
			 *		only the member names and values
			 *		must be checked, their order
			 * 		in the members list is irrelevant;
			 *		to support this efiiciently, the
			 *		lists must be sorted, and that is
			 *		currently not being done - fix this */
			i1 = 0;
			while (i1 != i2)
			{
				if (strcmp(t1->enum_data.enumerators[i1].name,
						t2->enum_data.enumerators[i1].name))
					return false;
				if (t1->enum_data.enumerators[i1].enum_const
						!= t2->enum_data.enumerators[i1].enum_const)
					return false;
				i1 ++;
			}
			/* types match - render them compatible */
			return true;
		}
		case DTYPE_CLASS_MEMBER:
			if (t2->dtype_class != DTYPE_CLASS_MEMBER)
				return dtype_access_are_types_compatible(t1->member_data.member_type, t2);
			if (strcmp(t1->name, t2->name))
				return false;
			return dtype_access_are_types_compatible(t1->member_data.member_type,
					t2->member_data.member_type);
		default:
			panic("");
			break;
	}
}

/*!
 *	\fn	bool dtype_access_is_ctype(enum CTYPE_ENUM what_type, struct dtype_data * type)
 *	\brief	determines if a supplied type is of a c type class specified
 *
 *	\todo	this is quite suboptimal - remove the stupid recursive
 *		invocations here
 *
 *	\note	arrays are a special case - they may be pointers
 *		or aggregates, depending on context of use (e.g.
 *		sizeof operator, ampersand operator); here, these
 *		are represented by DTYPE_CLASS_ARRAY and DTYPE_CLASS_ARRAY_SUBRANGE
 *		nodes
 *
 *	\param	what_type	the type class to check for
 *	\param	type	a pointer to the type to be checked against what_type
 *	\return	true, if the supplied type is in the type class
 *		specified by what_type, false othwerwise
 */
bool dtype_access_is_ctype(enum CTYPE_ENUM what_type, struct dtype_data * type)
{
	/* check for void type - the void type is an
	 * incomplete type (which can never be completed) */
	if (!type)
		return false;
	/* strip any type qualifiers */
	if (type->dtype_class == DTYPE_CLASS_TYPE_QUALIFIER)
		return dtype_access_is_ctype(what_type, type->type_qualifier_data.type);

	/* if this is a typedef - skip forward to the type defined */
	if (type->dtype_class == DTYPE_CLASS_TYPEDEF)
		return dtype_access_is_ctype(what_type, type->typedef_data.type);

	switch (what_type)
	{
		case CTYPE_INVALID:
			panic("");
			return 0;
		case CTYPE_SCALAR:
			return dtype_access_is_ctype(CTYPE_ARITHMETIC, type)
				|| dtype_access_is_ctype(CTYPE_POINTER, type)
				|| dtype_access_is_ctype(CTYPE_FLOAT, type);
		case CTYPE_ARITHMETIC:
			return dtype_access_is_ctype(CTYPE_INTEGER, type) || dtype_access_is_ctype(CTYPE_FLOAT, type);
		case CTYPE_POINTER:
			if (type->dtype_class == DTYPE_CLASS_PTR
					/* these added */
					/*! \todo	these don't make sense; i have apparently added
					 *		them as some point as a hack, but don't remember
					 *		why; these commented out on 30102012, are
					 *		scheduled for removal; remove them when these
					 *		indeed prove useless
					|| type->dtype_class == DTYPE_CLASS_ARRAY
					|| type->dtype_class == DTYPE_CLASS_ARRAY_SUBRANGE
					*/
					)
				return true;
			else
				return false;
		case CTYPE_INTEGER:
			return dtype_access_is_ctype(CTYPE_SIGNED, type) ||
				dtype_access_is_ctype(CTYPE_UNSIGNED, type);
		case CTYPE_FLOAT:
			if (type->dtype_class == DTYPE_CLASS_BASE_TYPE && type->base_type_encoding == DW_ATE_float)
				return true;
			else
				return false;
		case CTYPE_UNSIGNED:
		case CTYPE_SIGNED:
			if (type->dtype_class == DTYPE_CLASS_BASE_TYPE)
				switch (type->base_type_encoding)
				{
					case DW_ATE_signed:
					case DW_ATE_signed_char:
						if (what_type == CTYPE_SIGNED)
							return true;
						else
							return false;
					case DW_ATE_unsigned_char:
					case DW_ATE_unsigned:
						if (what_type == CTYPE_UNSIGNED)
							return true;
						else
							return false;
					case DW_ATE_float:
						return false;
					default:
						panic("");
						return false;
				}
			else
				return false;
		case CTYPE_AGGREGATE:
			switch (type->dtype_class)
			{
				case DTYPE_CLASS_ARRAY:
				/* this one added (special case) */
				case DTYPE_CLASS_ARRAY_SUBRANGE:
				case DTYPE_CLASS_STRUCT:
				case DTYPE_CLASS_UNION:
					return true;
			}
			return false;
		case CTYPE_STRUCT_OR_UNION_OR_CXX_CLASS:
			switch (type->dtype_class)
			{
				case DTYPE_CLASS_CLASS:
				case DTYPE_CLASS_STRUCT:
				case DTYPE_CLASS_UNION:
					return true;
			}
			return false;
			break;
		default:
			panic("");
			return false;
	}

}

/*!
 *	\fn	struct dtype_data * dtype_access_get_deref_type(struct dtype_data * type)
 *	\brief	given a pointer data type, retrieves the data type pointed to by the type passed
 *
 *	\param	type	a pointer to a dtype_data struct describing
 *			a pointer type or an array type; this cannot
 *			be null
 *
 *	\return	a pointer to the data type that the passed 'type' parameter
 *		points to, when the star (dereference) operator is
 *		applied to an expression of the type passed;
 *		note that this can be an array subrange
 */
struct dtype_data * dtype_access_get_deref_type(struct dtype_data * type)
{

	if (!type)
		panic("");
	switch (type->dtype_class)
	{
		case DTYPE_CLASS_TYPE_QUALIFIER:
			return dtype_access_get_deref_type(type->type_qualifier_data.type);
		case DTYPE_CLASS_TYPEDEF:
			return dtype_access_get_deref_type(type->typedef_data.type);
		case DTYPE_CLASS_PTR:
			return type->ptr_type;
		case DTYPE_CLASS_ARRAY:
			type = type->arr_data.subranges;
			if (!type)
				panic("");
			/* fall out */
		case DTYPE_CLASS_ARRAY_SUBRANGE:
			if (type->arr_subrange_data.sib_ptr)
				return type->arr_subrange_data.sib_ptr;
			return type->arr_subrange_data.element_type;
		default:
			panic("non pointer type passed");
	}
}

/*!
 *	\fn	struct dtype_data * dtype_access_get_unqualified_base_type(struct dtype_data * type)
 *	\brief	given a data type, returns the underlying, unqualified, 'bare' type by drilling into the data type given and skipping over any type qualifiers and into c-style 'typedefs'
 *
 *	\param	type	the data type of interest
 *	\return	a data type node which will have all of its leading
 *		type qualifiers skipped over, and, if the type
 *		node starts with a (chain of) c-style 'typedef'(s),
 *		the underlying type (that this/these typedef(s)
 *		provide synonym of) is reached into - and so on, so
 *		that this function returns a data type node which
 *		has no leading type qualifiers, and is not a
 *		c-style 'typedef */
struct dtype_data * dtype_access_get_unqualified_base_type(struct dtype_data * type)
{
	/* it is possible that 'type' here be null in the case
	 * of c 'void' pointers */
	if (type) while (1)
	{
		if (type->dtype_class == DTYPE_CLASS_TYPEDEF)
		{
			type = type->typedef_data.type;
			continue;
		}
		else if (type->dtype_class == DTYPE_CLASS_TYPE_QUALIFIER)
		{
			type = type->type_qualifier_data.type;
			continue;
		}
		else break;
	}
	return type;
}

/*!
 *	\fn	void init_types(struct gear_engine_context * ctx)
 *	\brief	initializes the dwarf type access module (this module)
 *
 *	\param	ctx	context to worl in
 *	\return	none */
void init_types(struct gear_engine_context * ctx)
{
int size;
/*! \todo	make the size adjustable */
#define HTAB_SIZE	39989


	/*! \todo	make the size adjustable */
	size = sizeof * ctx->types + HTAB_SIZE * sizeof(struct type_hash_node *);
	if (!(ctx->types = malloc(size)))
		panic("");
	memset(ctx->types, 0, size);
	ctx->types->htab_size = HTAB_SIZE;
}

