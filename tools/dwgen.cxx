#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <gelf.h>
#include <stdio.h>
#include <error.h>
#include <string.h>
#include <malloc.h>

#include <../../dwarf-20090716/libdwarf/dwarf.h>
#include <../../dwarf-20090716/libdwarf/libdwarf.h>
#include <map>
#include <string>

using namespace std;

#include "troll.hxx"

#define panic(_) do { printf("%s(), %i: %s\n", __func__, __LINE__, _); while(1); } while(0)

struct elf_scn_data
{
	char	* name;
	int	size;
	Dwarf_Unsigned	type;
	Dwarf_Unsigned	flags;
	Dwarf_Unsigned	link;
	Dwarf_Unsigned	info;
	/* set when actually writing the dwarf sections
	 * to the output elf via libelf */
	Elf_Scn * dwsec;
};

static map<int, struct elf_scn_data *> dwsectab;
static int last_dwarf_sectab_idx = 0;

static int dbg_section_create_callback(
    char*           name, 
    int             size, 
    Dwarf_Unsigned  type,
    Dwarf_Unsigned  flags, 
    Dwarf_Unsigned  link, 
    Dwarf_Unsigned  info, 
    /* unused - applicable only when handling relocations */
    Dwarf_Unsigned* sect_name_index, 
    int*            error)
{
struct elf_scn_data * p = new struct elf_scn_data;

	p->name = name;
	p->size = size;
	p->type = type;
	p->flags = flags;
	p->link = link;
	p->info = info;
	printf("creating section %s\n", name);
	dwsectab[last_dwarf_sectab_idx] = p;
	*sect_name_index = last_dwarf_sectab_idx;
	return last_dwarf_sectab_idx++;

}

static void libdwarf_err_handler(Dwarf_Error errnum, Dwarf_Ptr errarg) 
{
	error(1, 0, "libdwarf error\n");
}



static Dwarf_P_Debug pdbg;

void output_attr(Dwarf_P_Die owner_die, struct elf_dwarf_troll_class::dwarf_attr_struct * attr)
{
Dwarf_Error err;
Dwarf_P_Attribute pattr;
Dwarf_Half attrval;

	attrval = attr->attr;
	/*! \todo	properly handle invalid attribute values */
	if (attrval == 0x2020
			|| attrval == DW_AT_stmt_list
			|| attrval == DW_AT_encoding
			)
	{
		return;
	}

	/* filter out attributes */
	switch (attr->attr)
	{
		case DW_AT_location:
		case DW_AT_string_length:
		case DW_AT_return_addr:
		case DW_AT_data_member_location:
		case DW_AT_frame_base:
		case DW_AT_segment:
		case DW_AT_static_link:
		case DW_AT_use_location:
		case DW_AT_vtable_elem_location:
			return;
	}

	/* build the attribute based on its encoding form */
	switch (attr->form)
	{
		case DW_FORM_addr:
			if (dwarf_add_AT_targ_address_b(pdbg,
						owner_die,
						attrval,
						(Dwarf_Unsigned) attr->addr,
						0 /* unused - output is expected
							* to not need/undergo
							* section relocations */,
						&err) == (Dwarf_P_Attribute) DW_DLV_BADADDR)
				panic("");
			break;
		case DW_FORM_block:
		case DW_FORM_block1:
		case DW_FORM_block2:
		case DW_FORM_block4:
				panic("");
			break;
		case DW_FORM_data1:
		case DW_FORM_data2:
		case DW_FORM_data4:
		case DW_FORM_data8:
		case DW_FORM_udata:
			if (dwarf_add_AT_unsigned_const(pdbg,
						owner_die,
						attrval,
						attr->udata,
						&err) == (Dwarf_P_Attribute) DW_DLV_BADADDR)
			{
				printf("attr: %i\n", (int) attrval);
				panic("");
			}
			break;
		case DW_FORM_sdata:
			if (dwarf_add_AT_signed_const(pdbg,
						owner_die,
						attrval,
						attr->sdata,
						&err) == (Dwarf_P_Attribute) DW_DLV_BADADDR)
			{
				printf("attr: %i\n", (int) attrval);
				panic("");
			}
			break;
		case DW_FORM_string:
		case DW_FORM_strp:
			if (dwarf_add_AT_string(pdbg,
						owner_die,
						attrval,
						attr->str,
						&err) == (Dwarf_P_Attribute) DW_DLV_BADADDR)
				panic("");
			break;
		case DW_FORM_flag:
			if (dwarf_add_AT_flag(pdbg,
						owner_die,
						attrval,
						(Dwarf_Small) attr->flag,
						&err) == (Dwarf_P_Attribute) DW_DLV_BADADDR)
				panic("");
			break;
		case DW_FORM_ref_addr:
		case DW_FORM_ref1:
		case DW_FORM_ref2:
		case DW_FORM_ref4:
		case DW_FORM_ref8:
		case DW_FORM_ref_udata:
			/*! \todo	add code */
			break;

		default:
		case DW_FORM_indirect:
		case DW_FORM_sec_offset:
		case DW_FORM_exprloc:
		case DW_FORM_flag_present:
			panic("");
	}
}

