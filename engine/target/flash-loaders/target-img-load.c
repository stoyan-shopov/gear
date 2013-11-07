/*!
 *	\file	target-img-load.c
 *	\brief	code for loading data into target
 *	\author	shopov
 *
 *	\note	throughout the code and comments within this module, the
 *		word 'ram' (short for random access memory) is used to denote
 *		a memory region in a target that can be modified directly,
 *		without the need to perform special procedures in order
 *		to do so; likewise, the word 'flash' is used to denote
 *		a memory region in a target that, in order to be modified,
 *		there is the need to perform special procedures (performed
 *		by what is called here a 'flash loader')
 *
 *	\todo	the first version of this code is *highly* incomplete,
 *		ineffective and inflexible; for example, only executable
 *		elf files are handled right now, and many other limitations
 *		are also in effect (right now)
 *	\todo	described and documented here is only the first version
 *		of this module, which has many flaws and faults; the
 *		comments here must be thouroughly proofread and adjusted
 *		whenever changes of this module are introduced; the 'todo'
 *		notes here shall never be removed until a stable state
 *		of this module has been achieved, and what 'stable' here
 *		means is debatable, currently not defined
 *
 *
 *	the purpose of this module is to provide an interface for populating
 *	target memory regions with data; for embedded targets, these target
 *	memory regions typically consist of volatile (e.g. ram) and
 *	non-volatile (e.g. flash) memory; the data contents to be transferred
 *	to target memory may come from different sources, such as an object
 *	file (e.g. an elf file), a file in a format that is typically used for
 *	programming an embedded target (e.g. a hex or an s-record file),
 *	directly from a memory buffer passed to the routines in this
 *	module, etc.; for a typical embedded target, an example usage of
 *	this module can be populating the ram and flash regions of a
 *	target prior to launching a debugging session, or populating the
 *	flash regions only, in the case where only programming of the
 *	flash is needed (as is often needed in, for example, mass production)
 *
 *	as purely a target memory loader, this module does not care and
 *	does not know anything about debugging information - but still
 *	needs to be able to figure out several important parameters, vital
 *	for achieving its goals; these are described below
 *
 *	a key concept, vital to the operation of the target loader,
 *	is the target nonvolatile memory loader (hereafter referred to as the
 *	'flash memory loader', or just 'flash loader'); basically, this is
 *	a piece of code that is loaded in the target system (more
 *	specifically, in target memory that can be modified directly, without
 *	the need of any special procedures - this is most commonly ram memory)
 *	and is then utilized to program the target memory regions
 *	that need special procedures in order to be modified (this
 *	is most commonly nonvolatile memory - e.g. flash); in order to
 *	achieve this, this module must be able to access target memory,
 *	in order to load the flash loader and access flash loader
 *	data parameters, and also to set breakpoints and execute
 *	code in the target
 *
 *	so, in order to utilize a flash loader for a target in order
 *	to modify target memory that needs special handling, the
 *	following must be resolved:
 *		- the exact target used - in order to determine
 *		target parameters such as memory maps, any necessary special
 *		initialization sequences (e.g. setting up sdram, pll
 *		registers, etc.)
 *		- the exact flash loader(s) to be used - there may be
 *		several of them, e.g. in the case a target is outfitted with
 *		different memories (e.g. different external flash chips,
 *		internal flash, etc.) that may need different handling
 *
 *	furthermore, for each flash loader that is going to be used, the
 *	following conditions must hold:
 *		- the flash loader must be made present in the target
 *		memory system; if it is not already loaded in target
 *		memory when this module is invoked, then this module
 *		must be able to load the flash loader in target memory
 *		by directly utilizing target memory writing routines; this
 *		means that if the flash loader is not yet present in target
 *		memory (an example of when a flash loader is already
 *		present in target memory is, e.g. when the flash
 *		loader has been previously loaded in target nonvolatile
 *		memory, or is supplied by a chip vendor in internal chip rom
 *		memory), then all memory regions, that the flash loader
 *		must occupy in target memory, must be directly modifiable
 *		by this module (most commonly, this means that all of the
 *		flash loader memory regions must reside in target ram
 *		memory that is up and running (it may probably need some
 *		registers being set up - as outline above))
 *
 *	furthermore, it is mandatory that the value (the address) of a special
 *	data area, that contains data needed for the flash loader to be
 *	properly executed, must be resolved; this special data area, hereafter
 *	referred to as the 'gear_flash_data', has the following layout
 *	(in assembly):
 *	\todo	the code here is arm-specific, in the sense that a 32 bit
 *		architecture is assumed; should this ever change, make
 *		the appropriate adjustments to the documentation here

################# start of sample assembly code #################

# the scratch area size, in bytes, to be used by the
# gear engine flash loading code
#define SCRATCH_AREA_SIZE	0xxxxxxxx
#define HEADER_MAGIC_WORD	0xxxxxxxx
#define FOOTER_MAGIC_WORD	0xxxxxxxx

	.global gear_flash_data
	.data
	.align 4

	gear_flash_data:
	# header magic word
	.word	HEADER_MAGIC_WORD
	# void	* scratch_area;
	.word	scratch_area
	# int	scratch_area_len;
	.word	SCRATCH_AREA_SIZE
	# the gear engine flash loader halt location
	.word	gear_engine_halt_location
	# int	(*flash_init)(void);
	.word	flash_init
	# int	(*flash_shutdown)(void);
	.word	flash_shutdown
	# int	(*flash_erase)(void * addr, int size);
	.word	flash_erase
	# int	(*flash_write)(void * addr, void * data, int size);
	.word	flash_write
	# footer magic word
	.word	FOOTER_MAGIC_WORD

# sample scratch area definition
	scratch_area:
	.fill SCRATCH_AREA_SIZE, 1, 0
################# end of sample assembly code #################
 *
 * 	\warning	all of the data fields above should be aligned on a
 * 			target core word boundary, should be of size exactly
 * 			one target core word, and should strictly follow each
 * 			other in the order described above, without any
 * 			nonempty padding between thems and the total
 * 			byte size of gear_flash_data data area
 * 			must equal the 'GEAR_FLASH_DATA_BYTE_SIZE'
 * 			constant defined below;	these requirements
 * 			are absolutely vital for the proper functioning
 * 			of this module;
 *	the data fields in this data area are:
 *		- HEADER_MAGIC_WORD - a magic word put at the
 *		start of the gear_flash_data data area; along with
 *		FOOTER_MAGIC_WORD below, this is used
 *		as a crude error detection mechanism in the
 *		gear_flash_data data area layout; this
 *		must equal GEAR_FLASH_DATA_HEADER_MAGIC_WORD
 *		(defined below in enum GEAR_FLASH_DATA_MAGIC_ENUM);
 *		also see the note below about validating the
 *		gear_flash_data data area layout
 *		- void * scratch_area - the address of a memory area in
 *		the target that can be used by this module (the
 *		target memory loading module) as an area for
 *		passing data to the flash loader code; the larger
 *		this, the better, as this is mainly used as a data
 *		buffer for passing data to be written by the
 *		flash loader, and a larger data buffer generally
 *		results in faster operation, as the data buffer
 *		will generally have to be repopulated less often
 *		and the target shall have to be interrupted less often;
 *		also see the comments below about the scratch_area_len
 *		field below
 *		- int scratch_area_len - the size of the scratch area
 *		above, in bytes; unreasonably low (e.g. less than
 *		a target core word size) values may fail to work;
 *		it is best to make this as big as possible
 *		- gear_engine_halt_location - this is a pointer to
 *		a location in target memory that can be breakpointed
 *		and that, when breakpointed and attempted to execute
 *		an instruction from, must cleanly halt the
 *		target core; this is needed by this module in order
 *		to run the flash loader code, also read the comments below
 *		about the calling convention of the flash loader;
 *		generally, this should point to memory that wont cause
 *		some target core exception when tried to execute code
 *		from (most commonly, this should be a location in
 *		memory that is readily available to the target memory
 *		system, and maybe - for a harvard target core - architecture
 *		will also reside in a memory region that can be fetched
 *		code from)
 *		- int	(*flash_init)(void) - a pointer to the flash
 *		loader initialization code; this is guaranteed to
 *		be invoked prior to any other flash loader function
 *		(but also read the comments below about how the
 *		flash loader is initialized when its code is first
 *		loaded in target memory);
 *		this is supposed to perform any flash loader initialization
 *		code, if necessary (e.g., unlocking flash memory
 *		for writing, etc.);
 *		for general flash loader code calling conventions and
 *		return values, read the comments below
 *		- int	(*flash_shutdown)(void) - flash loader shutdown code;
 *		it is guaranteed that this will be the last flash loader
 *		function invoked by this module; it is meant to perform
 *		any flash loader shutdown maintenance operations,
 *		if necessary (e.g., locking access to flash memory);
 *		for general flash loader code calling conventions and
 *		return values, read the comments below
 *		- int	(*flash_erase)(void * addr, int size) - erase
 *		a region of target memory, preparing it for writing;
 *		the parameter addr is the starting address of the memory
 *		region to be erased, the parameter size is the size
 *		of the memory region, in bytes;
 *		for general flash loader code calling conventions and
 *		return values, read the comments below
 *		- int	(*flash_write)(void * addr, void * data, int size) -
 *		write data in a target memory region;
 *		the parameter adddr is the starting address of the memory
 *		region to be written to, the parameter data is a pointer
 *		to the data buffer containing the data to be written,
 *		the parameter size is the size of the memory region, in bytes;
 *		for general flash loader code calling conventions and
 *		return values, read the comments below
 *		- FOOTER_MAGIC_WORD - a magic word put at the
 *		end of the gear_flash_data data area; along with
 *		HEADER_MAGIC_WORD above, this is used
 *		as a crude error detection mechanism in the
 *		gear_flash_data data area layout; this
 *		must equal GEAR_FLASH_DATA_FOOTER_MAGIC_WORD
 *		(defined below in enum GEAR_FLASH_DATA_MAGIC_ENUM);
 *		also see the note below about validating the
 *		gear_flash_data data area layout
 *
 *	\todo	the magic word values right now are quite obscene...
 *
 *	\note	as a crude gear_flash_data data area layout validation
 *		mechanism, there are two magic words - one at the start
 *		and one at the end of the gear_flash_data data area;
 *		if these two do not equal the values
 *		GEAR_FLASH_DATA_HEADER_MAGIC_WORD and
 *		GEAR_FLASH_DATA_FOOTER_MAGIC_WORD,
 *		respectively, a gear_flash_data data area is considered
 *		invalid; the constants GEAR_FLASH_DATA_HEADER_MAGIC_WORD
 *		and GEAR_FLASH_DATA_FOOTER_MAGIC_WORD are defined
 *		in the GEAR_FLASH_DATA_MAGIC_ENUM enumeration below
 *
 *		general flash loader code calling conventions and result
 *		return details: for an arm, the standard aapcs calling
 *		convention for the flash loader routines is employed;
 *		on success, the flash loader routines must return zero,
 *		on failure, the flash loader routines must return a
 *		negative value; thus, the allowed return values for
 *		the flash loader routines are only the nonpositive integers;
 *		this module shall arrange the target execution environment
 *		for the flash loader in such a way, that the flash loader
 *		routines shall seem to have been invoked in such a way that
 *		they must return to the gear_engine_halt_location location
 *		described above (also read the comments above data field
 *		gear_engine_halt_location above); this is needed so that
 *		this module can breakpoint this location, and then invoke
 *		flash loaders routines in such a way that they return to
 *		this location - this way it will be known when a flash
 *		loader routine has completed and it is safe to proceed
 *		with other necessary target operations; also, when
 *		the flash loader image is first loaded in target memory,
 *		it is given a chance to initialize its runtime environment
 *		(such as setting up stacks, etc.) by breakpoining
 *		this same gear_engine_halt_location and transferring
 *		control to the flash loader entry point; in this case,
 *		it is also expected that the flash loader transfers
 *		control to the gear_engine_halt_location when done with
 *		initializing its runtime environment, and that the result
 *		returned shall, in this case, also denote success (0) or
 *		failure (a negative return value)
 *	\note	the allowed return values for the flash loader routines 
 *		are only the nonpositive integers
 *
 *	in summary, in order to utilize a flash loader, the following must be
 *	resolved:
 *		- the exact target used - in order to determine
 *		target parameters such as memory maps, any necessary special
 *		initialization sequences (e.g. setting up sdram, pll
 *		registers, etc.)
 *		- the exact flash loader(s) to be used - there may be
 *		several of them, e.g. in the case a target is outfitted with
 *		different memories (e.g. different external flash chips,
 *		internal flash, etc.) that may need different handling;
 *		the flash loader(s) to be used must be all directly loadable
 *		in the target, and a special gear_flash_data data area
 *		(detailed above) must be supplied, which is used by this
 *		module to invoke the flash loader so that it populates
 *		the required target memory regions
 *
 *	exactly how are these things resolved is dependent on the target
 *	environment used, the format of the data to be loaded and probably
 *	many other things; for specific details, see the appropriate routines
 *	within this module
 *
 *	as an example, a common case is the flash loader
 *	to be contained in an elf executable file, properly linked for
 *	some specific target system - in this case, this module inspects
 *	the loadable program headers of this file to make sure they are all
 *	contained in target ram (and not flash) memory, then loads the
 *	program segments that must be loaded; then, the executable elf file
 *	symbol table is searched for the globally visible symbol with
 *	the name defined in the 'ELF_GEAR_FLASH_DATA_SYMBOL_NAME' constant
 *	below, and its value is used as the starting address of the
 *	gear_flash_data data area described above
 *
 *	\note	however tempting to write in a flash loader c source file the
 *		construct: 
 *		////////// c source example start //////////
 *		struct
 *		{
 *			int	header_magic_word;
 *			void	* scratch_area;
 *			int	scratch_area_len;
 *			void	* gear_engine_halt_location;
 *			int	(*flash_init)(void);
 *			int	(*flash_shutdown)(void);
 *			int	(*flash_erase)(void * addr, int size);
 *			int	(*flash_write)(void * addr, void * data, int size);
 *			int	footer_magic_word;
 *		}
 *		gear_flash_data = { ... };
 *		////////// c source example end //////////
 *		this is probably not the best idea (even though it will
 *		probably work 100 per cent of the time); also read the warning
 *		about the gear_flash_data data area representation above;
 *		this is because the compiler used may have some fancy ideas
 *		about c structure member placing - it is thus best to exercise
 *		ultimate control on the translation process for the
 *		gear_flash_data data area and use some construct as the
 *		assembly code outlined above
 *
 *	the intention is to make this module flexible and versatile enough
 *	so that it can be easily incorporated in a standalone program
 *	to be used for loading data in target memory - that is, to be
 *	efficiently used as a target loader outside the gear engine
 *
 *	\todo	all of the above said, this module right now supports
 *		only loading elf executable images in target memory,
 *		using only elf executable flash loaders, and the target
 *		determination and configuration is currently hard coded
 *		(this alone is a topic that must be handled with great care)
 *
 *	\note	throughout the code and comments within this module, the
 *		word 'ram' (short for random access memory) is used to denote
 *		a memory region in a target that can be modified directly,
 *		without the need to perform special procedures in order
 *		to do so; likewise, the word 'flash' is used to denote
 *		a memory region in a target that, in order to be modified,
 *		there is the need to perform special procedures (performed
 *		by what is called here a 'flash loader')
 *
 *	\todo	detail the interaction with the executor (exec callback
 *		overriding, etc.) and the handling of commands from
 *		a gear frontend during flash loading
 *
 *	\todo	endianness is not handled anywhere here; everywhere here
 *		little endianness is assumed
 *
 *	\todo	the magic word values right now are quite obscene...
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

/*! \todo	this is braindamaged */
#define _LARGEFILE64_SOURCE
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <malloc.h>
#include <libelf.h>

