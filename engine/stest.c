


#include <unistd.h>
#if __LINUX__
#include <sys/socket.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#else
#define _WIN32_WINNT	0x0501
#include <windows.h>
#include <wincon.h>
#include <winsock2.h>
#include <winuser.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <ctype.h>
#include <string.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>




extern "C"
{
#include "dwarf-common.h"

/*! \todo document this */
struct cu_hash
{
	/*! the die offset for the described type */
	Dwarf_Off		die_offset;
	/*! pointer to the data type tree built for the type */
	struct cu_data		* cu_data;
	/*! link pointer for the hash table */
	struct cu_hash		* next;
};



#include "target-defs.h"
#include "gear-constants.h"
#include "gear-engine-context.h"
#include "engine-err.h"
#include "core-access.h"
#include "dwarf-expr.h"
#include "dwarf-loc.h"
#include "gear.h"
#include "arm-dis.h"
#include "symtab.h"
#include "dobj-access.h"
#include "type-access.h"
#include "aranges-access.h"
#include "util.h"
#include "srcfile.h"
#include "exec.h"
#include "breakpoint.h"
#include "dbx-support.h"
#include "target-comm.h"
#include "frame-reg-cache.h"
#include "target-dump-cstring.h"

#include "cu-access.h"
#include "subprogram-access.h"
#include "dwarf-ranges.h"

#include "srcinfo.h"

#include "gprintf.h"

extern void dwarf_hacked_init(struct gear_engine_context * ctx);
extern struct srcinfo_type_struct * srcfile_get_srcinfo(struct gear_engine_context * ctx);

static struct gear_engine_context ctx;


#include "c-parse-types.h"
extern int yyparse(void);

extern struct expr * parse_head;

};

#include "c-expr-eval.hxx"

#include <iostream>


static struct core_control target_cc =
{
	0, //.is_connected = 0,
	0, //.core_open = 0,
	0,
	0, //.core_mem_read = 0,
	0, //.core_mem_write = 0,
	0, //.core_reg_read = 0,
	0, //.core_reg_write = 0,
	0,
	0,
	0, //.core_set_break = 0,
	0, //.core_clear_break = 0,
	0, //.core_run = 0,
	0, //.core_halt = 0,
	0, //.core_insn_step = 0,
	0, //.io_ctl = 0,
	0, //.core_get_status = 0,
	0, //.core_register_target_state_change_callback = 0,
	0, //.core_unregister_target_state_change_callback = 0,
};

static void dump_data_struct(std::tr1::shared_ptr<struct data_contents> data)
{
	if (data->children.empty())
	{
		std::cout << data->type << "\t" << data->value << std::endl;
	}
	else
	{
		int i;
		for (i = 0; i < data->children.size(); i ++)
		{
			std::cout << data->children[i]->designator << "\t<designator end>\t";
			dump_data_struct(data->children[i]);
		}
	}
}

#if TROLL
struct srcinfo_type_struct * stest(int argc, char ** argv)
#elif TYPE_DUMP

std::tr1::shared_ptr<struct data_contents> stest (int argc, char ** argv)

#else

