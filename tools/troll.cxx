#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "troll.hxx"

#define panic(msg) { printf("%s(), %s, %i: %s\n", __func__, __FILE__, __LINE__, msg); while (1); }

struct elf_dwarf_troll_class::dwarf_attr_struct * elf_dwarf_troll_class::fill_dw_attr_node(Dwarf_Attribute attr)
{
struct dwarf_attr_struct * p;
Dwarf_Error err;

	if (!(p = (struct dwarf_attr_struct *) malloc(sizeof * p )))
		panic("");
	if (dwarf_whatattr(attr, &p->attr, &err)
			!= DW_DLV_OK)
		panic("");
	if (dwarf_whatform(attr, &p->form, &err)
			!= DW_DLV_OK)
		panic("");
	switch (p->form)
	{
		case DW_FORM_addr:
			if (dwarf_formaddr(attr, &p->addr, &err)
					!= DW_DLV_OK)
				panic("");
			break;
		case DW_FORM_block:
		case DW_FORM_block1:
		case DW_FORM_block2:
		case DW_FORM_block4:
			if (dwarf_formblock(attr, &p->block, &err)
					!= DW_DLV_OK)
				panic("");
			break;
		case DW_FORM_data1:
		case DW_FORM_data2:
		case DW_FORM_data4:
		case DW_FORM_data8:
		case DW_FORM_udata:
			if (dwarf_formudata(attr, &p->udata, &err)
					!= DW_DLV_OK)
				panic("");
			break;
		case DW_FORM_sdata:
			if (dwarf_formsdata(attr, &p->sdata, &err)
					!= DW_DLV_OK)
				panic("");
			break;
		case DW_FORM_string:
		case DW_FORM_strp:
			if (dwarf_formstring(attr, &p->str, &err)
					!= DW_DLV_OK)
				panic("");
			break;
		case DW_FORM_flag:
			if (dwarf_formflag(attr, &p->flag, &err)
					!= DW_DLV_OK)
				panic("");
			break;
		case DW_FORM_ref_addr:
		case DW_FORM_ref1:
		case DW_FORM_ref2:
		case DW_FORM_ref4:
		case DW_FORM_ref8:
		case DW_FORM_ref_udata:
			if (dwarf_global_formref(attr, &p->global_ref, &err)
					!= DW_DLV_OK)
				panic("");
			break;

		default:
		case DW_FORM_indirect:
		case DW_FORM_sec_offset:
		case DW_FORM_exprloc:
		case DW_FORM_flag_present:
			panic("");
	}
	p->next = 0;
	return p;
}

struct elf_dwarf_troll_class::dwarf_die_struct * elf_dwarf_troll_class::build_die_tree(Dwarf_Debug dbg, Dwarf_Die die)
{
struct dwarf_die_struct * p;
Dwarf_Error err;
Dwarf_Attribute * attrlist;
Dwarf_Signed attrcnt;
struct dwarf_attr_struct ** pattr;
struct dwarf_die_struct ** pchild;
Dwarf_Die child_die;
int i;

	if (!(p = (struct dwarf_die_struct *) calloc(1, sizeof * p )))
		panic("");
	p->die = die;
	if (dwarf_dieoffset(die, &p->offset_in_debug_info, &err)
			!= DW_DLV_OK)
		panic("");
	if (dwarf_die_CU_offset(die, &p->cu_rel_offset, &err)
			!= DW_DLV_OK)
		panic("");
	if (dwarf_tag(p->die, &p->tag, &err)
			!= DW_DLV_OK)
		panic("");

	if (dwarf_attrlist(p->die, &attrlist, &attrcnt, &err)
			== DW_DLV_ERROR)
		panic("");
	/* build attribute list */
	pattr = &p->attrlist;
	for (i = 0; i < attrcnt; i++)
	{
		pattr = &(*pattr = fill_dw_attr_node(attrlist[i]))->next;
	}
	*pattr = 0;
	pchild = &p->children;
	if ((i = dwarf_child(die, &child_die, &err))
			== DW_DLV_ERROR)
		panic("");
	while (i != DW_DLV_NO_ENTRY)
	{
		pchild = &(*pchild = build_die_tree(dbg, child_die))->next_sibling;
		if ((i = dwarf_siblingof(dbg, child_die, &child_die, &err))
				== DW_DLV_ERROR)
			panic("");
	}
	*pchild = 0;

