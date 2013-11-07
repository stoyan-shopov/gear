#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libelf.h>
#include <gelf.h>
#include <errno.h>
#include "dwarf-common.h"
#include "target-defs.h"
#include "gear-engine-context.h"
#include "engine-err.h"
#include "core-access.h"
#include "util.h"


#define LOAD_BUF_SIZE	1024

int elfimg_load(struct gear_engine_context * ctx, char * img_fname)
{
Elf         *elf;
Elf32_Ehdr * ehdr;
Elf32_Phdr * phdr_tab, * phdr;
Elf_Data    *data;
int         fd, i;
char	img_load_buf[LOAD_BUF_SIZE];
unsigned int mem_addr;
int filesz, memsz;
unsigned int nbytes;
off_t fpos;

	/*! \todo	handle errors here */
	elf_version(EV_CURRENT);

	if ((fd = open(img_fname, O_RDONLY)) == -1)
		panic("failed to open elf image");
	if (!(elf = elf_begin(fd, ELF_C_READ, NULL)))
		panic("elf_begin()");
	if (!(ehdr = elf32_getehdr(elf)))
		panic("elf32_getehdr()");
	printf("ok, retrieved elf header, validating...\n");
	/* validate the elf header */
	if (ehdr->e_ident[EI_MAG0] != ELFMAG0 ||
		ehdr->e_ident[EI_MAG1] != ELFMAG1 ||
		ehdr->e_ident[EI_MAG2] != ELFMAG2 ||
		ehdr->e_ident[EI_MAG3] != ELFMAG3 ||
		ehdr->e_ident[EI_CLASS] != ELFCLASS32 ||
		ehdr->e_ident[EI_OSABI] != ELFOSABI_ARM
		)
		panic("invalid elf header");
	if (ehdr->e_type != ET_EXEC)
		panic("elf file not an executable image");
	if (ehdr->e_machine != EM_ARM)
		panic("elf file is not for an arm target");
	printf("ok\n");
	printf("processing program headers...");

	if (!(phdr_tab = elf32_getphdr(elf)))
		panic("elf32_getphdr()");

	phdr = phdr_tab;

	if (ehdr->e_phnum == 0)
		panic("no program header found in the image, nothing to do");

	printf("good; will now load the image into core...\n");
	/* ok, load image into core */
	for (i = 0; i < ehdr->e_phnum; i++)
	{
		printf("segment nr %i: ", i);
		if (phdr->p_type != PT_NULL)
		{
			if (phdr->p_type != PT_LOAD)
				panic("unsupported program header segment type");
			if (phdr->p_filesz > phdr->p_memsz)
				panic("file bytes to load more than mem bytes, doesnt know what to do");
			printf("type: load, ");
			printf("addr: 0x%08lx, size: 0x%08lx\n", phdr->p_vaddr, phdr->p_memsz);
			printf("loading segment data into core...");
		}
		else
			printf("null\n");

		mem_addr = phdr->p_vaddr;
		filesz = phdr->p_filesz;
		memsz = phdr->p_memsz;
		fpos = lseek(fd, (off_t)phdr->p_offset, SEEK_SET);
		if (fpos == -1)
		{
			panic("file positioning error");
		}

		nbytes = LOAD_BUF_SIZE;

		while (filesz > 0)
		{
			if ((read(fd, img_load_buf, LOAD_BUF_SIZE)) == -1)
				panic("error loading image");
			ctx->cc->core_mem_write(ctx, mem_addr, img_load_buf, &nbytes);
			filesz -= LOAD_BUF_SIZE;
			memsz -= LOAD_BUF_SIZE;
			mem_addr += LOAD_BUF_SIZE;
		}
		memset(img_load_buf, 0, LOAD_BUF_SIZE);
		while (memsz > 0)
		{
			ctx->cc->core_mem_write(ctx, mem_addr, img_load_buf, &nbytes);
			memsz -= LOAD_BUF_SIZE;
			mem_addr += LOAD_BUF_SIZE;
		}
		/* skip to next header */
		phdr = (Elf32_Phdr *)((unsigned int) phdr + ehdr->e_phentsize);
		printf("done\n");
	}

	elf_end(elf);
	close(fd);

	return 0;
}
