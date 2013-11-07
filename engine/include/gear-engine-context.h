/*!
 *	\file	gear-engine-context.h
 *	\brief	main gear engine context data structure declaration
 *	\author	shopov
 *
 *	here, declared is the main gear engine target access context
 *	holding data structure; the primary rationale for the need
 *	of such a structure is multiple physical cores support; this
 *	is not entirely contrived, in my experience at least; the
 *	gear is targeted to embedded arm processor core targets,
 *	and in my experience, there have been several
 *	occasions on which a debugger capable of exercising a
 *	simultaneous control over several physical processor cores
 *	would have been helpful
 *
 *	basically, the context data structure holds everything related
 *	to debugging an executable image on a target core - e.g.
 *	symbol tables, target access channels, frontend channels,
 *	target execution states, etc.
 *
 *	if anyone thinks it would have been a better idea to code the
 *	entire gear engine in c++ in the first place, (s)he would be
 *	darn right; the reason why i(sgs) started coding this in c is
 *	that i do not know c++; i, however, understand the importance
 *	of c++, and that is why i started coding the sample frontend
 *	in c++ - this seemed like the best way to learn the
 *	beast at last - c++ must be supported in the long run anyway,
 *	which would not be very possible if i(sgs) am not familiar with
 *	it; in retrospect, i can say this has been a good
 *	decision - c++ did save a lot of time and helped much in
 *	defining the architecture of the frontend; i(sgs), however,
 *	do not think that coding the gear engine in c is a major
 *	fallacy of its design; i(sgs) have tried to keep the structure
 *	of the gear engine as modular as i can, and also - there
 *	is perhaps one more reason to code the gear engine in c -
 *	this way, it would be probably accessible by a larger
 *	number of people; one more notice of insight to the use and
 *	purpose of the gear engine context structure - if you imagine
 *	the gear was coded in c++, this would have been the implicit
 *	'this' pointer passed to many of the modules in the gear,
 *	used to discriminate between different physical cores being
 *	controlled by the gear
 *
 *	\note	many of the members of the gear engine context
 *		data structure are deliberately opaque (pointers
 *		to undefined data structures) - in case
 *		you look for them in a header file, you wont
 *		find them in one; that is intentionally done
 *		so that the opaque members are accessible only
 *		by code that is indeed intended (and authorized,
 *		by design) to operate on them and are thusly
 *		declared in the corresponding source files; so,
 *		in case you are interested in what one such member
 *		represents, you should look for the declarations in the
 *		c source files that ("logically" and (hopefully)
 *		"naturally") correspond to these members (e.g.,
 *		to review 'struct type_hash', you should look
 *		into source file 'type-access.c';
 *		the correspondence should be readily evident from
 *		the comments herein); good luck
 *
 *	\todo	transit to usage of the herein supplied gprintf/miprint
 *		output routines only; rename the currently stock
 *		gprintf/miprintf and corresponding files to
 *		something like gprintf-core/miprintf-core - this
 *		would also have the advantage of easing the process
 *		of getting rid of the calls to printf/gprintf
 *		thruout the code
 *
 *	Revision summary:
 *
 *	$Log: $
 */

#ifndef __GEAR_ENGINE_CONTEXT_H__
#define __GEAR_ENGINE_CONTEXT_H__


/*
 *
 * include section follows
 *
 */
#include "target-defs.h"

/*
 *
 * exported data types follow
 *
 */