#include "dwarf-common.h"
#include "gear-constants.h"
#include "gear-engine-context.h"
#include "target-img-load.h"
#include "engine-err.h"
#include "target-defs.h"
#include "core-access.h"
#include "util.h"

/*
 *
 * local constants follow
 *
 */

/*! the symbollic name used to locate the gear_flash_data data area in an elf file
 *
 * this is the symbollic name used to locate the gear_flash_data data area
 * (described in the comments at the start of this module)
 * in the case that a flash loader resides in an an elf executable
 * file and is read from there; the elf symbol table is scanned for
 * this symbol and the symbols value is deemed the starting address of
 * the gear_flash_data data area of the flash loader; the symbol must
 * have global binding and default visibility; this is used in
 * function retrieve_gear_flash_data_from_elf() */
static const char * const ELF_GEAR_FLASH_DATA_SYMBOL_NAME = "gear_flash_data";

/*! the byte size of the gear_flash_data data area */
static const int GEAR_FLASH_DATA_BYTE_SIZE = 9 * sizeof(ARM_CORE_WORD);

/*! the number of retries to poll the target waiting for it to halt when executing flash loader code in the target */
static const int FLASH_LOADER_HALT_POLL_RETRY_CNT = 10;

/*! the initial value of the sleep interval between successive polls of the target when waiting fo it to halt (in microseconds)
 *
 * this is the initial time to sleep between successive
 * polls of the target, when running flash loader code;
 * currently, this value is doubled between successive polls
 * in function exec_flasher();
 * a value of 2 milliseconds, and a value of 10 for the
 * FLASH_LOADER_HALT_POLL_RETRY_CNT value above yield
 * a total waiting time of about 4 seconds in the case of
 * some fatal error because of which execution of the target
 * will not halt; these values seem reasonable */
