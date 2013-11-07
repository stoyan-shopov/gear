/*!
 * \file	symtab.c
 * \brief	global debugger symbol table code
 * \author	shopov
 *
 *	the purpose of this is to quickly map symbol names to programming
 *	entities present in the program that is being debugged;
 *	a 'symbol' here denotes what is usually expected for
 *	a debugger - 'symbols' are functions, variables and data types;
 *	these are entities that have 'names' and may or may not occupy
 *	physical storage (e.g. program variables versus variable types);
 *	this module is responsible for maintaining a symbol table for
 *	the program being debugged, with the help of which efficient
 *	lookup by name is possible for the program symbols
 *
 *	not all program symbols are accounted for in the global debugger
 *	symbol table; symbols not put here are, for example, labels and
 *	variables of limited life - such as automatic ones and formal
 *	parameters
 *
 *	symbols that should be always put here are global and static variables,
 *	type definitions (typedefs), functions and non-anonymous struct, union
 *	and enum declarations
 *
 *	note that there can be multiple different symbols with the same name in
 *	the symbol table (e.g. static variables of the same name in different
 *	source files)
 *
 *	the global symbol table is here to provide efficient name lookup facilities
 *	and as such should contain a minimum of overhead and additional
 *	housekeeping informaion
 *
 *	verbose program structure information is obtained by the compilation unit parser,
 *	which builds the debug information tree naturally corresponding to
 *	the program being debugged; this parser is actually responsible
 *	for inserting symbols in the global symbol table
 *
 *	\todo	the global symbol table must be organized as a hash table, right now
 *		it is just a simple list
 *	\todo	it is possible to do merging of equal types present in different
 *		compilation units (and this will save *a lot* symbol space and remove
 *		the irritating duplicates) - should be done as soon as possible
 *	\todo	currently the pointers to symbol names are directly assigned to data
 *		fields in the hash nodes - no duplicates of the name strings are made;
 *		as currently these strings never get deallocated or changed, this is ok,
 *		but bear this in mind should anything in the symbol name string handling
 *		change in a way incompatible with this assumption
 *
 * Revision summary:
 *
 * $Log: $
 */


/*
 *
 * include section follows
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
/* for elf_hash()... */
#include <libelf.h>

#include "dwarf-common.h"
#include "gear-engine-context.h"
#include "gprintf.h"
#include "util.h"
#include "symtab.h"

/*
 *
 * local types follow
 *
 */



/*! the basic symbol table hash node
 *
 * this contains the name of the symbol that hashes to this
 * bucket in the hash table, a pointer to the next hash node
 * of the same hash value and a pointer to a list of symbols
 * with the same name */
struct symtab_hash_node
{
	/*! the name of the symbol(s) in this list */
	const char * name;
	/*! pointer to the next symbol hashing to the same value */
	struct symtab_hash_node * next;
	/*! pointer to the list of the symbols with this name */
	struct sym_struct * symlist;
};


/*! \todo document this */
struct symtab_hash_table
{
	int			htab_size;
	struct symtab_hash_node	* htab[0];
};

/*
 *
 * local functions follow
 *
 */

/*!
 *	\fn		static struct symtab_hash_node * get_hash_node(void)
 *	\brief	allocates a symtab_hash_node structure
 *
 *	\return	a pointer to the newly allocated node
 */

static struct symtab_hash_node * get_hash_node(void)
{
struct symtab_hash_node * p;

	if (!(p = calloc(1, sizeof * p)))
		panic("out of core");
	return p;
}

/*!
 * \fn		static struct sym_struct* get_sym_node(void)
 * \brief	allocates a sym_struct data structure
 *
 * \return	a pointer to the newly allocated node
 */

static struct sym_struct * get_sym_node(void)
{
struct sym_struct * p;

	if (!(p = malloc(sizeof(*p))))
		panic("out of core");
	memset(p, 0, sizeof(*p));
	return p;
}

/*!
 *	\fn	static struct symtab_hash_node * find_hash_node(struct gear_engine_context * ctx, const char * name)
 *	\brief	looks for a symtab_hash_node struct for the name supplied
 *
 *	\param	ctx	context to work in
 *	\param	name	the name of the symbol to search for
 *	\return	a pointer to the symtab_hash_node struct for the symbol, or 0
 *		if the symbol was not found
 */

