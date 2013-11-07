#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <gelf.h>
#include <stdio.h>

#define panic(__) do { printf("%i\n", __LINE__); while(1); } while(0)

int main(void)
{
int in_fd, out_fd;
Elf * inelf, * outelf;
GElf_Ehdr in_ehdr, * out_ehdr;
GElf_Phdr in_phdr, * out_phdr;
GElf_Shdr in_shdr;
Elf_Scn * in_sec, * out_sec;
Elf_Data * in_sec_data, * out_sec_data;
size_t nr_sections, nr_segments;
int i;
int elf_class;
const char * strtab;

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
	if (gelf_update_ehdr(outelf, &in_ehdr) == -1)
		panic("");
	/* copy program headers */
	nr_segments = ((Elf64_Ehdr *) &in_ehdr)->e_phnum;

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
	if ((i = ((Elf64_Ehdr *) &in_ehdr)->e_shstrndx)
			>= nr_sections)
		panic("");
	/* retrieve string table section */
	if (!(in_sec = elf_getscn(inelf, i)))
		panic("");
	/* get a pointer to the string table data */
	if (!(in_sec_data = elf_rawdata(in_sec,
		/* request first data buffer */0)))
		panic("");
	strtab = in_sec_data->d_buf;

	/* copy sections */
	for (i = /* skip the (mandatory) undefined
		  * section - it will be created automatically
		  * by libelf */ 1; i < nr_sections; i++)
	{
		/* retrieve the section to copy data from */
		if (!(in_sec = elf_getscn(inelf, i)))
			panic("");
		/* create the destination section */
		if (!(out_sec = elf_newscn(outelf)))
			panic("");
		if (!(gelf_getshdr(in_sec, &in_shdr)))
			panic("");
		/* dump section name - debugging */
		/*! \todo	make sure the strtab index is within bounds */
		printf("processing section %s\n", strtab + ((Elf64_Shdr *) &in_shdr)->sh_name);
		if (gelf_update_shdr(out_sec, &in_shdr) == -1)
			panic("");
		if (!(in_sec_data = elf_rawdata(in_sec,
			/* request first data buffer */0)))
			panic("");
		if (!(out_sec_data = elf_newdata(out_sec)))
			panic("");
		* out_sec_data = * in_sec_data;
		if (in_sec_data = elf_rawdata(in_sec, in_sec_data))
			panic("");

	}
	/* flush the data to the output elf file */
	/* update the program-controlled data items */
	if (elf_update(outelf, ELF_C_WRITE) == -1)
		panic("");
	elf_end(outelf);
	elf_end(inelf);
}