static const int FLASH_LOADER_SLEEP_USECONDS_INIT = 2000;

/*
 *
 * local data types follow
 *
 */

/*! the enumeration used when validating the layout of a gear_flash_data data area
 *
 * for details, see the comments about the validation of
 * a gear_flash_data data area at the start of this module; the enumerator
 * constants are only briefly described here
 *
 * \todo	if this is moved, fix the xref details above and
 *		fix xref details to this in the comments at the start
 *		of module target-img-load.c
 */
enum GEAR_FLASH_DATA_MAGIC_ENUM
{
	/*! value for the magic word at the start of the gear_flash_data data area */
	GEAR_FLASH_DATA_HEADER_MAGIC_WORD	= 0xdaeba,
	/*! value for the magic word at the end of the gear_flash_data data area */
	GEAR_FLASH_DATA_FOOTER_MAGIC_WORD	= 0xdaadaeba,

};

/*! the gear_flash_data data area described at the start of this module
 *
 * for details, see the comments at the start of this module, the
 * data fields of this structure are only briefly described here */
struct gear_flash_data
{
	/*! header magic word, must equal GEAR_FLASH_DATA_HEADER_MAGIC_WORD */
	ARM_CORE_WORD	header_magic_word;
	/*! gear loader target scratch area address
	 *
	 * this is mainly used as a buffer for passing data to the flash loader
	 * in the target memory */
	ARM_CORE_WORD	scratch_area_addr;
	/*! the size of the above, in bytes */
	ARM_CORE_WORD	scratch_area_len;
	/*! the address to be used by the gear loader as a halt location when running flash loader code */
	ARM_CORE_WORD	gear_engine_halt_location;
	/*! the address of the flash loader initialization routine */
	ARM_CORE_WORD	(*flash_init)(void);
	/*! the address of the flash loader shutdown routine */
	ARM_CORE_WORD	(*flash_shutdown)(void);
	/*! the address of the flash loader erase routine */
	ARM_CORE_WORD	(*flash_erase)(void * addr, ARM_CORE_WORD size);
	/*! the address of the flash loader write routine */
	ARM_CORE_WORD	(*flash_write)(void * addr, void * data, ARM_CORE_WORD size);
	/*! footer magic word, must equal GEAR_FLASH_DATA_FOOTER_MAGIC_WORD */
	ARM_CORE_WORD	footer_magic_word;
};