	return p;
}

const char * elf_dwarf_troll_class::dwarf_util_get_attr_name(Dwarf_Half attrval)
{
	switch (attrval)
	{
		case DW_AT_sibling:
			return("DW_AT_sibling ");
			break;
		case DW_AT_location:
			return("DW_AT_location ");
			break;
		case DW_AT_name:
			return("DW_AT_name ");
			break;
		case DW_AT_ordering:
			return("DW_AT_ordering ");
			break;
		case DW_AT_subscr_data:
			return("DW_AT_subscr_data ");
			break;
		case DW_AT_byte_size:
			return("DW_AT_byte_size ");
			break;
		case DW_AT_bit_offset:
			return("DW_AT_bit_offset ");
			break;
		case DW_AT_bit_size:
			return("DW_AT_bit_size ");
			break;
		case DW_AT_element_list:
			return("DW_AT_element_list ");
			break;
		case DW_AT_stmt_list:
			return("DW_AT_stmt_list ");
			break;
		case DW_AT_low_pc:
			return("DW_AT_low_pc ");
			break;
		case DW_AT_high_pc:
			return("DW_AT_high_pc ");
			break;
		case DW_AT_language:
			return("DW_AT_language ");
			break;
		case DW_AT_member:
			return("DW_AT_member ");
			break;
		case DW_AT_discr:
			return("DW_AT_discr ");
			break;
		case DW_AT_discr_value:
			return("DW_AT_discr_value ");
			break;
		case DW_AT_visibility:
			return("DW_AT_visibility ");
			break;
		case DW_AT_import:
			return("DW_AT_import ");
			break;
		case DW_AT_string_length:
			return("DW_AT_string_length ");
			break;
		case DW_AT_common_reference:
			return("DW_AT_common_reference ");
			break;
		case DW_AT_comp_dir:
			return("DW_AT_comp_dir ");
			break;
		case DW_AT_const_value:
			return("DW_AT_const_value ");
			break;
		case DW_AT_containing_type:
			return("DW_AT_containing_type ");
			break;
		case DW_AT_default_value:
			return("DW_AT_default_value ");
			break;
		case DW_AT_inline:
			return("DW_AT_inline ");
			break;
		case DW_AT_is_optional:
			return("DW_AT_is_optional ");
			break;
		case DW_AT_lower_bound:
			return("DW_AT_lower_bound ");
			break;
		case DW_AT_producer:
			return("DW_AT_producer ");
			break;
		case DW_AT_prototyped:
			return("DW_AT_prototyped ");
			break;
		case DW_AT_return_addr:
			return("DW_AT_return_addr ");
			break;
		case DW_AT_start_scope:
			return("DW_AT_start_scope ");
			break;
		case DW_AT_bit_stride:
			return("DW_AT_bit_stride ");
			break;
		case DW_AT_upper_bound:
			return("DW_AT_upper_bound ");
			break;
		case DW_AT_abstract_origin:
			return("DW_AT_abstract_origin ");
			break;
		case DW_AT_accessibility:
			return("DW_AT_accessibility ");
			break;
		case DW_AT_address_class:
			return("DW_AT_address_class ");
			break;
		case DW_AT_artificial:
			return("DW_AT_artificial ");
			break;
		case DW_AT_base_types:
			return("DW_AT_base_types ");
			break;
		case DW_AT_calling_convention:
			return("DW_AT_calling_convention ");
			break;
		case DW_AT_count:
			return("DW_AT_count ");
			break;
		case DW_AT_data_member_location:
			return("DW_AT_data_member_location ");
			break;
		case DW_AT_decl_column:
			return("DW_AT_decl_column ");
			break;
		case DW_AT_decl_file:
			return("DW_AT_decl_file ");
			break;
		case DW_AT_decl_line:
			return("DW_AT_decl_line ");
			break;
		case DW_AT_declaration:
			return("DW_AT_declaration ");
			break;
		case DW_AT_discr_list:
			return("DW_AT_discr_list ");
			break;
		case DW_AT_encoding:
			return("DW_AT_encoding ");
			break;
		case DW_AT_external:
			return("DW_AT_external ");
			break;
		case DW_AT_frame_base:
			return("DW_AT_frame_base ");
			break;
		case DW_AT_friend:
			return("DW_AT_friend ");
			break;
		case DW_AT_identifier_case:
			return("DW_AT_identifier_case ");
			break;
		case DW_AT_macro_info:
			return("DW_AT_macro_info ");
			break;
		case DW_AT_namelist_item:
			return("DW_AT_namelist_item ");
			break;
		case DW_AT_priority:
			return("DW_AT_priority ");
			break;
		case DW_AT_segment:
			return("DW_AT_segment ");
			break;
		case DW_AT_specification:
			return("DW_AT_specification ");
			break;
		case DW_AT_static_link:
			return("DW_AT_static_link ");
			break;
		case DW_AT_type:
			return("DW_AT_type ");
			break;
		case DW_AT_use_location:
			return("DW_AT_use_location ");
			break;
		case DW_AT_variable_parameter:
			return("DW_AT_variable_parameter ");
			break;
		case DW_AT_virtuality:
			return("DW_AT_virtuality ");
			break;
		case DW_AT_vtable_elem_location:
			return("DW_AT_vtable_elem_location ");
			break;
		case DW_AT_allocated:
			return("DW_AT_allocated ");
			break;
		case DW_AT_associated:
			return("DW_AT_associated ");
			break;
		case DW_AT_data_location:
			return("DW_AT_data_location ");
			break;
		case DW_AT_byte_stride:
			return("DW_AT_byte_stride ");
			break;
		case DW_AT_entry_pc:
			return("DW_AT_entry_pc ");
			break;
		case DW_AT_use_UTF8:
			return("DW_AT_use_UTF8 ");
			break;
		case DW_AT_extension:
			return("DW_AT_extension ");
			break;
		case DW_AT_ranges:
			return("DW_AT_ranges ");
			break;
		case DW_AT_trampoline:
			return("DW_AT_trampoline ");
			break;
		case DW_AT_call_column:
			return("DW_AT_call_column ");
			break;
		case DW_AT_call_file:
			return("DW_AT_call_file ");
			break;
		case DW_AT_call_line:
			return("DW_AT_call_line ");
			break;
		case DW_AT_description:
			return("DW_AT_description ");
			break;
		case DW_AT_binary_scale:
			return("DW_AT_binary_scale ");
			break;
		case DW_AT_decimal_scale:
			return("DW_AT_decimal_scale ");
			break;
		case DW_AT_small:
			return("DW_AT_small ");
			break;
		case DW_AT_decimal_sign:
			return("DW_AT_decimal_sign ");
			break;
		case DW_AT_digit_count:
			return("DW_AT_digit_count ");
			break;
		case DW_AT_picture_string:
			return("DW_AT_picture_string ");
			break;
		case DW_AT_mutable:
			return("DW_AT_mutable ");
			break;
		case DW_AT_threads_scaled:
			return("DW_AT_threads_scaled ");
			break;
		case DW_AT_explicit:
			return("DW_AT_explicit ");
			break;
		case DW_AT_object_pointer:
			return("DW_AT_object_pointer ");
			break;
		case DW_AT_endianity:
			return("DW_AT_endianity ");
			break;
		case DW_AT_elemental:
			return("DW_AT_elemental ");
			break;
		case DW_AT_pure:
			return("DW_AT_pure ");
			break;
		case DW_AT_recursive:
			return("DW_AT_recursive ");
			break;

		case DW_AT_lo_user:
			return("DW_AT_lo_user ");
			break;
		case DW_AT_MIPS_fde:
			return("DW_AT_MIPS_fde ");
			break;
		case DW_AT_MIPS_loop_begin:
			return("DW_AT_MIPS_loop_begin ");
			break;
		case DW_AT_MIPS_tail_loop_begin:
			return("DW_AT_MIPS_tail_loop_begin ");
			break;
		case DW_AT_MIPS_epilog_begin:
			return("DW_AT_MIPS_epilog_begin ");
			break;
		case DW_AT_MIPS_loop_unroll_factor:
			return("DW_AT_MIPS_loop_unroll_factor ");
			break;
		case DW_AT_MIPS_software_pipeline_depth:
			return("DW_AT_MIPS_software_pipeline_depth ");
			break;
		case DW_AT_MIPS_linkage_name:
			return("DW_AT_MIPS_linkage_name ");
			break;
		case DW_AT_MIPS_stride:
			return("DW_AT_MIPS_stride ");
			break;
		case DW_AT_MIPS_abstract_name:
			return("DW_AT_MIPS_abstract_name ");
			break;
		case DW_AT_MIPS_clone_origin:
			return("DW_AT_MIPS_clone_origin ");
			break;
		case DW_AT_MIPS_has_inlines:
			return("DW_AT_MIPS_has_inlines ");
			break;
		case DW_AT_MIPS_stride_byte:
			return("DW_AT_MIPS_stride_byte ");
			break;
		case DW_AT_MIPS_stride_elem:
			return("DW_AT_MIPS_stride_elem ");
			break;
		case DW_AT_MIPS_ptr_dopetype:
			return("DW_AT_MIPS_ptr_dopetype ");
			break;
		case DW_AT_MIPS_allocatable_dopetype:
			return("DW_AT_MIPS_allocatable_dopetype ");
			break;
		case DW_AT_MIPS_assumed_shape_dopetype:
			return("DW_AT_MIPS_assumed_shape_dopetype ");
			break;
		case DW_AT_MIPS_assumed_size:
			return("DW_AT_MIPS_assumed_size ");
			break;
		case DW_AT_sf_names:
			return("DW_AT_sf_names ");
			break;
		case DW_AT_src_info:
			return("DW_AT_src_info ");
			break;
		case DW_AT_mac_info:
			return("DW_AT_mac_info ");
			break;
		case DW_AT_src_coords:
			return("DW_AT_src_coords ");
			break;
		case DW_AT_body_begin:
			return("DW_AT_body_begin ");
			break;
		case DW_AT_body_end:
			return("DW_AT_body_end ");
			break;
		case DW_AT_hi_user:
			return("DW_AT_hi_user ");
			break;
		default:
			printf("*warning*: unknown dwarf attribute %i (0x%04x)\n",
					(int) attrval, (int) attrval);
			return("*** unknown attribute ***");
	}
}

