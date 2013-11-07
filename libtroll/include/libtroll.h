#ifndef __LIBTROLL_H__
#define __LIBTROLL_H__

#include <stdbool.h>
#include <stdint.h>

#include "engine-common.h"
#include "err.h"

enum TROLL_RECORD_ENUM
{
	TROLL_RECORD_INVALID = 0,
	TROLL_RECORD_BREAKPOINT,
	TROLL_RECORD_DISASSEMBLY_LIST,
	TROLL_RECORD_VARDISPLAY,
	TROLL_RECORD_MEMDUMP,
};

struct parse_type_common_struct
{
	enum TROLL_RECORD_ENUM	type;
	struct gear_engine_err_struct 		err;
/*! \todo	this does not belong here */
#define NR_MACHINE_INTERFACE_TOKENS		2
	unsigned long int	tokens[NR_MACHINE_INTERFACE_TOKENS];
};

#include <disasm-types.h>
#include <stackframe-types.h>
#include <bkpt-types.h>
#include <srcinfo.h>
#include <vardisplay-types.h>
#include <memdump-types.h>

typedef struct Troll_t
{
	bool	is_parse_error;
	int	lexpos;
}
* Troll;


struct parse_type_common_struct * troll_parse(Troll htroll, const char * str);
void * troll_clone(const void * src);
void * troll_allocate_record(enum TROLL_RECORD_ENUM type);
void troll_deallocate_record(void * record);
void troll_clone_parse_type_common_struct(struct parse_type_common_struct * dest, struct parse_type_common_struct * src);
/* returns nonzero on error */
int troll_init(void);
/* returns nonzero on error */
int troll_shutdown(void);

#endif