/*! data structure (put in a gear engine context) containing data used during target memory loading */
struct target_img_load_data
{
	/*! the target state, used when running flash loader code in the target
	 *
	 * this is updated by the target state change callback function
	 * flasher_target_state_change_callback()
	 *
	 *	\todo	this is currently unused, and may never be needed
	 * */
	enum TARGET_CORE_STATE_ENUM	target_state;
};

/*! \todo	this is outrageously inconsistent, fix it up after
 * 		the first version of this module is finished */
struct target_mem_map_struct
{
	ARM_CORE_WORD		ram_start;
	ARM_CORE_WORD		ram_size;
	ARM_CORE_WORD		flash_start;
	ARM_CORE_WORD		flash_size;

};

/*
 *
 * local data follows
 *
 */


/*
 *
 * local functions follows
 *
 */


/*!
 *	\fn	static int is_elf_segment_in_ram(const Elf32_Phdr * seg, const struct target_mem_map_struct * target)
 *	\brief	checks if a whole elf loadable program segment resides in a target ram region
 *
 *	\note	this function does not support loadable program segments
 *		spanning multiple (ram) target memory regions, even if these
 *		regions are contiguous (that is, it returns false(zero)
 *		on such occasions)
 *
 *	\param	seg	a pointer to an elf program header describing
 *			the program segment that is queried; if loadable,
 *			it is checked if it is wholy contained in a target
 *			ram memory region
 *	\param	target	a pointer to the target memory map descriptor
 *	\return	nonzero, if the supplied program segment is loadable and
 *		completely resides in a ram target memory region, zero otherwise */
static int is_elf_segment_in_ram(const Elf32_Phdr * seg, const struct target_mem_map_struct * target)
{
	if (seg->p_type == PT_LOAD
		&& (target->ram_start >= seg->p_vaddr
			&& target->ram_start + target->ram_size
			>= seg->p_vaddr + seg->p_memsz))
		return 1;
	else
		return 0;

}

/*!
 *	\fn	static int is_elf_segment_in_flash(const Elf32_Phdr * seg, const struct target_mem_map_struct * target)
 *	\brief	checks if a whole elf loadable program segment resides in a target flash region
 *
 *	\note	this function does not support loadable program segments
 *		spanning multiple (flash) target memory regions, even if these
 *		regions are contiguous (that is, it returns false(zero)
 *		on such occasions)
 *
 *	\param	seg	a pointer to an elf program header describing
 *			the program segment that is queried; if loadable,
 *			it is checked if it is wholy contained in a target
 *			flash memory region
 *	\param	target	a pointer to the target memory map descriptor
 *	\return	nonzero, if the supplied program segment is loadable and
 *		completely resides in a flash target memory region, zero otherwise */
static int is_elf_segment_in_flash(const Elf32_Phdr * seg, const struct target_mem_map_struct * target)
{
	if (seg->p_type == PT_LOAD
		&& (target->flash_start >= seg->p_vaddr
			&& target->flash_start + target->flash_size
			>= seg->p_vaddr + seg->p_memsz))
		return 1;
	else
		return 0;

}


/*!
 * 
 *	\fn	static int is_elf_file_supported(Elf32_Ehdr * ehdr)
 *	\brief	checks an executable elf file header for unsupported flags
 *
 *	\param	ehdr	a pointer to an elf header data structure
 *	\return	nonzero, if the elf header passed is of an elf file
 *		that can be handled by this module, zero otherwise
 *	\todo	handle any attributes section here
 */
static int is_elf_file_supported(Elf32_Ehdr * ehdr)
{
Elf32_Word flags;

	if (ehdr->e_ident[EI_MAG0] != ELFMAG0
			|| ehdr->e_ident[EI_MAG1] != ELFMAG1
			|| ehdr->e_ident[EI_MAG2] != ELFMAG2
			|| ehdr->e_ident[EI_MAG3] != ELFMAG3
			|| ehdr->e_ident[EI_CLASS] != ELFCLASS32
			|| ehdr->e_ident[EI_DATA] != ELFDATA2LSB
			|| ehdr->e_ident[EI_VERSION] != 1
			/*! \todo	who and how defines this
			 *		ELFOSABI_ARM ?!?!?! */
			|| ehdr->e_ident[EI_OSABI] != ELFOSABI_ARM
			/*! \todo	what is this, anyway!?!? */
			/*|| ehdr->e_ident[EI_ABIVERSION] != */)
		panic("");
	if (ehdr->e_type != ET_EXEC
			|| ehdr->e_machine != EM_ARM
			|| ehdr->e_version != 1)
		panic("");
	/* check for unsupported elf header flags */
	flags = ehdr->e_flags;
	if (flags & EF_ARM_RELEXEC
			|| !(flags & EF_ARM_HASENTRY)
			|| flags & EF_ARM_INTERWORK
			|| flags & EF_ARM_APCS_26
			|| flags & EF_ARM_APCS_FLOAT
			|| flags & EF_ARM_PIC
			|| flags & EF_ARM_ALIGN8
			|| flags & EF_ARM_NEW_ABI
			|| flags & EF_ARM_OLD_ABI
			/* the next flag values are defined
			 * in binutils/include/elf/arm.h;
			 * i(sgs) decided to hardcode
			 * this here as it may be too much
			 * trouble for one to get possession
			 * of this header */
			|| flags & 0x200 /* == EF_ARM_SOFT_FLOAT */
			|| flags & 0x400 /* == EF_ARM_VFP_FLOAT */
			|| flags & 0x800 /* == EF_ARM_MAVERICK_FLOAT */
			|| flags & 0x00800000 /* == EF_ARM_BE8 */
			|| flags & 0x00400000 /* == EF_ARM_LE8 */)
		panic("");	
	/* check abi version */
	if (EF_ARM_EABI_VERSION(flags) == 0x05000000 /* == EF_ARM_EABI_VER5 */)
		panic("");
	return 0;
}