const char * elf_dwarf_troll_class::dwarf_util_get_tag_name(Dwarf_Half tag)
{
const char * tagstr;

	switch (tag)
	{
		case DW_TAG_array_type:
			tagstr = "DW_TAG_array_type";
			break;
		case DW_TAG_class_type:
			tagstr = "DW_TAG_class_type";
			break;
		case DW_TAG_entry_point:
			tagstr = "DW_TAG_entry_point";
			break;
		case DW_TAG_enumeration_type:
			tagstr = "DW_TAG_enumeration_type";
			break;
		case DW_TAG_formal_parameter:
			tagstr = "DW_TAG_formal_parameter";
			break;
		case DW_TAG_imported_declaration:
			tagstr = "DW_TAG_imported_declaration";
			break;
		case DW_TAG_label:
			tagstr = "DW_TAG_label";
			break;
		case DW_TAG_lexical_block:
			tagstr = "DW_TAG_lexical_block";
			break;
		case DW_TAG_member:
			tagstr = "DW_TAG_member";
			break;
		case DW_TAG_pointer_type:
			tagstr = "DW_TAG_pointer_type";
			break;
		case DW_TAG_reference_type:
			tagstr = "DW_TAG_reference_type";
			break;
		case DW_TAG_compile_unit:
			tagstr = "DW_TAG_compile_unit";
			break;
		case DW_TAG_string_type:
			tagstr = "DW_TAG_string_type";
			break;
		case DW_TAG_structure_type:
			tagstr = "DW_TAG_structure_type";
			break;
		case DW_TAG_subroutine_type:
			tagstr = "DW_TAG_subroutine_type";
			break;
		case DW_TAG_typedef:
			tagstr = "DW_TAG_typedef";
			break;
		case DW_TAG_union_type:
			tagstr = "DW_TAG_union_type";
			break;
		case DW_TAG_unspecified_parameters:
			tagstr = "DW_TAG_unspecified_parameters";
			break;
		case DW_TAG_variant:
			tagstr = "DW_TAG_variant";
			break;
		case DW_TAG_common_block:
			tagstr = "DW_TAG_common_block";
			break;
		case DW_TAG_common_inclusion:
			tagstr = "DW_TAG_common_inclusion";
			break;
		case DW_TAG_inheritance:
			tagstr = "DW_TAG_inheritance";
			break;
		case DW_TAG_inlined_subroutine:
			tagstr = "DW_TAG_inlined_subroutine";
			break;
		case DW_TAG_module:
			tagstr = "DW_TAG_module";
			break;
		case DW_TAG_ptr_to_member_type:
			tagstr = "DW_TAG_ptr_to_member_type";
			break;
		case DW_TAG_set_type:
			tagstr = "DW_TAG_set_type";
			break;
		case DW_TAG_subrange_type:
			tagstr = "DW_TAG_subrange_type";
			break;
		case DW_TAG_with_stmt:
			tagstr = "DW_TAG_with_stmt";
			break;
		case DW_TAG_access_declaration:
			tagstr = "DW_TAG_access_declaration";
			break;
		case DW_TAG_base_type:
			tagstr = "DW_TAG_base_type";
			break;
		case DW_TAG_catch_block:
			tagstr = "DW_TAG_catch_block";
			break;
		case DW_TAG_const_type:
			tagstr = "DW_TAG_const_type";
			break;
		case DW_TAG_constant:
			tagstr = "DW_TAG_constant";
			break;
		case DW_TAG_enumerator:
			tagstr = "DW_TAG_enumerator";
			break;
		case DW_TAG_file_type:
			tagstr = "DW_TAG_file_type";
			break;
		case DW_TAG_friend:
			tagstr = "DW_TAG_friend";
			break;
		case DW_TAG_namelist:
			tagstr = "DW_TAG_namelist";
			break;
		case DW_TAG_namelist_item:
			tagstr = "DW_TAG_namelist_item";
			break;
		case DW_TAG_packed_type:
			tagstr = "DW_TAG_packed_type";
			break;
		case DW_TAG_subprogram:
			tagstr = "DW_TAG_subprogram";
			break;
		case DW_TAG_template_type_parameter:
			tagstr = "DW_TAG_template_type_parameter";
			break;
		case DW_TAG_template_value_parameter:
			tagstr = "DW_TAG_template_value_parameter";
			break;
		case DW_TAG_thrown_type:
			tagstr = "DW_TAG_thrown_type";
			break;
		case DW_TAG_try_block:
			tagstr = "DW_TAG_try_block";
			break;
		case DW_TAG_variant_part:
			tagstr = "DW_TAG_variant_part";
			break;
		case DW_TAG_variable:
			tagstr = "DW_TAG_variable";
			break;
		case DW_TAG_volatile_type:
			tagstr = "DW_TAG_volatile_type";
			break;
		case DW_TAG_dwarf_procedure:
			tagstr = "DW_TAG_dwarf_procedure";
			break;
		case DW_TAG_restrict_type:
			tagstr = "DW_TAG_restrict_type";
			break;
		case DW_TAG_interface_type:
			tagstr = "DW_TAG_interface_type";
			break;
		case DW_TAG_namespace:
			tagstr = "DW_TAG_namespace";
			break;
		case DW_TAG_imported_module:
			tagstr = "DW_TAG_imported_module";
			break;
		case DW_TAG_unspecified_type:
			tagstr = "DW_TAG_unspecified_type";
			break;
		case DW_TAG_partial_unit:
			tagstr = "DW_TAG_partial_unit";
			break;
		case DW_TAG_imported_unit:
			tagstr = "DW_TAG_imported_unit";
			break;
		default:
			printf("*warning*: unknown tag %i (0x%04x)\n",
					(int) tag,
					(int) tag);
			tagstr = "<<<unknown tag>>>";
			break;
	}

	return tagstr;
}

