/*!
 *	\file	util.h
 *	\brief	utility routines and macros
 *	\author	shopov
 *
 *	Revision summary:
 *
 *	$Log: $
 */


/*
 *
 * exported macros follow
 *
 */
#include <stdio.h>

#define panic(__x)	do { gprintf("%s(), %i: %s\n", __FILE__, __LINE__, __x); fflush(stdout); fflush(stderr); exit(1); } while (0)
#define TRACE()		do { gprintf("TRACE: %s(), %i\n", __FILE__, __LINE__); fflush(stdout); fflush(stderr); } while (0)