/*!
 *	\fn	static void load_elf_prog_segment_in_ram(struct gear_engine_context * ctx, Elf32_Phdr * seg, int elf_fd)
 *	\brief	loads a loadable elf program segment in target ram
 *
 *	this routine transfers a loadable elf program segment in target
 *	ram; the destination target address equals the segment virtual
 *	address; if necessary, the ram image of the segment is padded
 *	with zeroes (this is the case when the segment memory size,
 *	is larger than the segment file size; bss-style segments are
 *	supported (the case when the segment file size equals zero)
 *
 *	\param	ctx	context to work in
 *	\param	seg	elf program segment to load in target memory
 *	\param	elf_fd	file descriptor of the elf file containing the
 *			program segment to be loaded, used for reading
 *			from the file
 *	\return	none
 *
 *	\todo	define and code error codes */
static void load_elf_prog_segment_in_ram(struct gear_engine_context * ctx, Elf32_Phdr * seg, int elf_fd)
{
char * buf;
int nbytes;

	/* sanity checks */
	if (seg->p_type != PT_LOAD)
		panic("");
	if (seg->p_filesz > seg->p_memsz)
		panic("");
	if (!seg->p_memsz)
		panic("");

	if (!(buf = calloc(1, seg->p_memsz)))
		panic("");
	if (lseek64(elf_fd, (off64_t) seg->p_offset, SEEK_SET)
			!= (off64_t) seg->p_offset)
		panic("");
	if (read(elf_fd, buf, seg->p_filesz)
			!= seg->p_filesz)
		panic("");
	/* ok, buffer is in core, padded with zeroes
	 * if necessary, transfer it to target memory */
	nbytes = seg->p_memsz;
	if (ctx->cc->core_mem_write(ctx, seg->p_vaddr, buf, &nbytes)
			!= GEAR_ERR_NO_ERROR)
		panic("");
	if (nbytes != seg->p_memsz)
		panic("");
	free(buf);
}

/*!
 *	\fn	static struct gear_flash_data * retrieve_gear_flash_data_from_elf(Elf * elf)
 *	\brief	retrieves the gear_flash_data data area of a flash loader contained in an elf executable file
 *
 *	this routine retrieves the gear_flash_data data area of
 *	a flash loader, if it is in an elf executable file; the
 *	elf files symbol table is scanned for the symbol with
 *	the name defined by the constand 'ELF_GEAR_FLASH_DATA_SYMBOL_NAME'
 *	above; this symbol, if found, is expected to be with global
 *	binding and default visibility, and its value is taken
 *	to equal the start address of the flash loader gear_flash_data
 *	data area; the flash_data_area data area is retrieved from
 *	the elf file, its header and footer magic words are checked,
 *	and if everything seems ok, a pointer to a struct
 *	gear_flash_data data structure is returned to the caller
 *
 *	\param	elf	a handle to the elf executable file containing
 *			the flash loader; typically obtained by
 *			calling the elf_begin() routine from the
 *			libelf library
 *	\return	a pointer to a struct gear_flash_data data structure, if
 *		a gear_flash_data data area was successfully retrieved
 *		from the flash loader elf executable file supplied,
 *		0 otherwise; the gear_flash_data data structure
 *		must be free()-d by the caller when no longer of use */
static struct gear_flash_data * retrieve_gear_flash_data_from_elf(Elf * elf)
{
Elf32_Shdr * shdr, * strtab_shdr;
Elf_Scn * scn, * strtab_scn;
Elf_Data * symdata, * strdata, * flash_data;
/* the elf header is needed here in order to determine
 * the string table section header index */
Elf32_Ehdr * ehdr;
Elf32_Sym * sym;
Elf32_Addr addr;
int i;
struct gear_flash_data * gear_flash_data;

	scn = elf_nextscn(elf, 0);
	while (scn)
	{
		shdr = elf32_getshdr(scn);
		if (!shdr)
			panic("");
		if (shdr->sh_type == SHT_SYMTAB)
			break;
		scn = elf_nextscn(elf, scn);
	}
	if (!scn)
		panic("");
	/* read the string table */
	ehdr = elf32_getehdr(elf);
	if (!ehdr)
		panic("");
	strtab_scn = elf_getscn(elf, (size_t) ehdr->e_shstrndx);
	if (!strtab_scn)
		panic("");
	strtab_shdr = elf32_getshdr(strtab_scn);
	if (!strtab_shdr)
		panic("");
	if (strtab_shdr->sh_type != SHT_STRTAB)
		panic("");
	strdata = elf_getdata(strtab_scn, 0);
	if (!strdata)
		panic("");
	if (!strdata->d_buf || strdata->d_type != ELF_T_BYTE)
		panic("");

	symdata = elf_getdata(scn, 0);
	while (symdata)
	{
		if (!symdata->d_buf || strdata->d_type != ELF_T_SYM)
			panic("");
		sym = (Elf32_Sym *) symdata->d_buf;
		/* check symbol name */
		i = sym->st_name;
		if (i >= strdata->d_size)
			panic("");
		i = strdata->d_size - i;
		if (!strncmp(strdata->d_buf + sym->st_name,
				ELF_GEAR_FLASH_DATA_SYMBOL_NAME, i))
			break;
		/* skip to the next symbol */
		symdata = elf_getdata(scn, symdata);
	}
	if (!symdata)
		/* ELF_GEAR_FLASH_DATA_SYMBOL_NAME symbol not found -
		 * gear_flash_data data area not available */
		panic("");