static struct symtab_hash_node * find_hash_node(struct gear_engine_context * ctx, const char * name)
{
struct symtab_hash_node * p;

	if (!name)
		return 0;

	p = ctx->symtab->htab[elf_hash(name) % ctx->symtab->htab_size];
	while (p)
	{
		if (!strcmp(name, p->name))
			break;
		p = p->next;
	}
	return p;
}

/*
 *
 * exported functions follow
 *
 */


/*!
 *	\fn	struct sym_struct * symtab_find_sym(struct gear_engine_context * ctx, const char * name)
 *	\brief	locates a symbol in the symbol table
 *
 *	\param	ctx	context to work in
 *	\param	name	name of the symbol to be found
 *	\return	pointer to a list of symbols with the name of interest, 0
 *		if the symbol is not found
 */
struct sym_struct * symtab_find_sym(struct gear_engine_context * ctx, const char * name)
{
struct symtab_hash_node * p;

	if (!name)
		panic("");
	return (p = find_hash_node(ctx, name)) ?
		p->symlist : 0;
}


/*!
 *	\fn	void symtab_store_sym(struct gear_engine_context * ctx, const char * name, enum SYM_CLASS_ENUM symclass, void * payload)
 *	\brief	puts a symbol in the symbol table
 *
 *	\param	ctx	context to work in
 *	\param	name	the symbol's name
 *	\param	symclass	class of the symbol (see enum ::SYM_CLASS_ENUM for details)
 *	\param	payload		the pointer to the symclass-specific data for the symbol
 *	\return	none
 */
void symtab_store_sym(struct gear_engine_context * ctx, const char * name, enum SYM_CLASS_ENUM symclass, void * payload)
{
struct symtab_hash_node * p;
struct sym_struct * sym;

	if (!name)
		panic("");
	/*! \todo	always allocate and initialize a new sym_struct node; once
	 *		type merging is done (see the todo section at the beginning of this
	 *		file), this will no longer be the case */
	sym = get_sym_node();
	/* validate the symbol class */
	printf("%s(): storing symbol \"%s\" into the symbol table\n", __func__, name);
	switch (symclass)
	{
		default:
		case SYM_INVALID:
			panic("");
			break;
		case SYM_DATA_OBJECT:
			sym->dobj = payload;
			gprintf("%s(): storing data object '%s'\n", __func__, name);
			break;
		case SYM_SUBROUTINE:
			sym->subprogram = payload;
			break;
		case SYM_TYPE:
			sym->dtype = payload;
			break;
		case SYM_ENUM_CONSTANT:
			sym->enum_const_data = payload;
			break;
	}
	sym->symclass = symclass;

	/* first, see if there is a hash node for the name supplied */
	p = find_hash_node(ctx, name);
	if (!p)
	{
	unsigned long h;	
		/* symbol not found create and insert new hash node */
		p = get_hash_node();
		/*! \todo	this can be a *very* serious problem
		 *		but right now name strings should never
		 *		get deallocated in a debugging session */
		p->name = name;
		h = elf_hash(name) % ctx->symtab->htab_size;
		p->next = ctx->symtab->htab[h];
		ctx->symtab->htab[h] = p;
	}
	/*! \todo	directly plug the new symbol in the symbol list;
	 *		this must go when symbols get sorted on this
	 *		list according to their class (there are varios
	 *		reasons to do this, but right now theres no time) */
	sym->next = p->symlist;
	p->symlist = sym;
	{
		static int xxx = 0;
		gprintf("symtab size: %i\n", xxx ++);
	}

	type_access_stats.nr_symtab_nodes ++;
}

/*!
 *	\fn	void init_symtab(struct gear_engine_context * ctx)
 *	\brief	symbol table initialization
 *
 *	\param	ctx	context to work in
 *	\return	none
 */

void init_symtab(struct gear_engine_context * ctx)
{
int size;
/*! \todo	make the size adjustable */
#define HTAB_SIZE	39989


	/*! \todo	make the size adjustable */
	size = sizeof * ctx->symtab + HTAB_SIZE * sizeof(struct symtab_hash_node *);
	if (!(ctx->symtab = malloc(size)))
		panic("");
	memset(ctx->symtab, 0, size);
	ctx->symtab->htab_size = HTAB_SIZE;
}