void elf_dwarf_troll_class::dump_dwarf_attr(struct dwarf_attr_struct * attr)
{
	switch (attr->form)
	{
		case DW_FORM_addr:
			printf("<addr> 0x%08x ", (int) attr->addr);
			break;
		case DW_FORM_block:
		case DW_FORM_block1:
		case DW_FORM_block2:
		case DW_FORM_block4:
			printf("<block> ");
			/* dump the block */
			{
				int i;
				if (attr->block->bl_from_loclist)
					printf("loclist data ");
				printf("<blklen %i> ", (int)attr->block->bl_len);
				printf("raw data: ");
				for (i = 0; i < attr->block->bl_len; i++)
					printf("%02x ", *((unsigned char *) attr->block->bl_data + i));
				printf("\n");
			}
			break;
		case DW_FORM_data4:
		case DW_FORM_data8:
			/* depending on the attribute value,
			 * a data4/data8 value can be one of:
			 *	- constant
			 *	- lineptr
			 *	- loclistptr
			 *	- macptr
			 *	- rangelistptr
			 *
			 * handle these cases here */
			/* see if the attribute allows for
			 * a loclistpr and handle it if it does */
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
					/* the class is a loclistptr */
					printf("<loclistptr at .debug_loc 0x%08x> ", (int) attr->udata);
					/* dump the list itself */
					{
						int res;
						Dwarf_Addr start, end;
						Dwarf_Ptr data;
						Dwarf_Unsigned offset;
						Dwarf_Unsigned entry_len;
						Dwarf_Error err;

						offset = attr->udata;

						if ((res = dwarf_get_loclist_entry(dbg,
								offset,
								&end,
								&start,
								&data,
								&entry_len,
								&offset,
								&err)) == DW_DLV_ERROR)
							panic("");
						if (res == DW_DLV_NO_ENTRY
								|| ((int)start == 0 && (int) end == 0))
								//|| ((int)start == (Dwarf_Addr) 0 && (int) end == (Dwarf_Addr) 0))
							printf("loclist empty");
						else while(1)
						{
							if (res == DW_DLV_NO_ENTRY
									|| ((int) start == 0 && (int) end == 0))
									//|| (start == (Dwarf_Addr) 0 && end == (Dwarf_Addr) 0))
								/* end of list */
								break;
							else if (res == DW_DLV_OK)
							{
								/* dump location expression */
								int i;
								printf("start: 0x%08x, end 0x%08x ", (int) start, (int) end);
								printf("raw data: ");
								for (i = 0; i < entry_len; i++)
									printf("%02x ", *((unsigned char *) data + i));
								printf("\n");
							}
							else
								panic("");
							res = dwarf_get_loclist_entry(dbg,
									offset,
									&end,
									&start,
									&data,
									&entry_len,
									&offset,
									&err);

						}
						printf("\n");
					}
					return;
				default:
					break;
			}
		case DW_FORM_data1:
		case DW_FORM_data2:
		case DW_FORM_udata:
			printf("<udata> %i ", (int) attr->udata);
			break;
		case DW_FORM_sdata:
			printf("<sdata> %i ", (int) attr->udata);
			break;
		case DW_FORM_string:
		case DW_FORM_strp:
			printf("<string> %s ", attr->str);
			break;
		case DW_FORM_flag:
			printf("<flag> %i ", (int) attr->flag);
			break;
		case DW_FORM_ref_addr:
		case DW_FORM_ref1:
		case DW_FORM_ref2:
		case DW_FORM_ref4:
		case DW_FORM_ref8:
		case DW_FORM_ref_udata:
			printf("<reference> %i ", (int) attr->global_ref);
			break;

		default:
		case DW_FORM_indirect:
		case DW_FORM_sec_offset:
		case DW_FORM_exprloc:
		case DW_FORM_flag_present:
			panic("");
	}
}