	/* symbol found - validate it */
	if (ELF32_ST_BIND(sym->st_info) != STB_GLOBAL
			|| ELF32_ST_VISIBILITY(sym->st_other) != STV_DEFAULT)
		panic("");
	/* read the symbol containing section header;
	 * the section virtual(load) address is
	 * needed to properly locate the
	 * gear_flash_data data area */
	scn = elf_getscn(elf, sym->st_shndx);
	if (!scn)
		panic("");
	shdr = elf32_getshdr(scn);
	if (!shdr)
		panic("");
	/* validate section */
	if (shdr->sh_type != SHT_PROGBITS
			|| shdr->sh_flags & SHF_MASKPROC)
		panic("");
	/* read section contents */
	flash_data = elf_getdata(scn, 0);
	/* compute the gear_flash_data data area offset in
	 * the section */
	addr = sym->st_value;
	if (addr < shdr->sh_addr)
		panic("");
	addr -= shdr->sh_addr;
	if (addr + GEAR_FLASH_DATA_BYTE_SIZE > flash_data->d_size)
		panic("");
	/* populate a fresh gear_flash_data data structure */
	if (!(gear_flash_data = malloc(sizeof * gear_flash_data)))
		panic("");
	/*! \todo	endianness is not handled here at all */
	memcpy(&gear_flash_data->header_magic_word,
			(char *) flash_data->d_buf + 0 * sizeof(ARM_CORE_WORD),
				sizeof(ARM_CORE_WORD));
	memcpy(&gear_flash_data->scratch_area_addr,
			(char *) flash_data->d_buf + 1 * sizeof(ARM_CORE_WORD),
				sizeof(ARM_CORE_WORD));
	memcpy(&gear_flash_data->scratch_area_len,
			(char *) flash_data->d_buf + 2 * sizeof(ARM_CORE_WORD),
				sizeof(ARM_CORE_WORD));
	memcpy(&gear_flash_data->gear_engine_halt_location,
			(char *) flash_data->d_buf + 3 * sizeof(ARM_CORE_WORD),
				sizeof(ARM_CORE_WORD));
	memcpy(&gear_flash_data->flash_init,
			(char *) flash_data->d_buf + 4 * sizeof(ARM_CORE_WORD),
				sizeof(ARM_CORE_WORD));
	memcpy(&gear_flash_data->flash_shutdown,
			(char *) flash_data->d_buf + 5 * sizeof(ARM_CORE_WORD),
				sizeof(ARM_CORE_WORD));
	memcpy(&gear_flash_data->flash_erase,
			(char *) flash_data->d_buf + 6 * sizeof(ARM_CORE_WORD),
				sizeof(ARM_CORE_WORD));
	memcpy(&gear_flash_data->flash_write,
			(char *) flash_data->d_buf + 7 * sizeof(ARM_CORE_WORD),
				sizeof(ARM_CORE_WORD));
	memcpy(&gear_flash_data->footer_magic_word,
			(char *) flash_data->d_buf + 8 * sizeof(ARM_CORE_WORD),
				sizeof(ARM_CORE_WORD));
	/* check magic words */
	if (gear_flash_data->header_magic_word != GEAR_FLASH_DATA_HEADER_MAGIC_WORD
			|| gear_flash_data->footer_magic_word != GEAR_FLASH_DATA_FOOTER_MAGIC_WORD)
		panic("");
	/* make sure all of the addresses supplied reside within the flash
	 * loader program segments */
	/*! \todo	code this */
	gprintf("!!! validate address ranges !!!\n");
	return gear_flash_data;
}

/*!
 *	\fn	static bool flasher_target_state_change_callback(struct gear_engine_context * ctx, enum TARGET_CORE_STATE_ENUM state)
 *	\brief	the target state change callback used to update the internal to this module target state variable
 *
 *	\todo	currently, this is a dummy routine that does nothing
 *		and is here just to prevent any other callbacks
 *		from being invoked; figure out if this is really
 *		needed for douing anything else
 *
 *	\param	ctx	context to work in
 *	\param	state	the new state of the target
 *	\return	always false, denoting that no more target state change
 *		callbacks in the callback stack should be invoked
 *		(it is supposed that this callback is the only
 *		one inveoked) */
static bool flasher_target_state_change_callback(struct gear_engine_context * ctx, enum TARGET_CORE_STATE_ENUM state)
{
	ctx->img_loader_data->target_state = state;
	return false;
}


/*!
 *	\fn	static int exec_flasher(struct gear_engine_context * ctx, ARM_CORE_WORD run_addr, ARM_CORE_WORD halt_addr, ARM_CORE_WORD param0, ARM_CORE_WORD param1)
 *	\brief	executes code already loaded in target memory, until a supplied halt address is hit by the target code
 *
 *	this routine puts a breakpoint on the halt_addr address supplied and
 *	runs the target from the run_addr address supplied, and waits until
 *	the target stops at the breakpoint on the halt_addr address; it is
 *	assumed that the code to execute has already been loaded in target
 *	memory
 *
 *	\param	ctx	context to work in
 *	\param	run_addr	start_address to run the target from
 *	\param	halt_addr	an address to put a breakpoint on, and
 *				wait until the target code halts on
 *	\param	param0	first parameter value to be passed to the flash
 *			loader code (not always used)
 *	\param	param1	second parameter value to be passed to the flash
 *			loader code (not always used)
 *	\return	a positive integer value error code, in case there was an error
 *		during the execution of code in the target; the flash loader
 *		return code otherwise (the flash loader is allowed to return
 *		only nonpositive integer error values); thus, as the flash loader
 *		code generally returns zero on success, a zero returned from
 *		this routine indicates success (that is, both executing the code
 *		in the target wass successful, and the target routine completed
 *		without errors)
 *
 *	\todo	maybe change the error return code; specify error codes and
 *		conditions precisely */