/*! the main gear engine context data structure */
struct gear_engine_context
{
	/*! settings associated with this context instance */
	struct
	{
		/*! the port on which the gear engine listens for incoming connections */
		unsigned short server_port_nr;
		/*! the port to which to attempt connection to a target core controller */
		unsigned short target_ctl_port_nr;
		/*! the file name of the executable to debug */
		const char * dbg_process_disk_file_name;
	}
	settings;
	/*! the file name of the elf-format file from which to retrieve debug information about the process to debug
	 *
	 * \note	on systems where elf is the native executable
	 *		file format (e.g. linux), this file and
	 *		the one above (dbg_process_disk_file_name)
	 *		may actually (but not necessarily) be the
	 *		same file; for other systems (e.g. windows),
	 *		where the native executable file format is
	 *		not elf, but it is still possible to convert
	 *		the executable file from the non-elf format to
	 *		an elf file for the purposes of retrieving dwarf
	 *		debug information - the files will necessarily
	 *		differ; for example, on windows systems, it is
	 *		possible to convert a gcc/ld-generated executable
	 *		file from the windows native 'pe' file format to
	 *		an elf file which contains all the information
	 *		needed for debugging (i.e. the .debug_* sections
	 *		which contain the debug information); the
	 *		conversion to an elf file on these systems could
	 *		be achieved, for example, by directly employing
	 *		the bfd library, or indirectly - by invoking
	 *		some external program to do the job; the gear
	 *		engine currently employs the latter approach
	 *		by invoking:
	 *		'i386-mingw32-objcopy --output-target elf32-i386 infile.exe outfile.elf'
	 */
	char * dbg_info_elf_disk_file_name;
	/*! the file descriptor of the opened elf file described above (dbg_info_elf_disk_file_name)
	 *
	 * this is needed in order to call various functions
	 * from libdwarf and libelf, and can also be used
	 * for directly accessing the elf executable file (e.g. for
	 * reading program sections, segments, symbols, strings, etc.)
	 *
	 * \note	this file descriptor equals -1 if the file is not
	 *		open or invalid; in case the file descriptor
	 *		is valid (i.e. it is not equal to -1),
	 *		it should have been opened in read-only
	 *		mode, and therefore any write attempts to
	 *		the file will fail */
	int	dbg_elf_fd;
	/*! the elf descriptor as returned by libelf when invoking elf_begin() for the file descriptor above (dbg_elf_fd) */
	Elf	* libelf_elf_desc;
	/*! dwarf debug information access descriptor
	 *
	 * this is returned by a successfull call to
	 * dwarf_init(), from the libdwarf library, and
	 * is necessary for performing many of the
	 * queries to different debugging information
	 * entities in the file containing debug
	 * information (most commonly, the executable
	 * elf file being debugged */
	Dwarf_Debug		dbg;
	/*! data structure for access to the target
	 *
	 * this is the pointer to the core control
	 * data structure which is to be used for
	 * controlling the target and operating onto
	 * target resources */
	struct core_control	* cc;
	/*! target/abi specific parameter access data structure
	 *
	 * for details, see the comments in file target-description.h */
	struct target_desc_struct * tdesc;
	/*! dump a c-like formatted (escaped) string from target memory
	 *
	 * if nonzero, this pointer specifies a routine that may
	 * be used for dumping c-like string literals (properly
	 * escaped, without enclosing double quotes) from target
	 * memory, the string is to be found at the address passed
	 * as the 'addr' parameter; it is assumed that this function
	 * will dump the string by invoking the 'miprintf()'
	 * function to directly print to a gear engine frontend;
	 * it is also assumed that this function will not attempt
	 * to dump any more than MAX_DUMPED_CSTRLEN_FROM_TARGET_MEM
	 * (defined in file gear-limits.h) number of characters;
	 * this routine is mainly useful for dumping strings
	 * for a human user for a better debugging experience... */
	void (* dump_cstring_from_target_mem)(struct gear_engine_context * ctx, ARM_CORE_WORD addr);
	/*! a pointer to the general purpose printing routine for this context
	 *
	 * see the comments in gprintf-core.h for details */
	int (* gprintf)(const char * format, ...);
	/*! a pointer to the machine interface printing routine for this context
	 *
	 * see the comments in miprintf-core.h for details */
	int (* miprintf)(const char * format, ...);
	/*! debuggee data types hash table pointer; opaque
	 *
	 * for details, see the comments in type-access.c */
	struct type_hash	* types;
	/*! debuggee compilation units hash table pointer; opaque
	 *
	 * for details, see the comments in cu-access.c */
	struct cu_hash		* cus;
	/*! the debuggee symbol hash table; opaque
	 *
	 * see the comments in symtab.c for details */
	struct symtab_hash_table	* symtab;
	/*! source file information for the executable; opaque
	 *
	 * see the comments in srcfile.c for details */
	struct src_data		* src_data;
	/*! the compilation units address range table; opaque
	 *
	 * for details, see file aranges_access.c */
	struct cu_aranges	* cu_aranges;
	/*! breakpoint related data
	 *
	 * for details, see file breakpoint.c */
	struct bkpt_struct	* bkpts;
	/*! executor related data
	 *
	 * for details, see file exec.c */
	struct exec_data_struct	* exec_data;
	/*! target core controller communication data
	 *
	 * for details, see module target-comm.c */
	struct core_connection_data	* core_comm;
	/*! target image loader data
	 *
	 * for details, see file target-img-load.c */
	struct target_img_load_data	* img_loader_data;
	/*! target call stack frame unwind data
	 *
	 * for details, see file frame-reg-cache.c */
	struct frame_data_struct	* frame_data;
};

#endif /* __GEAR_ENGINE_CONTEXT_H__ */