void elf_dwarf_troll_class::dump_dwarf_tree(int nesting_level, struct dwarf_die_struct * root)
{
struct dwarf_attr_struct * attr;
struct dwarf_die_struct * child;

	if (!root)
		return;
	/* dump tag */
	printf("<%i> < %i> (global .debug_info offset %i)\t%s\t", nesting_level,
			(int) root->cu_rel_offset,
			(int) root->offset_in_debug_info,
			dwarf_util_get_tag_name(root->tag));
	/* dump the list of attributes */
	attr = root->attrlist;
	while (attr)
	{
		printf("\n\t\t%s", dwarf_util_get_attr_name(attr->attr));
		dump_dwarf_attr(attr);
		attr = attr->next;
	}
	printf("\n");
	/* dump children */
	child = root->children;
	while (child)
	{
		dump_dwarf_tree(nesting_level + 1, child);
		child = child->next_sibling;
	}
}

/* constructor */
elf_dwarf_troll_class::elf_dwarf_troll_class(const char * elf_file_name)
{
int elf_fd;
int res;
Dwarf_Error err;
Dwarf_Unsigned	next_cu_header_offset;
Dwarf_Off cur_cu_offset;
Dwarf_Die die;
struct dwarf_die_struct ** pdie;


	elf_fd = open(elf_file_name, O_RDONLY | O_BINARY, 0);
	if (elf_version(EV_CURRENT) == EV_NONE)
	{
		panic("libelf out of date\n");
		exit(1);
	}

	if (elf_fd < 0)
	{
		panic("could not open input elf file\n");
		exit(1);
	}

	if ((res = dwarf_init(elf_fd, DW_DLC_READ, NULL, NULL, &dbg, &err)) != DW_DLV_OK)
	{
		panic("dwarf_init() failure");
	}

	while ((res = dwarf_next_cu_header(dbg,
					/* dont need these */
					0,
					0,
					0,
					0,
					&next_cu_header_offset,
					&err)) == DW_DLV_OK)
		/* kill time; as weve bloody got any of it to waste... */	 
		;
	if (res != DW_DLV_NO_ENTRY)
		panic("");
	/* start from offset 0 - the start of the debug information
	 * (.dbg_info) section */	 
	cur_cu_offset = 0;
	/* process all compilation units */
	pdie = &root;
	while ((res = dwarf_next_cu_header(dbg,
					/* dont need these */
					0,
					0,
					0,
					0,
					&next_cu_header_offset,
					&err)) == DW_DLV_OK)
	{
		if (dwarf_get_cu_die_offset_given_cu_header_offset
			(dbg, cur_cu_offset, &cur_cu_offset, &err)
				!= DW_DLV_OK)
			panic("");
		if (dwarf_offdie(dbg, cur_cu_offset, &die, &err)
				!= DW_DLV_OK)
			panic("");
		*pdie = build_die_tree(dbg, die);
		pdie = &((*pdie)->next_sibling);
		cur_cu_offset = next_cu_header_offset;
	}
}


void elf_dwarf_troll_class::print_tree(void)
{
	struct dwarf_die_struct * p = root;
	while (p)
		dump_dwarf_tree(0, p), p = p->next_sibling;
}

#if TROLL_TESTDRIVE
int main(int argc, char ** argv)
{
class elf_dwarf_troll_class * dwobj;

	if (argc != 2)
	{
		printf("usage: %s elf_file\n", argv[0]);
		exit(1);
	}
	dwobj = new elf_dwarf_troll_class(argv[1]);
	dwobj->print_tree();
	return 0;
}
#endif