int main(int argc, char ** argv)
#endif
{
struct srcinfo_type_struct * srcinfo;
struct cu_data * cu;
struct cu_hash * cuhs;
struct subprogram_data * subp;
std::tr1::shared_ptr<struct data_contents> data;

	memset(& ctx, 0, sizeof ctx);
	ctx.cc = & target_cc;

	init_types(& ctx);
	init_symtab(& ctx);

	ctx.dbg_info_elf_disk_file_name = strdup((argc == 2) ? argv[1] : ((const char * []){"vx2.elf", "c:/shopov/src/stm0/mycc.elf", "e:/xstall/stm0/mycc.elf"}[2]));
	dwarf_hacked_init(& ctx);

	gprintf("initialization complete\n");
	gprintf("dtype_data usage counts:\n");
	gprintf("nr_base_type_nodes == %i\n", type_access_stats.nr_base_type_nodes);
	gprintf("nr_typedef_nodes == %i\n", type_access_stats.nr_typedef_nodes);
	gprintf("nr_tqual_nodes == %i\n", type_access_stats.nr_tqual_nodes);
	gprintf("nr_arr_type_nodes == %i\n", type_access_stats.nr_arr_type_nodes);
	gprintf("nr_ptr_nodes == %i\n", type_access_stats.nr_ptr_nodes);
	gprintf("nr_struct_nodes == %i\n", type_access_stats.nr_struct_nodes);
	gprintf("nr_union_nodes == %i\n", type_access_stats.nr_union_nodes);
	gprintf("nr_member_nodes == %i\n", type_access_stats.nr_member_nodes);
	gprintf("nr_enumerator_nodes == %i\n", type_access_stats.nr_enumerator_nodes);
	gprintf("nr_enumeration_nodes == %i\n", type_access_stats.nr_enumeration_nodes);
	gprintf("nr_dobj_nodes == %i\n", type_access_stats.nr_dobj_nodes);
	gprintf("nr_subprogram_nodes == %i\n", type_access_stats.nr_subprogram_nodes);
	gprintf("nr_lexblocks == %i\n", type_access_stats.nr_lexblocks);
	gprintf("nr_subprogram_prototype_nodes == %i\n", type_access_stats.nr_subprogram_prototype_nodes);
	gprintf("nr_symtab_nodes == %i\n", type_access_stats.nr_symtab_nodes);


	srcfile_build_src_cu_tab(& ctx);

	srcinfo = srcfile_get_srcinfo(& ctx);

	init_aranges(& ctx);
	/* dump zero-length subprogram names */
	printf("list of compilation units:\n");
	for (cuhs = ctx.cus; cuhs; cuhs = cuhs->next)
		printf("%s\n", cuhs->cu_data->name);
	for (cuhs = ctx.cus; cuhs; cuhs = cuhs->next)
	{
		cu = cuhs->cu_data;
		printf("compilation unit %s\n", cu->name);
		for (subp = cu->subs; subp; subp = subp->sib_ptr)
		{
			printf("\tsubprogram %s\n", subp->name);
			if (subp->is_abstract_inline_instance_root)
				printf("\t\tabstract inline instance root\n");
			if (!strcmp(subp->name, "PowerOff"))
				printf("!!! at poweroff !!!\n");

			if (subp->addr_ranges && dwarf_ranges_get_range_count(& ctx, subp->addr_ranges) == 1)
			{
				ARM_CORE_WORD lo, hi;
				dwarf_ranges_get_range_data_at_idx(& ctx, subp->addr_ranges, 0, cu->default_cu_base_address, & lo, & hi);
				if (lo == hi)
					printf("\t\tSUBPROGRAM SCHEDULED FOR REMOVAL (EMPTY ADDRESS RANGE)!!!\n");

			}
		}
	}
	{
		struct cu_info_struct * comp_units = srcinfo->comp_units;
		struct srclist_type_struct * srclist;
		for (srclist = srcinfo->srclist; srclist; srclist = srclist->next)
		{
			struct subprogram_type_struct * subs = srclist->subprogram_arr;
			int i;
			struct srcline_addr_pair_struct * slines = srclist->srcaddr_pairs;
			for (i = 0; i < srclist->srcaddr_len - 1; i ++)
			{
				if (slines[i].srcline_nr > 0 && slines[i].srcline_nr > 0
						&& slines[i].addr && slines[i].addr == slines[i + 1].addr)
				{
					printf("----------------------------------------\n");
					printf("address 0x%08x in sourcefile %s corresponds to multiple source code lines\n",
							slines[i].addr, srclist->srcname);
					printf("first entry: compilation unit %s, subprogram %s, line number %i\n",
							comp_units[slines[i].cuname_idx].name,
							subs[slines[i].subarr_idx].name,
							slines[i].srcline_nr);
					printf("second entry: compilation unit %s, subprogram %s, line number %i\n",
							comp_units[slines[i + 1].cuname_idx].name,
							subs[slines[i + 1].subarr_idx].name,
							slines[i + 1].srcline_nr);
				}
			}
		}
	}
#if !TYPE_DUMP
	printf("type an expression to parse:\n");
	if (yyparse())
	{
		printf("syntax error parsing expression\n");
	}
	else
	{
		printf("ok\n");
		class c_expr_eval x(& ctx);

		printf("trying to evaluate expression type...\n");
		try
		{
			struct eval_node n;
			//n = x.evaluate_type(parse_head);
			n = x.compute_value(parse_head);
			x.debug_dump_value(n);
			data = x.debug_dump_value_1(n);
			type_data_dump(n.dtype, 0, 0, 0, 0);

			std::cout << "----------------------------------" << std::endl;
			std::cout << "----------------------------------" << std::endl;
			std::cout << "----------------------------------" << std::endl;

			dump_data_struct(data);
		}
		catch (std::string errmsg)
		{
			std::cout << "expression evaluation error: " << errmsg << "\n";
		}
	}
#endif

#if TROLL
	return (int) srcinfo;
#elif TYPE_DUMP
	return data;
#else
	return 0;
#endif
}

struct gear_engine_context * get_gear_ctx(void) { return & ctx; }
