/*!
 *	\file	gprintf.h
 *	\brief	gear engine general purpose (non-machine interface) printer header
 *	\author	shopov
 *
 *	the gprintf routine is used for general purpose output
 *	throughout the gear engine - status output, logging purposes
 *	debugging output, etc.; this is actually a simple substitute
 *	for 'printf' itself and the purpose of having 'gprintf' is
 *	to have more flexibility and control on redirecting the
 *	gear engine output
 *
 *	gprintf should be used whenever printf is intended to be used;
 *	gprintf can always be thought of as a functional equivalent
 *	of printf, the prototypes of printf and gprintf are the same
 *	and the manual page for printf is valid (in a functional sense)
 *	for gprintf as well - unspecified is only where the actual
 *	output from gprintf is directed (e.g. a file, the standard output, etc.) 
 *
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

int gprintf(const char * format, ...);
int vgprintf(const char * format, va_list ap);