static int exec_flasher(struct gear_engine_context * ctx, ARM_CORE_WORD run_addr, ARM_CORE_WORD halt_addr,
		ARM_CORE_WORD param0, ARM_CORE_WORD param1)
{
int i;
/* this is the time to sleep between successive calls to retrieve the
 * target state when waiting for the target to halt; currently,
 * this time is doubled between successive polls for the target state,
 * to give the target sufficient time to run;
 * a starting value of 1 or 2 milliseconds seems reasonable */
useconds_t sleep_time;
ARM_CORE_WORD reg[3];
enum TARGET_CORE_STATE_ENUM state;

	/* retrieve the target core state */
	target_comm_issue_core_status_request(ctx);
	if (ctx->cc->core_get_status(ctx, &state) !=
			GEAR_ERR_NO_ERROR)
		panic("");
	if (state != TARGET_CORE_STATE_HALTED)
		panic("");
	/* write the parameter values and the program counter to the target core */
	reg[0] = param0;
	reg[1] = param1;
	reg[2] = run_addr;
	if (ctx->cc->core_reg_write(ctx, 0, (1 << 15) | (1 << 1) | (1 << 0), reg)
			!= GEAR_ERR_NO_ERROR)
		panic("");
	/* put a breakpoint on the expected halting address */
	if (ctx->cc->core_set_break(ctx, halt_addr)
			!= GEAR_ERR_NO_ERROR)
		panic("");
	if (ctx->cc->core_run(ctx)
			!= GEAR_ERR_NO_ERROR)
		panic("");
	i = FLASH_LOADER_HALT_POLL_RETRY_CNT;
	sleep_time = FLASH_LOADER_SLEEP_USECONDS_INIT;
	while (i)
	{
		target_comm_issue_core_status_request(ctx);
		if (ctx->cc->core_get_status(ctx, &state) !=
				GEAR_ERR_NO_ERROR)
		if (state == TARGET_CORE_STATE_HALTED)
			break;
		usleep(sleep_time);
		sleep_time *= 2;
		i--;
	}
	if (!i)
		panic("");

	/* remove the breakpoint at the halt address */
	if (ctx->cc->core_clear_break(ctx, halt_addr)
			!= GEAR_ERR_NO_ERROR)
		panic("");
	/* retrieve the error code */
	if (ctx->cc->core_reg_read(ctx, 0, 1 << 0, reg)
			!= GEAR_ERR_NO_ERROR)
		panic("");
	return reg[0];
}

/*
 *
 * exported functions follow
 *
 */

/*! \todo	document this */
enum GEAR_ENGINE_ERR_ENUM target_img_load_elf(struct gear_engine_context * ctx, const char * elf_img_name, int load_flags)
{
static const struct target_mem_map_struct target_mem_map_descs[] = {{
	/* this is the memory map for an lpc2129 */
	0x40000000, 0x00004000,
	0x00000000, 0x00040000
}, },
* const target_mem_map = target_mem_map_descs;
/* target load image related data */
int img_fd;
Elf * img_elf;
Elf32_Ehdr * img_ehdr;
Elf32_Phdr * img_phdr_tab;
/* flash loader image related data */
int flasher_fd;
Elf * flasher_elf;
Elf32_Ehdr * flasher_ehdr;
Elf32_Phdr * flasher_phdr_tab;
/* elf header */
/* elf program header pointer - used to loop through an
 * elf program header table */
Elf32_Phdr * phdr;
int i;
int ram_flag, flash_flag;
struct gear_flash_data * gear_flash_data;

	/*! \todo	properly determine exactly which core is used */
	/* coordinate elf library version */
	if (elf_version(EV_CURRENT) == EV_NONE)
		panic("");
	/* open the target load elf image for reading */
	if ((img_fd = open(elf_img_name, O_RDONLY)) == -1)
		panic("");
	img_elf = elf_begin(img_fd, ELF_C_READ, 0);
	if (!img_elf)
		panic("");
	/* validate target load image elf file */
	img_ehdr = elf32_getehdr(img_elf);
	if (!img_ehdr)
		panic("");

	if (!is_elf_file_supported(img_ehdr))
		panic("");
	/* ok, the elf header seems one that we can handle, now
	 * fetch the program headers */
	img_phdr_tab = elf32_getphdr(img_elf);
	if (!img_phdr_tab)
		panic("");
	/* see if all of the target load image loadable program
	 * segments reside in ram */
	ram_flag = flash_flag = 0;
	phdr = img_phdr_tab;
	/* validate image program segments */
	for (i = 0; i < img_ehdr->e_phnum; i++)
	{
		switch (phdr->p_type)
		{
			case PT_LOAD:
				if (is_elf_segment_in_flash(phdr, target_mem_map_descs + 0))
					flash_flag = 1;
				else if (is_elf_segment_in_ram(phdr, target_mem_map_descs + 0))
					ram_flag = 1;
				else
					/*! \todo	this may be not really
					 * 		an error, handle
					 * 		this gracefully */
					panic("");
				break;
			case PT_NULL:
				break;
			case PT_DYNAMIC: case PT_INTERP: case PT_NOTE:
			case PT_SHLIB: case PT_PHDR: case PT_TLS: case PT_NUM:
			case PT_LOOS: case PT_GNU_EH_FRAME: case PT_GNU_STACK:
			case PT_GNU_RELRO: case PT_LOSUNW:
			/* case PT_SUNWBSS: - same as PT_LOSUNW */
			case PT_SUNWSTACK: /* case PT_HISUNW: - same as PT_HIOS */
			case PT_HIOS: case PT_LOPROC: case PT_HIPROC:
			case PT_ARM_EXIDX:
				panic("");
				break;
			default:
				panic("");
		}
		/* skip to the next program header */
		phdr = (Elf32_Phdr *)((char *) phdr
				+ img_ehdr->e_phentsize);
	}
	if (!ram_flag && !flash_flag)
		panic("");
	/* time to access the target core - install
	 * target state change override callbacks */
	ctx->cc->core_register_target_state_change_callback(ctx, flasher_target_state_change_callback);
	/* see if all of the program segments reside in ram */
	if (!flash_flag)
	{
		if (load_flags & LOAD_RAM)
		{
			phdr = img_phdr_tab;
			for (i = 0; i < img_ehdr->e_phnum; i++)
			{
				if (phdr->p_type == PT_LOAD)
					load_elf_prog_segment_in_ram(ctx, phdr, img_fd);
				/* skip to the next program header */
				phdr = (Elf32_Phdr *)((char *) phdr
						+ img_ehdr->e_phentsize);
			}
		}
		/* cleanup */
		if (ctx->cc->core_unregister_target_state_change_callback(ctx, flasher_target_state_change_callback)
				!= GEAR_ERR_NO_ERROR)
			panic("");
		if (elf_end(img_elf))
			panic("");
		return GEAR_ERR_NO_ERROR;
	}
	/* some of the target load image program segments
	 * reside in flash memory; in order to write these segments
	 * in flash, first load the flash loader in target
	 * memory */

