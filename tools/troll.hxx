
#include <libelf.h>
#include <dwarf.h>
#include <libdwarf.h>

class elf_dwarf_troll_class
{
	public:
struct dwarf_attr_struct
{
	struct dwarf_attr_struct * next;
	Dwarf_Half	attr;
	Dwarf_Half	form;
	union
	{
		/* DW_FORM_ref_addr */
		Dwarf_Off	global_ref;
		/* DW_FORM_addr */
		Dwarf_Addr	addr;
		Dwarf_Bool	flag;
		Dwarf_Unsigned	udata;
		Dwarf_Signed	sdata;
		Dwarf_Block	* block;
		char		* str;
		struct
		{
			Dwarf_Locdesc	** llbuf;
			Dwarf_Signed	listlen;
		};
	};
};

struct dwarf_die_struct
{
	struct elf_dwarf_troll_class::dwarf_die_struct * get_sibling(void) { return next_sibling; }
	struct elf_dwarf_troll_class::dwarf_die_struct * get_first_child(void) { return children; }
	struct elf_dwarf_troll_class::dwarf_attr_struct * get_attrlist(void) { return attrlist; }
	Dwarf_Half get_tag(void) { return tag; }
private:
	friend class elf_dwarf_troll_class;

	Dwarf_Half	tag;
	Dwarf_Die	die;
	Dwarf_Off	offset_in_debug_info;
	Dwarf_Off	cu_rel_offset;
	struct dwarf_attr_struct * attrlist;
	struct dwarf_die_struct * children, * next_sibling;
};
private:
/* the libdwarf handle used to access the debugging
 * information in the underlying elf file */
Dwarf_Debug dbg;
/* the debug information entries (die-s) tree root */
struct dwarf_die_struct * root;

	struct elf_dwarf_troll_class::dwarf_attr_struct * fill_dw_attr_node(Dwarf_Attribute attr);
	struct elf_dwarf_troll_class::dwarf_die_struct * build_die_tree(Dwarf_Debug dbg, Dwarf_Die die);
	const char * dwarf_util_get_attr_name(Dwarf_Half attrval);
	static const char * dwarf_util_get_tag_name(Dwarf_Half tag);
	void dump_dwarf_attr(struct dwarf_attr_struct * attr);
	void dump_dwarf_tree(int nesting_level, struct dwarf_die_struct * root);

public:
	struct elf_dwarf_troll_class::dwarf_die_struct * get_root(void) { return root; }
	void print_tree(void);
	elf_dwarf_troll_class(const char * elf_file_name);
};