Dwarf_P_Die output_die(Dwarf_P_Die parent, struct elf_dwarf_troll_class::dwarf_die_struct * die)
{
Dwarf_P_Die pdie;
Dwarf_Error err;
struct elf_dwarf_troll_class::dwarf_attr_struct * attr;
struct elf_dwarf_troll_class::dwarf_die_struct * child;

	if ((pdie = dwarf_new_die(pdbg,
				die->get_tag(),
				parent, 0, 0, 0, &err))
			== (Dwarf_P_Die) DW_DLV_BADADDR)
		panic("");
	/* dump the attributes */
	attr = die->get_attrlist();
	while (attr)
	{
		output_attr(pdie, attr);
		attr = attr->next;
	}
	/* dump children */
	child = die->get_first_child();
	while (child)
	{
		output_die(pdie, child);
		child = child->get_sibling();
	}
	return pdie;
}


static void create_dwarf_sections(void)
{
Dwarf_Error err;
struct elf_dwarf_troll_class::dwarf_die_struct * die;
struct elf_dwarf_troll_class troll("test.elf");
Dwarf_P_Die pdie;

int i = 0;
	if ((pdbg = dwarf_producer_init_b(DW_DLC_WRITE/* | DW_DLC_SIZE_32 | DW_DLC_TARGET_LITTLEENDIAN*/,
			dbg_section_create_callback,
			libdwarf_err_handler,
			0,
			&err)) == (Dwarf_P_Debug) DW_DLV_BADADDR)
		panic("");
	die = troll.get_root();
	if (die->get_tag() == DW_TAG_compile_unit)
	if (die->get_tag() != DW_TAG_compile_unit)
		panic("");
	pdie = output_die(0, die);
	if (dwarf_add_die_to_debug(pdbg, pdie, &err)
		== DW_DLV_NOCOUNT)
		panic("");
	die = die->get_sibling();
	while (die)
	{
		Dwarf_P_Die p;
		if (dwarf_die_link(p = output_die(0, die), 0, 0, pdie, 0, &err)
				== (Dwarf_P_Die) DW_DLV_BADADDR)
			panic("");
		pdie = p;
		die = die->get_sibling();
		printf("%i\n", i++);
	}
}