	/* open the flash loader elf image for reading */
	/*! \todo	the flasher elf is hardcoded right now - remedy this */
	if ((flasher_fd = open("xxx", O_RDONLY)) == -1)
		panic("");
	flasher_elf = elf_begin(flasher_fd, ELF_C_READ, 0);
	if (!flasher_elf)
		panic("");
	/* validate flasher elf file */
	flasher_ehdr = elf32_getehdr(flasher_elf);
	if (!flasher_ehdr)
		panic("");

	if (!is_elf_file_supported(flasher_ehdr))
	/* ok, the elf header seems one that we can handle, now
	 * fetch the program headers */
	flasher_phdr_tab = elf32_getphdr(img_elf);
	if (!flasher_phdr_tab)
		panic("");
	/* see if all of the target load image loadable program
	 * segments reside in ram */
	ram_flag = flash_flag = 0;
	phdr = flasher_phdr_tab;
	/* validate flasher program segments */
	for (i = 0; i < flasher_ehdr->e_phnum; i++)
	{
		switch (phdr->p_type)
		{
			case PT_LOAD:
				if (is_elf_segment_in_flash(phdr, target_mem_map_descs + 0))
					flash_flag = 1;
				else if (is_elf_segment_in_ram(phdr, target_mem_map_descs + 0))
					ram_flag = 1;
				else
					/*! \todo	this may be not really
					 * 		an error, handle
					 * 		this gracefully */
					panic("");
				break;
			case PT_NULL:
				break;
			case PT_DYNAMIC: case PT_INTERP: case PT_NOTE:
			case PT_SHLIB: case PT_PHDR: case PT_TLS: case PT_NUM:
			case PT_LOOS: case PT_GNU_EH_FRAME: case PT_GNU_STACK:
			case PT_GNU_RELRO: case PT_LOSUNW:
			/* case PT_SUNWBSS: - same as PT_LOSUNW */
			case PT_SUNWSTACK: /* case PT_HISUNW: - same as PT_HIOS */
			case PT_HIOS: case PT_LOPROC: case PT_HIPROC:
			case PT_ARM_EXIDX:
				panic("");
				break;
			default:
				panic("");
		}
		/* skip to the next program header */
		phdr = (Elf32_Phdr *)((char *) phdr
				+ flasher_ehdr->e_phentsize);
	}
	if (!ram_flag && !flash_flag)
		panic("");
	if (flash_flag)
		panic("");

	phdr = flasher_phdr_tab;
	/* here, both the loaded image elf and the flasher elf
	 * loadable program segments have already been checked that they do
	 * not occupy invalid memory regions */

	for (i = 0; i < flasher_ehdr->e_phnum; i++)
	{
		if (phdr->p_type == PT_LOAD)
			load_elf_prog_segment_in_ram(ctx, phdr, flasher_fd);
		/* skip to the next program header */
		phdr = (Elf32_Phdr *)((char *) phdr
				+ flasher_ehdr->e_phentsize);
	}
	/* retrieve a gear_flash_data data structure from the
	 * flasher elf file */
	gear_flash_data = retrieve_gear_flash_data_from_elf(flasher_elf);
	if (!gear_flash_data)
		panic("");
	/* flasher elf file no longer needed */
	if (elf_end(flasher_elf))
		panic("");

	/* execute the flash loader from its entry point to allow
	 * it to initialize its runtime environment */
	if (exec_flasher(ctx, flasher_ehdr->e_entry,
			gear_flash_data->gear_engine_halt_location, 0, 0))
		/*! \todo	handle errors here */
		panic("");
	/* execute the flash_init() flash loader routine */
	if (exec_flasher(ctx, (ARM_CORE_WORD) gear_flash_data->flash_init,
			gear_flash_data->gear_engine_halt_location, 0, 0))
		/*! \todo	handle errors here */
		panic("");
	/* write all of the image loadable program segments that 
	 * reside in flash to the target */
	phdr = img_phdr_tab;
	for (i = 0; i < img_ehdr->e_phnum; i++)
	{
		if (phdr->p_type == PT_LOAD
				&& is_elf_segment_in_flash(phdr, target_mem_map_descs + 0))
		{
			/* erase flash region */
			panic("");
			//xxx_erase_and_write_image_to_flash();
		}
		/* skip to the next program header */
		phdr = (Elf32_Phdr *)((char *) phdr
				+ img_ehdr->e_phentsize);
	}
	/* execute the flash_shutdown() flash loader routine */
	if (exec_flasher(ctx, (ARM_CORE_WORD) gear_flash_data->flash_shutdown,
			gear_flash_data->gear_engine_halt_location, 0, 0))
		panic("");
	/* flash segments loaded; if loading of the ram segments
	 * is requested, load them as well */
	if (load_flags & LOAD_RAM)
	{
		phdr = img_phdr_tab;
		for (i = 0; i < img_ehdr->e_phnum; i++)
		{
			if (phdr->p_type == PT_LOAD
					&& is_elf_segment_in_ram(phdr, target_mem_map_descs + 0))
				load_elf_prog_segment_in_ram(ctx, phdr, flasher_fd);
			/* skip to the next program header */
			phdr = (Elf32_Phdr *)((char *) phdr
					+ img_ehdr->e_phentsize);
		}
	}
	/* cleanup */
	if (ctx->cc->core_unregister_target_state_change_callback(ctx, flasher_target_state_change_callback)
			!= GEAR_ERR_NO_ERROR)
		panic("");
	if (elf_end(img_elf))
		panic("");
	return GEAR_ERR_NO_ERROR;
}

void init_img_load(struct gear_engine_context * ctx)
{
	if (!(ctx->img_loader_data = malloc(sizeof * ctx->img_loader_data)))
		panic("");
}


