gprintf("die offset: %i\n", (int) type_die_offset);
printf("dwarf tag not handled: %s\n", dwarf_util_get_tag_name(tagval));
{ Dwarf_Off x; if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic(""); printf("cu relative offset: %i\n", (int) x); }


{
	Dwarf_Off x;
	if (dwarf_die_CU_offset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("\n");
	printf("cu relative offset: %i (0x%08x)\n", (int) x, (int) x);
	if (dwarf_dieoffset(die, &x, &err) != DW_DLV_OK) panic("");
	printf("absolute .debug_info offset: %i (0x%08x)\n", (int) x, (int) x);
}



Dwarf_Die sib_die;

	res = dwarf_siblingof(ctx->dbg, child_die, &sib_die, &err);
	dwarf_dealloc(ctx->dbg, child_die, DW_DLA_DIE);
	child_die = sib_die;

struct user_regs_struct
{
  long int ebx; 0
  long int ecx; 1
  long int edx; 2
  long int esi; 3
  long int edi; 4
  long int ebp; 5
  long int eax; 6
  long int xds; 7
  long int xes; 8
  long int xfs; 9
  long int xgs; 10
  long int orig_eax; 11
  long int eip; 12
  long int xcs; 13
  long int eflags; 14
  long int esp; 15
  long int xss; 16
};