int main(void)
{
int in_fd, out_fd;
Elf * inelf, * outelf;
GElf_Ehdr in_ehdr, * out_ehdr;
GElf_Phdr in_phdr, * out_phdr;
GElf_Shdr in_shdr;
Elf_Scn * in_sec, * out_sec, * out_strtab_sec;
Elf_Data * in_sec_data, * out_sec_data;
size_t nr_sections, nr_segments;
int i, j, out_strtab_idx;
int elf_class;
const char * strtab;
Dwarf_Error err;

	create_dwarf_sections();
	/* open the input and output elf files */
	if ((in_fd = open("in.elf", O_RDONLY)) == -1)
		panic("");
	if ((out_fd = open("out.elf", O_RDWR | O_CREAT | O_TRUNC, 0666)) == -1)
		panic("");

	/* coordinate elf version */
	if (elf_version(EV_CURRENT) == EV_NONE)
		panic("");
	inelf = elf_begin(in_fd, ELF_C_READ, 0);
	if (!inelf)
		panic("");
	outelf = elf_begin(out_fd, ELF_C_WRITE, 0);
	if (!outelf)
		panic("");
	/* retrieve the elf class (32 or 64 bit)
	 * and use it from now on for class dependent
	 * operations */
	elf_class = gelf_getclass(inelf);
	if (elf_class == ELFCLASSNONE)
		panic("");
	/* retrieve the elf file header of the input elf file */
	if (!gelf_getehdr(inelf, &in_ehdr))
		panic("");
	/*
	if (!gelf_getphdr(inelf, &in_phdr))
		panic("");
		*/
	if (elf_getshnum(inelf, &nr_sections) == -1)
		panic("");
	/* create a new elf header and a program header
	 * for the output file */
	if (!(out_ehdr = (GElf_Ehdr *) gelf_newehdr(outelf, elf_class)))
		panic("");
	/* copy program headers */
	nr_segments = in_ehdr.e_phnum;

	/* create destination program header table */
	if (!(out_phdr = (GElf_Phdr *) gelf_newphdr(outelf, nr_segments)))
		panic("");
	for (i = 0; i < nr_segments; i++)
	{
		if (!gelf_getphdr(inelf, i, &in_phdr))
			panic("");
		if (gelf_update_phdr(outelf, i, &in_phdr) == -1)
			panic("");
	}
	/* obtain a pointer to the string table - used
	 * for querying section names */
	if ((i = in_ehdr.e_shstrndx)
			>= nr_sections)
		panic("");
	/* retrieve string table section */
	if (!(in_sec = elf_getscn(inelf, i)))
		panic("");
	/* get a pointer to the string table data */
	if (!(in_sec_data = elf_rawdata(in_sec,
		/* request first data buffer */0)))
		panic("");
	strtab = (const char *) in_sec_data->d_buf;
	/* this index is an offset in the destination
	 * elf file .strtab section and is used when
	 * putting in the names of the newly created
	 * dwarf sections */
	out_strtab_idx = in_sec_data->d_size;

	/* copy sections */
	for (j = i = /* skip the (mandatory) undefined
		  * section - it will be created automatically
		  * by libelf */ 1; i < nr_sections; i++)
	{
		/* retrieve the section to copy data from */
		if (!(in_sec = elf_getscn(inelf, i)))
			panic("");
		if (!(gelf_getshdr(in_sec, &in_shdr)))
			panic("");
		/* dump section name - debugging */
		/*! \todo	make sure the strtab index is within bounds */
		printf("processing section %s...", strtab + in_shdr.sh_name);
		/* see if this is a dwarf debugging section, and
		 * if so - skip it; dwarf sections will be
		 * created below with the assistance of libdwarf */
		if (strstr(strtab + in_shdr.sh_name, "debug"))
		{
			printf("dwarf section detected; skipping...\n");
			continue;
		}
		printf("\n");

		/* create the destination section */
		if (!(out_sec = elf_newscn(outelf)))
			panic("");
		if (gelf_update_shdr(out_sec, &in_shdr) == -1)
			panic("");
		if (!(in_sec_data = elf_rawdata(in_sec,
			/* request first data buffer */0)))
			panic("");
		if (!(out_sec_data = elf_newdata(out_sec)))
			panic("");
		* out_sec_data = * in_sec_data;
		/* if the .strtab is being copied - stash some data
		 * handles for it in order to add the names of the dwarf
		 * sections created below (after all sections are
		 * copied, with the help of libdwarf) */
		if (/* this is .strtab */ i == in_ehdr.e_shstrndx)
		{
			/* adjust strtab section index in the output elf header */
			printf("out strtab idx %i\n", j);
			in_ehdr.e_shstrndx = j;
			out_strtab_sec = out_sec;
		}
		/* update output file section index */
		j++;
		if (in_sec_data = elf_rawdata(in_sec, in_sec_data))
			panic("");

	}
	/* create and write the dwarf sections, that is,
	 * the fresh dwarf sections built by libdwarf */
	if ((j = dwarf_transform_to_disk_form(pdbg, &err))
			== DW_DLV_BADADDR)
		panic("");

	for (i = 0; i < j; i++)
	{
		GElf_Shdr dwshdr;
		Elf_Data * dwdata;
		/* this is the number of the dwarf section
		 * (as returned by dbg_section_create_callback())
		 * in which to put the current run of data */
		Dwarf_Signed dw_sec_nr;
		Dwarf_Unsigned dw_sec_size;
		Dwarf_Ptr dwbytes;

		if (!(dwbytes = dwarf_get_section_bytes(pdbg,
				i /* actually ignored by libdwarf */,
				&dw_sec_nr,
				&dw_sec_size,
				&err)))
			panic("");
		if (dw_sec_nr < 0 || dw_sec_nr >= last_dwarf_sectab_idx)
			panic("");

		/* see if this dwarf section has already been
		 * built */
		if (!dwsectab[dw_sec_nr]->dwsec)
		{
			int slen;
			/* no - build the section */
			/* create the destination section */
			if (!(dwsectab[dw_sec_nr]->dwsec = elf_newscn(outelf)))
				panic("");
			if (!gelf_getshdr(dwsectab[dw_sec_nr]->dwsec, &dwshdr))
				panic("");
			/* create a new string in the destination
			 * elf .strtab section for use by the currently
			 * processed dwarf section */
			if (!(out_sec_data = elf_newdata(out_strtab_sec)))
				panic("");
			out_sec_data->d_buf = dwsectab[dw_sec_nr]->name;
			slen = out_sec_data->d_size = strlen(dwsectab[dw_sec_nr]->name)
				+ /* account for the null byte terminator */ 1;
			out_sec_data->d_type = ELF_T_BYTE;
			out_sec_data->d_version = EV_CURRENT;
			out_sec_data->d_align = 1;
			/* set the output dwarf data section header fields */
			dwshdr.sh_name = out_strtab_idx;
			/* proceed to the next available string index
			 * in the destination .strtab section (to be
			 * used for the next(if any) dwarf section) */
			out_strtab_idx += slen;
			dwshdr.sh_type = dwsectab[dw_sec_nr]->type;
			dwshdr.sh_flags = dwsectab[dw_sec_nr]->flags;
			dwshdr.sh_info = dwsectab[dw_sec_nr]->info;
			dwshdr.sh_link = dwsectab[dw_sec_nr]->link;
			dwshdr.sh_addralign = 1;
			/* flush the section header to libelf */
			if (gelf_update_shdr(dwsectab[dw_sec_nr]->dwsec, &dwshdr) == -1)
				panic("");
		}

		/* create the output dwarf data */
		if (!(dwdata = elf_newdata(dwsectab[dw_sec_nr]->dwsec)))
			panic("");
		/* make a copy of the dwarf data bytes */
		if (!(dwdata->d_buf = malloc(dw_sec_size)))
			panic("");
		memcpy(dwdata->d_buf, dwbytes, dw_sec_size);
		dwdata->d_size = dw_sec_size;
		dwdata->d_type = ELF_T_BYTE;
		dwdata->d_version = EV_CURRENT;
		dwdata->d_align = 1;
	}

	if (gelf_update_ehdr(outelf, &in_ehdr) == -1)
		panic("");
	/* flush the data to the output elf file */
	if (elf_update(outelf, ELF_C_WRITE) == -1)
		panic("");
	elf_end(outelf);
	elf_end(inelf);
	return 0;
#if 0

	/* now dump the fresh dwarf sections built by libdwarf */
	if ((i = dwarf_transform_to_disk_form(pdbg, &err))
			== DW_DLV_BADADDR)
		panic("");
	for (j = 0; j < i; j++)
	{
		Elf_Scn * scn;
		GElf_Shdr shdr;
		/* create the destination section */
		if (!(scn = elf_newscn(outelf)))
			panic("");
		if (!(shdr = gelf_getshdr(scn, &shdr)))
			panic("");
	}

	/* flush the data to the output elf file */
	/* update the program-controlled data items */
	if (elf_update(outelf, ELF_C_WRITE) == -1)
		panic("");
	elf_end(outelf);
	elf_end(inelf);
#endif
}

