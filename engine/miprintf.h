/*!
 *	\file	miprintf.h
 *	\brief	gear engine machine interface printer header
 *	\author	shopov
 *
 *	the miprintf routine is used for machine interface purpose output
 *	throughout the gear engine - this is output intended for
 *	interpretation by another software agent and it is
 *	assumed that the format of this output is such that
 *	it is agreed onto (can be regarded as a defacto gear engine
 *	output format specification documented elsewhere) and is easy to
 *	be routinely interpreted (perhaps via automatically generated
 *	scanner/parser pairs); this output is generally structured
 *	along the lines of the machine interface (mi) output format
 *	and doctrine of the one true and great gnu debugger (gdb);
 *	the output is generally ascii and not binary; some of the
 *	rationales for this are:
 *		- if the output is in ascii, it is still not too
 *		difficult to interpret by humans without additional
 *		preprocessing
 *		- in an environment where the gear engine is
 *		intended to be used as some small component of
 *		a larger system, it is perhaps easier to write
 *		scanner(lexer)/parser pairs for the gear engine
 *		output; it is also perhaps easier to debug them
 *		- it is easier in the engine itself to produce the
 *		output this way; it is also easier to debug it and
 *		does not impose to stringent requirements on all
 *		of the output formatting (e.g. whitespace)
 *		- in the gear engine, the output generators can
 *		be easily and clearly coded as simple 'printf'
 *		style printers, and, indeed, the function prototype
 *		for the backbone 'miprintf' function is the same
 *		as 'printf' - this way miprintf can be thought of
 *		as a functional equivalent of printf and the manual
 *		page for printf is valid (in a functional sense)
 *		for miprintf as well - unspecified is only where
 *		the actual output from miprintf is directed
 *		(this is generally sent through some communication
 *		channel - e.g. a socket, a pipe, etc. - to the
 *		machine interface output consumer)
 *	this being said, it should be also noted that a common
 *	example of a gear engine machine interface consumer is a
 *	debugger front end - this is the most common case
 *
 *	\todo	provide a link to the machine output format
 *		specification document, when one is ready
 *
 *	Revision summary:
 *
 *	$Log: $
 */


/*
 *
 * exported function declarations follow
 *
 */

#include <stdbool.h>

void miprintf_switch_debug(bool is_debug_print_enabled);
int miprintf(const char * format, ...);
int vmiprintf(const char * format, va_list ap);

