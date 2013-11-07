#include "c-expr-eval.hxx"
#include <iostream>

#include <stdio.h>

#include <limits.h>
#include <algorithm>



struct type_dump_seq
{
	/*! a flag denoting that we are currently printing the so-termed 'prefix' part of a declaration
	 *
	 * basically, a declaration is viewed as having three parts:
	 * a prefix part, an identifier part (can be missing), a suffix part
	 * (can be missing as well) - for example in the declaration:
	 * 'int (*arr_ptr)[3]', 'int (*' is the prefix part, 'arr_ptr' is
	 * the identifier part, and ')[3]' is the suffix part;
	 * as the one above this is really only used for proper printing
	 * of parentheses in 'pointers-to-arrays' declarations
	 */
	bool	is_prefix_printing	: 1;
	/*! indentation level used for prettyprinting
	 *
	 * this *must* be initialized to zero prior to the invocation of
	 * type_data_dump_int; prettyprinting basically causes the output to
	 * have $-s printed instead of tabs and @-s printed instead of newlines
	 * for type declarations containing structures and unions - this could be
	 * used for prettyprinting in a gear engine output consumer (e.g. a frontend);
	 * if such prettyprinting is not required in the consumer, $-s and @-s
	 * should simply be discarded from the type declaration string */
	int		indent_level;
};

void c_expr_eval::type_data_dump_int(struct dtype_data * type, bool is_prev_deref, bool is_in_deref, struct type_dump_seq * seq, std::stringstream & ss)
{
	int i;
	/*! printing sequencer data structure */
	struct type_dump_seq lseq;
	/*! a helper for handling arrays/array subranges */
	bool is_subrange;

	if (!type)
	{
		if (seq->is_prefix_printing)
			ss << "void ";
		return;
	}
	/* initialize a local copy of the printing sequencer */
	lseq = *seq;

	is_subrange = false;

	switch (type->dtype_class)
	{
		case DTYPE_CLASS_BASE_TYPE:
			if (!lseq.is_prefix_printing)
				return;
			switch (type->base_type_encoding)
			{
				case DW_ATE_unsigned:
				case DW_ATE_unsigned_char:

					switch (type->data_size)
					{
						case 1:
							if (type->name)
								ss << type->name << " ";
							else
								ss << "unsigned char ";
							break;
						case 2:
							if (type->name)
								ss << type->name << " ";
							else
								ss << "unsigned short ";
							break;
						case 4:
							if (type->name)
								ss << type->name << " ";
							else
								ss << "unsigned int ";
							break;
						case 8:
							if (type->name)
								ss << type->name << " ";
							else
								ss << "unsigned long long ";
							break;
						default:
							throw(std::string(__func__) + "(): " + "unsupported integer object size");
					}
					break;
				case DW_ATE_signed:
				case DW_ATE_signed_char:
					switch (type->data_size)
					{
						case 1:
							if (type->name)
								ss << type->name << " ";
							else
								ss << "signed char ";
							break;
						case 2:
							if (type->name)
								ss << type->name << " ";
							else
								ss << "signed short ";
							break;
						case 4:
							if (type->name)
								ss << type->name << " ";
							else
								ss << "signed int ";
							break;
						case 8:
							if (type->name)
								ss << type->name << " ";
							else
								ss << "signed long long ";
							break;
						default:
							throw(std::string(__func__) + "(): " + "unsupported integer object size");
					}
					break;
				case DW_ATE_boolean:
					switch (type->data_size)
					{
						case 1:
							if (type->name)
								ss << type->name << " ";
							else
								ss << "bool ";
							break;
						default:
							throw(std::string(__func__) + "(): " + "unsupported boolean size");
					}
					break;
				case DW_ATE_float:
					switch (type->data_size)
					{
						case sizeof(float):
							if (type->name)
								ss << type->name << " ";
							else
								ss << "float ";
							break;
						case sizeof(double):
							if (type->name)
								ss << type->name << " ";
							else
								ss << "double ";
							break;
						case sizeof(long double):
							if (type->name)
								ss << type->name << " ";
							else
								ss << "long double ";
							break;
						default:
							throw(std::string(__func__) + "(): " + "unsupported floating point object size");
					}
					break;
				default:
					throw(std::string(__func__) + "(): " + "unsupported base type encoding");
			}
			break;
		case DTYPE_CLASS_PTR:
			{
				/* initialize printing sequencer */
				if (lseq.is_prefix_printing)
				{
					type_data_dump_int(type->ptr_type, true, true, &lseq, ss);
					ss << "*";
				}
				else
				{
					type_data_dump_int(type->ptr_type, true, true, &lseq, ss);
				}
				break;
			}
		case DTYPE_CLASS_ARRAY_SUBRANGE:
			is_subrange = true;
			/* fall out */
		case DTYPE_CLASS_ARRAY:
			{
				/* used to iterate on the subranges */
				struct dtype_data *	p;
				/* number of array dimensions */
				int ndims;
				/*! an array storing the dimensions of an array that is being processed by ::type_data_dump_int */
				int arrdims[MAX_ARRAY_DIMENSIONS];
				struct dtype_data * arr_type;

				if (!is_subrange)
					arr_type = type->arr_data.element_type;
				else
					arr_type = type->arr_subrange_data.element_type;

				/* count number of indices of the array */
				i = 0;
				if (!is_subrange)
					p = type->arr_data.subranges;
				else
					p = type;
				while (p)
				{
					/*! \todo	handle flexible array members */
					if (p->arr_subrange_data.upper_bound == UINT_MAX)
						throw(std::string(__func__) + "(): " + "flexible array members not yet supported");
					arrdims[i++] = p->arr_subrange_data.upper_bound;
					if (i == MAX_ARRAY_DIMENSIONS)
						throw(std::string(__func__) + "(): " + "array has too many dimensions");
					p = p->arr_subrange_data.sib_ptr;
				}
				if (!i)
					throw(std::string(__func__) + "(): " + "bad dwarf array type description");
				ndims = i;

				/* just print type and dimensions */
				if (lseq.is_prefix_printing)
				{
					type_data_dump_int(arr_type, false, is_in_deref, &lseq, ss);
					if (is_prev_deref)
						ss << "(";
				}
				else
				{
					if (is_prev_deref)
						ss << ")";
					for (i = 0; i < ndims; i++)
					{
						ss << "[" << arrdims[i] + 1 << "]";
					}
					type_data_dump_int(arr_type, false, is_in_deref, &lseq, ss);
				}

				break;

			}

		case DTYPE_CLASS_CLASS:
			{
				if (lseq.is_prefix_printing)
				{
					/* print type */
					ss << "class ";
					/* if a pointer modifier has
					 * been detected, do not print
					 * type details for an aggregate;
					 * this is a simple heuristics to
					 * avoid infinite recursion in the
					 * case of aggregates containing
					 * as members pointers to themselves
					 *
					 * \todo	it may be considered
					 *		that all members
					 *		of a named aggregate
					 *		type be handled this
					 *		way (apparently, this
					 *		is what gdb does)
					 *
					 * an exception here is made for
					 * anonymous aggregate members, which
					 * are always printed */
					if (is_in_deref && type->name)
					{
						/* just print a short name and return */
						ss << type->name << " ";
						/* done printing type data */
						break;
					}
					if (type->name)
						ss << type->name << " ";
					else
						ss << "<anonymous>";
				}
				break;


			}

		case DTYPE_CLASS_STRUCT:
		case DTYPE_CLASS_UNION:
			{
				struct dtype_data * member;

				if (lseq.is_prefix_printing)
				{
					/* print type */
					if (type->dtype_class == DTYPE_CLASS_STRUCT)
						ss << "struct ";
					else
						ss << "union ";
					/* if a pointer modifier has
					 * been detected, do not print
					 * type details for an aggregate;
					 * this is a simple heuristics to
					 * avoid infinite recursion in the
					 * case of aggregates containing
					 * as members pointers to themselves
					 *
					 * \todo	it may be considered
					 *		that all members
					 *		of a named aggregate
					 *		type be handled this
					 *		way (apparently, this
					 *		is what gdb does)
					 *
					 * an exception here is made for
					 * anonymous aggregate members, which
					 * are always printed */
					if (is_in_deref && type->name)
					{
						/* just print a short name and return */
						ss << type->name << " ";
						/* done printing type data */
						break;
					}
					if (type->name)
						ss << type->name << " ";
					else
						ss << "<anonymous>";

					ss << "@";
					for (i = 0; i < lseq.indent_level; i++, ss << "$");
					ss << "{@";
					/* increase indentation level for printing the members */
					lseq.indent_level++;
					for (member = type->struct_data.data_members;
							member;
							member = member->member_data.sib_ptr)
					{
						lseq.is_prefix_printing = 1;
						for (i = 0; i < lseq.indent_level; i++, ss << "$");
						type_data_dump_int(member, false, is_in_deref, &lseq, ss);
						lseq.is_prefix_printing = 0;
						type_data_dump_int(member, false, is_in_deref, &lseq, ss);
						ss << "@";
					}
					/* done printing members - go back one indentation level */
					lseq.indent_level--;
					for (i = 0; i < lseq.indent_level; i++, ss << "$");
					ss << "} ";
				}
				break;
			}
		case DTYPE_CLASS_MEMBER:
			{
				if (lseq.is_prefix_printing)
				{
					type_data_dump_int(type->member_data.member_type, false, is_in_deref, &lseq, ss);
				}
				else
				{
					ss << type->name;
					type_data_dump_int(type->member_data.member_type, false, is_in_deref, &lseq, ss);
					ss << ";";
				}
				break;
			}

		case DTYPE_CLASS_SUBROUTINE:
			if (lseq.is_prefix_printing)
			{
				type_data_dump_int(type->subroutine_data.sub->type, false, is_in_deref, &lseq, ss);
				if (is_prev_deref)
					ss << "(";
			}
			else
			{
				struct dobj_data * param;
				struct type_dump_seq tseq;

				if (is_prev_deref)
					ss << ")";

				param = type->subroutine_data.sub->params;
				ss << "(";
				if (!param)
					ss << "void";
				else while (param)
				{
					tseq.is_prefix_printing = 1;

					type_data_dump_int(param->type, false, false, &tseq, ss);
					tseq.is_prefix_printing = 0;
					type_data_dump_int(param->type, false, false, &tseq, ss);

					param = param->sib_ptr;
					if (param)
						ss << ", ";
				}

				ss << ")";
				type_data_dump_int(type->subroutine_data.sub->type, false, is_in_deref, &lseq, ss);
			}
			break;

		case DTYPE_CLASS_TYPE_QUALIFIER:
			{
				if (lseq.is_prefix_printing)
				{
					switch (type->type_qualifier_data.qualifier_id)
					{
						default:
						case TYPE_QUALIFIER_INVALID:
							ss << "<unknown type qualifier> ";
							break;
						case TYPE_QUALIFIER_CONST:
							ss << "const ";
							break;
						case TYPE_QUALIFIER_RESTRICT:
							ss << "restrict ";
							break;
						case TYPE_QUALIFIER_VOLATILE:
							ss << "volatile ";
							break;
					}
					type_data_dump_int(type->type_qualifier_data.type, false, is_in_deref, &lseq, ss);
				}
				else
				{
					type_data_dump_int(type->type_qualifier_data.type, false, is_in_deref, &lseq, ss);
				}
				break;
			}

		case DTYPE_CLASS_TYPEDEF:
			if (lseq.is_prefix_printing)
			{
				/*! \todo	make this properly (now debug only) */
				if (1) ss << "<<<typedef>>> " << type->name << " ";
				if (1) type_data_dump_int(type->typedef_data.type, false, is_in_deref, &lseq, ss);
			}
			break;
		case DTYPE_CLASS_ENUMERATION:
			if (lseq.is_prefix_printing)
				ss << "enum " << type->name << " ";
			break;
		default:
			throw(std::string(__func__) + "(): " + "unsupported data type detected");
	}
}















void c_expr_eval::array_fixup(struct eval_node & p)
{
struct eval_node n;	
struct dtype_data * pt;

	if (!dtype_access_is_ctype(CTYPE_AGGREGATE, p.dtype) || dtype_access_is_ctype(CTYPE_STRUCT_OR_UNION_OR_CXX_CLASS, p.dtype))
		/* nothing to do */
		return;

	n.objkind = eval_node::OBJ_DTYPE;
	n.is_val_applicable = true;
	n.dtype = (pt = new dtype_data);
	memset(pt, 0, sizeof * pt);

	pt->head.tag = DW_TAG_pointer_type;
	pt->dtype_class = DTYPE_CLASS_PTR;
	pt->data_size = sizeof(ARM_CORE_WORD);
	pt->ptr_type = dtype_access_get_deref_type(p.dtype);

	if (is_computing_value)
	{
		n.is_val_computed = true;
		n.ptr_val = get_addr(p);
	}

	p = n;
}

unsigned int c_expr_eval::get_sizeof(struct eval_node & p)
{
	if (p.objkind != eval_node::OBJ_DTYPE)
		throw(std::string(__func__) + "(): " + "unsupported object type");
	return dtype_access_sizeof((struct dwarf_head_struct *) p.dtype);
}


ARM_CORE_WORD c_expr_eval::get_addr(struct eval_node & p)
{
	if (!is_lvalue(p))
		throw(std::string(__func__) + "(): " + "tried to compute the address of a non-lvalue object");
	if (p.is_addr_computed)
		/* object addres already computed - nothing more to do */
		return p.addr;
	bool is_result_a_reg;
	enum DWARF_EXPR_INFO_ENUM eval_info;
	/*! \todo	take target context in account here */
	/*! \todo	handle target errors here */
	p.addr = dwarf_expr_eval(ctx, * p.location.llbuf, 0, & is_result_a_reg, & eval_info);
	if (is_result_a_reg || eval_info >= DW_EXPR_LOCATION_IS_CONSTANT_BUT_NOT_ADDR)
		throw(std::string(__func__) + "(): " + "handle this");

	p.is_addr_computed = true;

	return p.addr;
}

void c_expr_eval::fetch_data(struct eval_node & p)
{
	if (p.objkind != eval_node::OBJ_DTYPE || !p.is_val_applicable)
		throw(std::string(__func__) + "(): " + "bad object type, or object does not have a value");
	if (p.is_val_computed)
		/* data available - nothing more to do */
		return;

	unsigned int i;
	ARM_CORE_WORD addr;
	addr = get_addr(p);
	i = get_sizeof(p);
	if (i == 0)
		throw(std::string(__func__) + "(): " + "data object size unknown");

	if (is_scalar_obj(p))
	{
		if (i > sizeof p.ld)
			throw(std::string(__func__) + "(): " + "scalar object size too large");
		/*! \todo	enable this */
		if (0) if (ctx->cc->core_mem_read(ctx, & p.ld, addr, & i) != GEAR_ERR_NO_ERROR)
			throw(std::string(__func__) + "(): " + "error reading target memory");
	}
	else
	{
		p.data = std::tr1::shared_ptr<uint8_t>(new uint8_t[i]);
		p.dlen = i;
		/*! \todo	enable this */
		if (0) if (ctx->cc->core_mem_read(ctx, p.data.get(), addr, & i) != GEAR_ERR_NO_ERROR)
			throw(std::string(__func__) + "(): " + "error reading target memory");
	}
	/*! \todo	remove this */
	if (1)
	{
		int i;
		for (i = 0; i < p.dlen; i++)
			p.data.get()[i] = i;
	}

	p.is_val_computed = true;
}


struct eval_node c_expr_eval::resolve_id(const char * id)
{
struct dwarf_head_struct * dwhead;
struct eval_node x;

	dwhead = deprecated_scope_locate_dobj(ctx, id, (struct scope_resolution_flags) { 1, });

	if (!dwhead)
	{
		throw(std::string(__func__) + "(): " + "identifier \"" + id + "\" not found; aborting expression evaluation\n");
	}

	/* determine and properly handle the type of the symbol just found */
	switch (dwarf_util_get_tag_category(dwhead->tag))
	{
		case DWARF_TAG_CATEGORY_SUBPROGRAM:
			x.objkind = eval_node::OBJ_SUBPROGRAM;
			x.subp = (struct subprogram_data *) dwhead;
			break;
		case DWARF_TAG_CATEGORY_DATA_OBJECT:
			x.objkind = eval_node::OBJ_DTYPE;
			x.dtype = ((struct dobj_data *) dwhead)->type;
			x.is_val_applicable = true;
			x.is_addr_applicable = ((struct dobj_data *) dwhead)->is_location_valid != 0;
			if (x.is_addr_applicable)
				x.location = ((struct dobj_data *) dwhead)->location;
			if (!x.is_addr_applicable)
				std::cout << std::string(__func__) + "(): " + "identifier \"" + id +"\" is not an lvalue\n";
			break;
		default:
			throw(std::string(__func__) + "(): " + "unsupported type for identifier " + id + "\n");
			break;
	}

	return x;
}

bool c_expr_eval::is_nonzero(struct eval_node & p)
{
bool res;

	if (!is_scalar_obj(p))
		throw(std::string(__func__) + "(): " + "scalar expected");
	if (!is_computing_value)
		throw(std::string(__func__) + "(): " + "value requested when not computing a value");

	if (is_ptr_obj(p))
		res = get_ptr_val(p) ? true : false;
	else if (is_int_obj(p))
		res = get_int(p) ? true : false;
	else if (is_float_obj(p))
		res = get_float(p) ? true : false;
	else
		throw(std::string(__func__) + "(): " + "unknown scalar type");
	p.is_val_computed = true;

	return res;
}

bool c_expr_eval::is_int_obj(struct eval_node & p) { if (p.is_val_applicable) return dtype_access_is_ctype(CTYPE_INTEGER, p.dtype); else return false; }
bool c_expr_eval::is_signed_int_obj(struct eval_node & p) { if (p.is_val_applicable) return dtype_access_is_ctype(CTYPE_SIGNED, p.dtype); else return false; }
bool c_expr_eval::is_unsigned_int_obj(struct eval_node & p) { if (p.is_val_applicable) return dtype_access_is_ctype(CTYPE_UNSIGNED, p.dtype); else return false; }
bool c_expr_eval::is_float_obj(struct eval_node & p) { if (p.is_val_applicable) return dtype_access_is_ctype(CTYPE_FLOAT, p.dtype); else return false; }
bool c_expr_eval::is_arith_obj(struct eval_node & p) { if (p.is_val_applicable) return dtype_access_is_ctype(CTYPE_ARITHMETIC, p.dtype); else return false; }
bool c_expr_eval::is_scalar_obj(struct eval_node & p) { if (p.is_val_applicable) return dtype_access_is_ctype(CTYPE_SCALAR, p.dtype); else return false; }
bool c_expr_eval::is_struct_obj(struct eval_node & p) { if (p.is_val_applicable) return dtype_access_is_ctype(CTYPE_STRUCT_OR_UNION_OR_CXX_CLASS, p.dtype); else return false; }
bool c_expr_eval::is_array_obj(struct eval_node & p) { if (p.is_val_applicable) return dtype_access_is_ctype(CTYPE_AGGREGATE, p.dtype) && !dtype_access_is_ctype(CTYPE_STRUCT_OR_UNION_OR_CXX_CLASS, p.dtype); else return false; }
bool c_expr_eval::is_aggregate_obj(struct eval_node & p) { if (p.is_val_applicable) return dtype_access_is_ctype(CTYPE_AGGREGATE, p.dtype); else return false; }
bool c_expr_eval::is_ptr_obj(struct eval_node & p) { if (p.is_val_applicable) return dtype_access_is_ctype(CTYPE_POINTER, p.dtype); else return false; }
bool c_expr_eval::is_func(struct eval_node & p) { if (p.objkind == eval_node::OBJ_SUBPROGRAM) return true; else return false; }


int64_t c_expr_eval::get_int(struct eval_node & p)
{
int64_t res;

	if (!is_arith_obj(p))
		throw(std::string(__func__) + "(): " + "bad type: arithmetic type expected");
	fetch_data(p);
	if (is_int_obj(p))
	{
		if (is_signed_int_obj(p))
		{
			switch (get_sizeof(p))
			{
				case sizeof(int8_t): res = p.i8; break;
				case sizeof(int16_t): res = p.i16; break;
				case sizeof(int32_t): res = p.i32; break;
				case sizeof(int64_t): res = p.i64; break;
				default: throw(std::string(__func__) + "(): " + "unsupported integer data type size");
			}
		}
		else
		{
			/* unsigned integer */
			switch (get_sizeof(p))
			{
				case sizeof(uint8_t): res = p.u8; break;
				case sizeof(uint16_t): res = p.u16; break;
				case sizeof(uint32_t): res = p.u32; break;
				case sizeof(uint64_t): res = p.u64; break;
				default: throw(std::string(__func__) + "(): " + "unsupported integer data type size");
			}
		}
	}
	else if (is_float_obj(p))
	{
			switch (get_sizeof(p))
			{
				case sizeof(float): res = p.f; break;
				case sizeof(double): res = p.d; break;
				case sizeof(long double): res = p.ld; break;
				default: throw(std::string(__func__) + "(): " + "unsupported floating point data type size");
			}
	}
	else
		throw(std::string(__func__) + "(): " + "unknown arithmetic type");
	return res;
}

long double c_expr_eval::get_float(struct eval_node & p)
{
long double res;

	if (!is_arith_obj(p))
		throw(std::string(__func__) + "(): " + "bad type: arithmetic type expected");
	fetch_data(p);
	if (is_int_obj(p))
	{
		if (is_signed_int_obj(p))
		{
			switch (get_sizeof(p))
			{
				case sizeof(int8_t): res = p.i8; break;
				case sizeof(int16_t): res = p.i16; break;
				case sizeof(int32_t): res = p.i32; break;
				case sizeof(int64_t): res = p.i64; break;
				default: throw(std::string(__func__) + "(): " + "unsupported integer data type size");
			}
		}
		else
		{
			/* unsigned integer */
			switch (get_sizeof(p))
			{
				case sizeof(int8_t): res = p.u8; break;
				case sizeof(int16_t): res = p.u16; break;
				case sizeof(int32_t): res = p.u32; break;
				case sizeof(int64_t): res = p.u64; break;
				default: throw(std::string(__func__) + "(): " + "unsupported integer data type size");
			}
		}
	}
	else if (is_float_obj(p))
	{
			switch (get_sizeof(p))
			{
				case sizeof(float): res = p.f; break;
				case sizeof(double): res = p.d; break;
				case sizeof(long double): res = p.ld; break;
				default: throw(std::string(__func__) + "(): " + "unsupported floating point data type size");
			}
	}
	else
		throw(std::string(__func__) + "(): " + "unknown arithmetic type");
	return res;
}

uint32_t c_expr_eval::get_ptr_val(struct eval_node & p)
{

	if (!is_ptr_obj(p))
		throw(std::string(__func__) + "(): " + "bad type: pointer type expected");
	fetch_data(p);
	return p.ptr_val;
}

bool c_expr_eval::is_ptr_to_incomplete_or_void_obj(struct eval_node & p)
{
bool res;

	res = false;
	if (p.is_val_applicable)
	{
		/*! \todo	also check for incomplete types here */
		if (dtype_access_is_ctype(CTYPE_POINTER, p.dtype) && !dtype_access_get_deref_type(p.dtype))
			res = true;
	}

	return res;
}

struct eval_node c_expr_eval::get_arith_superior(struct eval_node & x1, struct eval_node & x2)
{
	if (!(is_arith_obj(x1) && is_arith_obj(x2)))
		throw(std::string(__func__) + "(): " + "bad types: arithmetic types expected");
	if (dtype_access_is_ctype(CTYPE_FLOAT, x1.dtype) || dtype_access_is_ctype(CTYPE_FLOAT, x2.dtype))
		return make_float_obj();
	else
		return make_signed_int_obj();
}

struct eval_node c_expr_eval::make_float_obj(void)
{
struct eval_node n;
struct dtype_data * t;

	n.objkind = eval_node::OBJ_DTYPE;
	n.is_val_applicable = true;
	n.dtype = (t = new dtype_data);
	memset(t, 0, sizeof * t);

	t->head.tag = DW_TAG_base_type;
	t->dtype_class = DTYPE_CLASS_BASE_TYPE;
	t->data_size = sizeof(long double);
	t->base_type_encoding = DW_ATE_float;

	return n;

}

struct eval_node c_expr_eval::make_signed_int_obj(void)
{
struct eval_node n;
struct dtype_data * t;

	n.objkind = eval_node::OBJ_DTYPE;
	n.is_val_applicable = true;
	n.dtype = (t = new dtype_data);
	memset(t, 0, sizeof * t);

	t->head.tag = DW_TAG_base_type;
	t->dtype_class = DTYPE_CLASS_BASE_TYPE;
	t->data_size = sizeof(int64_t);
	t->base_type_encoding = DW_ATE_signed;

	return n;
}

struct eval_node c_expr_eval::make_ptr_obj(struct eval_node & p)
{
struct eval_node n;
struct dtype_data * t, * pt;

	if (!is_lvalue(p))
		throw(std::string(__func__) + "(): " + "bad operand: expected an lvalue");
	pt = p.dtype;
	n.objkind = eval_node::OBJ_DTYPE;
	n.is_val_applicable = true;
	n.dtype = (t = new dtype_data);
	memset(t, 0, sizeof * t);

	t->head.tag = DW_TAG_pointer_type;
	t->dtype_class = DTYPE_CLASS_PTR;
	t->data_size = sizeof(ARM_CORE_WORD);
	t->ptr_type = pt;

	return n;
}

struct eval_node c_expr_eval::get_struct_member_obj(struct eval_node & p, const char * id)
{
struct dtype_data * m;
struct eval_node n;

	if (p.objkind != eval_node::OBJ_DTYPE || !p.is_val_applicable || !dtype_access_is_ctype(CTYPE_STRUCT_OR_UNION_OR_CXX_CLASS, p.dtype))
		throw(std::string(__func__) + "(): " + "bad object type");

	/* scan the list of members */
	m = dtype_access_get_unqualified_base_type(p.dtype)->struct_data.data_members;
	while (m)
	{
		std::cout << "checking member name \"" << m->name << "\"...\n";
		/*! \todo	properly handle anonymous members here */
		if (/* skip over anonymous members */m->name
				&& !strcmp(m->name, id))
			break;
		m = m->member_data.sib_ptr;
	}
	if (!m)
	{
		/* member not found */
		throw(std::string(__func__) + "(): " + "unknown struct/union field name: " + id);
	}

	n.objkind = eval_node::OBJ_DTYPE;
	n.is_addr_applicable = p.is_addr_applicable;
	n.is_val_applicable = p.is_val_applicable;
	n.dtype = m->member_data.member_type;

	if (is_computing_value)
	{
		if (!is_lvalue(p))
			throw(std::string(__func__) + "(): " + "expected an lvalue");
		n.addr = get_addr(p) + m->member_data.member_location;
		n.is_addr_computed = true;
		make_lvalue(n);
	}

	return n;
}

struct eval_node c_expr_eval::ptr_arith_add(struct eval_node & n1, struct eval_node & n2)
{
struct eval_node t;	

	if (!((is_ptr_obj(n1) && is_int_obj(n2)) || (is_ptr_obj(n2) && is_int_obj(n1))))
		throw(std::string(__func__) + "(): " + "bad arguments for pointer arithmetic (expected a pointer and an integer)");
	if (!is_ptr_obj(n1))
	{

		t = n1;
		n1 = n2;
		n2 = t;
	}

	if (is_computing_value)
	{
		ARM_CORE_WORD addr;

		t = get_deref_obj(n1);

		addr = get_ptr_val(n1);
		addr += get_int(n2) * get_sizeof(t);

		n1.is_val_computed = true;
		n1.ptr_val = addr;
	}
	make_nonlvalue(n1);
	return n1;

}

/*
 *
 * c-language expression evaluation routines follow
 *
 */


struct eval_node c_expr_eval::eval_type_primary_expr(struct primary_expr * p)
{
	/*
	   primary_expression
	   : IDENTIFIER
	   | CONSTANT
	   | STRING_LITERAL
	   | '(' expression ')'
	   ;
	   */
	struct eval_node n;
	if (p->ident)
	{
		n = resolve_id(p->ident);
	}
	else if (p->str_literal)
		throw(std::string(__func__) + "(): " + "does not yet handle string literals\n");
	else if (p->expr)
		n = eval_type_expr(p->expr);
	else
	{
		switch (p->const_kind)
		{
			default:
				throw(std::string(__func__) + "(): " + "unknown constant kind");
				break;
			case CONST_FLOAT:
				n = make_float_obj();
				n.is_val_computed = true;
				n.ld = p->flconst;
				break;
			case CONST_INT:
				n = make_signed_int_obj();
				n.is_val_computed = true;
				n.i64 = p->iconst;
				break;
		}
	}

	return n;
}

struct eval_node c_expr_eval::eval_type_postfix_expr(struct postfix_expr * p)
{
	/*
	   postfix_expression
	   : primary_expression
	   | postfix_expression '[' expression ']'
	   | postfix_expression '(' ')'
	   | postfix_expression '(' argument_expression_list ')'
	   | postfix_expression '.' IDENTIFIER
	   | postfix_expression PTR_OP IDENTIFIER
	   | postfix_expression INC_OP
	   | postfix_expression DEC_OP
	   ;
	   */
	struct eval_node n, x;
	if (p->postfix_expr)
	{
		n = eval_type_postfix_expr(p->postfix_expr);
		array_fixup(n);
	}

	switch (p->op)
	{
		default:
			throw(std::string(__func__) + "(): " + "bad postfix operator");
		case INVALID_POSTFIX_OP:
			n = eval_type_primary_expr(p->primary_expr);
			break;
		case ARR_SUBSCRIPT:
			x = eval_type_expr(p->expr);
			array_fixup(x);
			n = ptr_arith_add(n, x);
			n = get_deref_obj(n);
			break;
		case FUNC_CALL:
			throw(std::string(__func__) + "(): " + "todo: function call operator not yet supported");
			break;
		case POSTFIX_INC_OP:
		case POSTFIX_DEC_OP:
			if (!(is_scalar_obj(n) && !is_ptr_to_incomplete_or_void_obj(n) && is_lvalue(n)))
				throw(std::string(__func__) + "(): " + "bad type for postfix increment/decrement operator");
			if (is_computing_value)
				throw(std::string(__func__) + "(): " + "unfinished");
			make_nonlvalue(n);
			break;
		case STRUCT_MEMBER_PTR_ACCESS:
			if (!(is_ptr_obj(n) && !is_ptr_to_incomplete_or_void_obj(n)))
				throw(std::string(__func__) + "(): " + "bad operands for pointer member access operator");
			n = get_deref_obj(n);
			/* fallout */
		case STRUCT_MEMBER_ACCESS:
			n = get_struct_member_obj(n, p->ident);
			if (!is_lvalue(n))
				throw(std::string(__func__) + "(): " + "operator not supported on non-lvalues");
			break;
	}

	return n;
}

struct eval_node c_expr_eval::eval_type_arg_expr_list(struct arg_expr_list * p)
{
	throw(std::string(__func__) + "(): " + "operator not supported");
}


struct eval_node c_expr_eval::eval_type_unary_expr(struct unary_expr * p)
{
bool is_computing_value;
	/*
	   unary_expression
	   : postfix_expression
	   | INC_OP unary_expression
	   | DEC_OP unary_expression
	   | unary_operator cast_expression
	   | SIZEOF unary_expression
	   | SIZEOF '(' type_name ')'
	   ;

	   unary_operator
	   : '&'
	   | '*'
	   | '+'
	   | '-'
	   | '~'
	   | '!'
	   ;
	   */
	struct eval_node n;
	/* a helper node */
	struct eval_node x;
	/* lookahead - handle special cases; in c, unary ampersand and the 'sizeof' operator
	 * have special-case semantics when applied to arrays and function designators */
	is_computing_value = this->is_computing_value;
	if (p->op == SIZEOF_OP)
		this->is_computing_value = false;
	if (p->cast_expr)
	{
		n = eval_type_cast_expr(p->cast_expr);
		if (p->op != UN_AMPERSAND)
			array_fixup(n);
	}
	else if (p->unary_expr)
	{
		n = eval_type_unary_expr(p->unary_expr);
		if (p->op != SIZEOF_OP)
			array_fixup(n);
	}
	this->is_computing_value = is_computing_value;

	switch (p->op)
	{
		default:
			throw(std::string(__func__) + "(): " + "bad unary operator");
		case INVALID_UNARY_OP:
			n = eval_type_postfix_expr(p->postfix_expr);
			break;
		case PREFIX_INC_OP:
		case PREFIX_DEC_OP:
			if (!(is_scalar_obj(n) && !is_ptr_to_incomplete_or_void_obj(n)))
				throw(std::string(__func__) + "(): " + "bad type for prefix increment/decrement operator");
			if (is_computing_value)
				throw(std::string(__func__) + "(): " + " unfinished");

			make_nonlvalue(n);
			break;
		case SIZEOF_OP:
			if (p->type_name)
				throw(std::string(__func__) + "(): " + "unfinished");
			x = make_signed_int_obj();
			if (is_computing_value)
			{
				x.i64 = get_sizeof(n);
				x.is_val_computed = true;
			}
			n = x;
			break;
		case UN_AMPERSAND:
			if (!is_lvalue(n))
				throw(std::string(__func__) + "(): " + "bad type for unary ampersand operator");
			x = make_ptr_obj(n);
			if (is_computing_value)
			{
				x.is_val_computed = true;
				x.ptr_val = get_addr(n);
			}
			n = x;
			make_nonlvalue(n);
			break;
		case UN_ASTERISK:
			if (!is_ptr_obj(n) || is_ptr_to_incomplete_or_void_obj(n))
				throw(std::string(__func__) + "(): " + "bad type for unary asterisk (pointer dereference) operator; expected a pointer to a complete type");

			n = get_deref_obj(n);
			break;
		case UN_PLUS:
		case UN_MINUS:
			if (!is_arith_obj(n))
				throw(std::string(__func__) + "(): " + "bad type for unary plus/minus operator; expected an arithmetic type");
			if (is_computing_value)
			{
				if (is_float_obj(n))
				{
					if (p->op == UN_MINUS)
						n.ld = - get_float(n);
					else
						n.ld = get_float(n);
				}
				else if (is_int_obj(n))
				{
					if (p->op == UN_MINUS)
						n.i64 = - get_int(n);
					else
						n.i64 = get_int(n);
				}
				else
					throw(std::string(__func__) + "(): " + "unknown arithmetic type");
				n.is_val_computed = true;
			}
			break;
		case UN_TILDE:
			if (!is_int_obj(n))
				throw(std::string(__func__) + "(): " + "bad type for tilde (bitwise inversion) operator; expected an integer");
			if (is_computing_value)
			{
				struct eval_node s;
				s = make_signed_int_obj();
				s.i64 = ~ get_int(n);
				s.is_val_computed = true;
				n = s;
			}
			break;
		case UN_EXCLAM:
			{
				if (!is_scalar_obj(n))
					throw(std::string(__func__) + "(): " + "bad type for exclamation mark (logical inversion) operator; expected an integer");
				x = make_signed_int_obj();

				if (is_computing_value)
				{
				bool res;

					res = is_nonzero(n);
					x.is_val_computed = true;
					x.i64 = res ? 0 : 1;
				}
				n = x;
			}
			break;
	}
	return n;
}

struct eval_node c_expr_eval::eval_type_cast_expr(struct cast_expr * p)
{
	/*
	   cast_expression
	   : unary_expression
	   | '(' type_name ')' cast_expression
	   ;
	   */
	struct eval_node n;
	if (p->type_name)
		throw(std::string(__func__) + "(): " + "operator not supported");
	else
		n = eval_type_unary_expr(p->unary_expr);

	return n;

}


struct eval_node c_expr_eval::eval_type_mult_expr(struct mult_expr * p)
{
	/*
	   multiplicative_expression
	   : cast_expression
	   | multiplicative_expression '*' cast_expression
	   | multiplicative_expression '/' cast_expression
	   | multiplicative_expression '%' cast_expression
	   ;
	   */
	struct eval_node n, x;
	n = eval_type_cast_expr(p->cast_expr);
	if (p->mult_expr)
		x = eval_type_mult_expr(p->mult_expr);
	switch (p->op)
	{
		default:
			throw(std::string(__func__) + "(): " + "bad multiplicative operator");
		case INVALID_MULT_OP:
			break;
		case MULT_TIMES:
			if (!(is_arith_obj(n) && is_arith_obj(x)))
				throw(std::string(__func__) + "(): " + "bad operand types for binary asterisk (multiplication) operator; expected arithmetic types");
			{
				struct eval_node s;
				s = get_arith_superior(n, x);
				if (is_computing_value)
				{
					if (is_float_obj(s)) s.ld = get_float(x) * get_float(n);
					else s.i64 = get_int(x) * get_int(n);
					s.is_val_computed = true;
				}
				n = s;
			}
			break;
		case MULT_DIV:
			if (!(is_arith_obj(n) && is_arith_obj(x)))
				throw(std::string(__func__) + "(): " + "bad operand types for binary slash (division) operator; expected arithmetic types");
			{
				struct eval_node s;
				s = get_arith_superior(n, x);
				if (is_computing_value)
				{
					if (is_float_obj(s))
					{
						if (get_float(n) == 0.)
							throw(std::string(__func__) + "(): " + "floating point divide by zero");

						s.ld = get_float(x) / get_float(n);
					}
					else
					{
						if (get_int(n) == 0)
							throw(std::string(__func__) + "(): " + "integer divide by zero");
						s.i64 = get_int(x) / get_int(n);
					}
					s.is_val_computed = true;
				}
				n = s;
			}
			break;
		case MULT_MOD:
			if (!(is_arith_obj(n) && is_arith_obj(x)))
				throw(std::string(__func__) + "(): " + "bad operand types for binary percentage (modulo) operator; expected arithmetic types");
			n = get_arith_superior(n, x);
			break;
	}
	return n;
}

struct eval_node c_expr_eval::eval_type_additive_expr(struct additive_expr * p)
{
	/*
	   additive_expression
	   : multiplicative_expression
	   | additive_expression '+' multiplicative_expression
	   | additive_expression '-' multiplicative_expression
	   ;
	   */
	struct eval_node n, x;
	n = eval_type_mult_expr(p->mult_expr);
	if (p->additive_expr)
	{
		x = eval_type_additive_expr(p->additive_expr);
		array_fixup(n);
		array_fixup(x);
	}
	switch (p->op)
	{
		default:
			throw(std::string(__func__) + "(): " + "bad additive operator");
		case INVALID_ADDITIVE_OP:
			break;
		case ADDITIVE_PLUS:
			if (!(
						(is_arith_obj(n) && is_arith_obj(x))
						|| ((is_ptr_obj(x) && !is_ptr_to_incomplete_or_void_obj(x)) && is_int_obj(n))
						|| ((is_ptr_obj(n) && !is_ptr_to_incomplete_or_void_obj(n)) && is_int_obj(x))
			     ))
				throw(std::string(__func__) + "(): " + "bad types for binary plus operator");
			if (is_arith_obj(n) && is_arith_obj(x))
			{
				struct eval_node s;
				s = get_arith_superior(n, x);
				if (is_computing_value)
				{
					if (is_float_obj(s)) s.ld = get_float(x) + get_float(n);
					else s.i64 = get_int(x) + get_int(n);
					s.is_val_computed = true;
				}
				n = s;
			}
			else
			{
				/* pointer arithmetic */
				n = ptr_arith_add(n, x);
			}
			break;
		case ADDITIVE_MINUS:
			std::cout << (is_ptr_obj(x) ? "pointer, " : "not a pointer; ");
			std::cout << (is_ptr_obj(n) ? "pointer, " : "not a pointer; ");
			if (!(
						(is_arith_obj(n) && is_arith_obj(x))
						|| ((is_ptr_obj(x) && !is_ptr_to_incomplete_or_void_obj(x)) && is_int_obj(n))
						|| ((is_ptr_obj(x) && !is_ptr_to_incomplete_or_void_obj(x)) && (is_ptr_obj(n) && !is_ptr_to_incomplete_or_void_obj(n)))
			     ))
				throw(std::string(__func__) + "(): " + "bad types for binary minus operator");
			if (is_arith_obj(n) && is_arith_obj(x))
			{
				struct eval_node s;
				s = get_arith_superior(n, x);
				if (is_computing_value)
				{
					if (is_float_obj(s)) s.ld = get_float(x) - get_float(n);
					else s.i64 = get_int(x) - get_int(n);
					s.is_val_computed = true;
				}
				n = s;
			}
			else if (is_int_obj(n))
			{
				throw(std::string(__func__) + "(): " + "finish binary minus");
				n = x;
			}
			else
			{
				throw(std::string(__func__) + "(): " + "finish binary minus");
				if (!are_types_equivalent(n, x))
					throw(std::string(__func__) + "(): " + "incompatible pointer types for binary minus operator");
				else
					n = make_signed_int_obj();
			}
			break;
	}
	return n;

}


struct eval_node c_expr_eval::eval_type_shift_expr(struct shift_expr * p)
{
	/*
	   shift_expression
	   : additive_expression
	   | shift_expression LEFT_OP additive_expression
	   | shift_expression RIGHT_OP additive_expression
	   ;
	   */
	struct eval_node n, x;
	n = eval_type_additive_expr(p->additive_expr);
	if (p->shift_expr)
		x = eval_type_shift_expr(p->shift_expr);
	switch (p->op)
	{
		default:
			throw(std::string(__func__) + "(): " + "bad shift operator");
		case INVALID_SHIFT_OP:
			break;
		case SHIFT_LEFT:
		case SHIFT_RIGHT:
			{
				struct eval_node s;

				if (!(is_int_obj(n) && is_int_obj(x)))
					throw(std::string(__func__) + "(): " + "bad types for left/right shift operator; expected integer types");
				s = make_signed_int_obj();
				if (is_computing_value)
				{
					if (p->op == SHIFT_LEFT)
						s.i64 = get_int(x) << get_int(n);
					else
						s.i64 = get_int(x) >> get_int(n);
					s.is_val_computed = true;
				}
				n = s;
			}
			break;
	}
	return n;

}


struct eval_node c_expr_eval::eval_type_rel_expr(struct rel_expr * p)
{
	/*
	   relational_expression
	   : shift_expression
	   | relational_expression '<' shift_expression
	   | relational_expression '>' shift_expression
	   | relational_expression LE_OP shift_expression
	   | relational_expression GE_OP shift_expression
	   ;
	   */
	struct eval_node n, x;
	n = eval_type_shift_expr(p->shift_expr);
	if (p->rel_expr)
	{
		x = eval_type_rel_expr(p->rel_expr);
		array_fixup(n);
		array_fixup(x);
	}
	switch (p->op)
	{
		default:
			throw(std::string(__func__) + "(): " + "bad relational operator");
		case INVALID_REL_OP:
			break;
		case REL_LESS:
		case REL_GREATER:
		case REL_LESS_OR_EQUAL:
		case REL_GREATER_OR_EQUAL:
		{
			struct eval_node s;
			if (!(
						(is_arith_obj(n) && is_arith_obj(x))
						|| (is_ptr_obj(n) && is_ptr_obj(x) && are_types_equivalent(n, x))
			     ))
				throw(std::string(__func__) + "(): " + "bad relational operator operands");
			s = make_signed_int_obj();
			if (is_computing_value)
			{
				bool res;
				if (is_arith_obj(x))
				{
					struct eval_node t;
					t = get_arith_superior(n, x);
					if (is_float_obj(t))
						switch (p->op)
						{
							case REL_LESS:
								res = get_float(x) < get_float(n) ? true : false;
								break;
							case REL_GREATER:
								res = get_float(x) > get_float(n) ? true : false;
								break;
							case REL_LESS_OR_EQUAL:
								res = get_float(x) <= get_float(n) ? true : false;
								break;
							case REL_GREATER_OR_EQUAL:
								res = get_float(x) >= get_float(n) ? true : false;
								break;
						}
					else switch (p->op)
						{
							case REL_LESS:
								res = get_int(x) < get_int(n) ? true : false;
								break;
							case REL_GREATER:
								res = get_int(x) > get_int(n) ? true : false;
								break;
							case REL_LESS_OR_EQUAL:
								res = get_int(x) <= get_int(n) ? true : false;
								break;
							case REL_GREATER_OR_EQUAL:
								res = get_int(x) >= get_int(n) ? true : false;
								break;
						}
				}
				else
				{
					ARM_CORE_WORD addr1, addr2;
					addr1 = get_ptr_val(x);
					addr2 = get_ptr_val(n);

					switch (p->op)
					{
						case REL_LESS:
							res = addr1 < addr2 ? true : false;
							break;
						case REL_GREATER:
							res = addr1 > addr2 ? true : false;
							break;
						case REL_LESS_OR_EQUAL:
							res = addr1 <= addr2 ? true : false;
							break;
						case REL_GREATER_OR_EQUAL:
							res = addr1 >= addr2 ? true : false;
							break;
					}
				}
				s.i64 = res ? 1 : 0;
				s.is_val_computed = true;
			}
			n = s;
		}
			break;
	}
	return n;
}


struct eval_node c_expr_eval::eval_type_eq_expr(struct eq_expr * p)
{
	/*
	   equality_expression
	   : relational_expression
	   | equality_expression EQ_OP relational_expression
	   | equality_expression NE_OP relational_expression
	   ;
	   */
	struct eval_node n, x;
	n = eval_type_rel_expr(p->rel_expr);
	if (p->eq_expr)
	{
		x = eval_type_eq_expr(p->eq_expr);
		array_fixup(n);
		array_fixup(x);
	}
	switch (p->op)
	{
		default:
			throw(std::string(__func__) + "(): " + "bad equality operator");
		case INVALID_EQ_OP:
			break;
		case EQ_EQUAL:
		case EQ_NOT_EQUAL:
		{
			struct eval_node s;
			if (!(
						(is_arith_obj(n) && is_arith_obj(x))
						|| (is_ptr_obj(n) && is_ptr_obj(x) && are_types_equivalent(n, x))
			     ))
				throw(std::string(__func__) + "(): " + "bad relational operator operands");
			s = make_signed_int_obj();
			if (is_computing_value)
			{
				bool res;
				if (is_arith_obj(x))
				{
					struct eval_node t;
					t = get_arith_superior(n, x);
					if (is_float_obj(t))
						switch (p->op)
						{
							case EQ_EQUAL:
								res = get_float(x) == get_float(n) ? true : false;
								break;
							case EQ_NOT_EQUAL:
								res = get_float(x) != get_float(n) ? true : false;
								break;
						}
					else switch (p->op)
						{
							case EQ_EQUAL:
								res = get_int(x) == get_int(n) ? true : false;
								break;
							case EQ_NOT_EQUAL:
								res = get_int(x) != get_int(n) ? true : false;
								break;
						}
				}
				else
				{
					ARM_CORE_WORD addr1, addr2;
					addr1 = get_ptr_val(x);
					addr2 = get_ptr_val(n);

					switch (p->op)
					{
						case EQ_EQUAL:
							res = addr1 == addr2 ? true : false;
							break;
						case EQ_NOT_EQUAL:
							res = addr1 != addr2 ? true : false;
							break;
					}
				}
				s.i64 = res ? 1 : 0;
				s.is_val_computed = true;
			}
			n = s;
		}
		break;
	}

	return n;
}

struct eval_node c_expr_eval::eval_type_and_expr(struct and_expr * p)
{
	/*
	   and_expression
	   : equality_expression
	   | and_expression '&' equality_expression
	   ;
	   */
	struct eval_node n, x;
	n = eval_type_eq_expr(p->eq_expr);
	if (p->and_expr)
		x = eval_type_and_expr(p->and_expr);
	switch (p->op)
	{
		default:
			throw(std::string(__func__) + "(): " + "bad 'bitwise and' operator");
		case INVALID_AND_OP:
			break;
		case AND_OP_AMPERSAND:
			{
				struct eval_node s;
				if (!(is_int_obj(n) && is_int_obj(x)))
					throw(std::string(__func__) + "(): " + "bad 'bitwise and' operator operands, integers expected");
				s = make_signed_int_obj();

				if (is_computing_value)
				{
					s.i64 = get_int(x) & get_int(n);
					s.is_val_computed = true;
				}

				n = s;
				break;
			}
	}
	return n;

}


struct eval_node c_expr_eval::eval_type_xor_expr(struct xor_expr * p)
{
	/*
	   exclusive_or_expression
	   : and_expression
	   | exclusive_or_expression '^' and_expression
	   ;
	   */
	struct eval_node n, x;
	n = eval_type_and_expr(p->and_expr);
	if (p->xor_expr)
		x = eval_type_xor_expr(p->xor_expr);
	switch (p->op)
	{
		default:
			throw(std::string(__func__) + "(): " + "bad 'xor' operator");
		case INVALID_XOR_OP:
			break;
		case XOR_OP:
			{
				struct eval_node s;
				if (!(is_int_obj(n) && is_int_obj(x)))
					throw(std::string(__func__) + "(): " + "bad xor operator operands, integers expected");
				s = make_signed_int_obj();

				if (is_computing_value)
				{
					s.i64 = get_int(x) ^ get_int(n);
					s.is_val_computed = true;
				}

				n = s;
				break;
			}
	}
	return n;

}

struct eval_node c_expr_eval::eval_type_incl_or_expr(struct incl_or_expr * p)
{
	/*
	   inclusive_or_expression
	   : exclusive_or_expression
	   | inclusive_or_expression '|' exclusive_or_expression
	   ;
	   */
	struct eval_node n, x;
	n = eval_type_xor_expr(p->xor_expr);
	if (p->incl_or_expr)
		x = eval_type_incl_or_expr(p->incl_or_expr);
	switch (p->op)
	{
		default:
			throw(std::string(__func__) + "(): " + "bad 'bitwise or' operator");
		case INVALID_INCL_OR_OP:
			break;
		case INCL_OR_OP:
			{
				struct eval_node s;
				if (!(is_int_obj(n) && is_int_obj(x)))
					throw(std::string(__func__) + "(): " + "bad 'bitwise or' operator operands, integer expected");
				s = make_signed_int_obj();

				if (is_computing_value)
				{
					s.i64 = get_int(x) | get_int(n);
					s.is_val_computed = true;
				}

				n = s;
				break;
			}
	}
	return n;

}

struct eval_node c_expr_eval::eval_type_logical_and_expr(struct logical_and_expr * p)
{
	/*
	   logical_and_expression
	   : inclusive_or_expression
	   | logical_and_expression AND_OP inclusive_or_expression
	   ;
	   */
	struct eval_node n, x;
	n = eval_type_incl_or_expr(p->incl_or_expr);
	if (p->logical_and_expr)
	{
		x = eval_type_logical_and_expr(p->logical_and_expr);
		array_fixup(x);
		array_fixup(n);
	}
	switch (p->op)
	{
		default:
			throw(std::string(__func__) + "(): " + "bad 'logical and' operator");
		case INVALID_LOGICAL_AND_OP:
			break;
		case LOGICAL_AND_OP:
			{
				struct eval_node s;
				if (!(is_scalar_obj(n) && is_scalar_obj(x)))
					throw(std::string(__func__) + "(): " + "bad 'logical and' operator operands");
				s = make_signed_int_obj();

				if (is_computing_value)
				{
					s.i64 = is_nonzero(x) && is_nonzero(n) ? 1 : 0;
					s.is_val_computed = true;
				}

				n = s;
				break;
			}
			break;
	}
	return n;

}


struct eval_node c_expr_eval::eval_type_logical_or_expr(struct logical_or_expr * p)
{
	/*
	   logical_or_expression
	   : logical_and_expression
	   | logical_or_expression OR_OP logical_and_expression
	   ;
	   */
	struct eval_node n, x;
	n = eval_type_logical_and_expr(p->logical_and_expr);
	if (p->logical_or_expr)
	{
		x = eval_type_logical_or_expr(p->logical_or_expr);
		array_fixup(x);
		array_fixup(n);
	}
	switch (p->op)
	{
		default:
			throw(std::string(__func__) + "(): " + "bad 'logical or' operator");
		case INVALID_LOGICAL_OR_OP:
			break;
		case LOGICAL_OR_OP:
			{
				struct eval_node s;
				if (!(is_scalar_obj(n) && is_scalar_obj(x)))
					throw(std::string(__func__) + "(): " + "bad 'logical or' operator operands");
				s = make_signed_int_obj();

				if (is_computing_value)
				{
					s.i64 = is_nonzero(x) || is_nonzero(n) ? 1 : 0;
					s.is_val_computed = true;
				}

				n = s;
				break;
			}
			break;
	}
	return n;

}

struct eval_node c_expr_eval::eval_type_cond_expr(struct cond_expr * p)
{
	/*
	   conditional_expression
	   : logical_or_expression
	   | logical_or_expression '?' expression ':' conditional_expression
	   ;
	   */

	struct eval_node n;
	n = eval_type_logical_or_expr(p->logical_or_expr);
	switch (p->op)
	{
		default:
			throw(std::string(__func__) + "(): " + "bad conditional operator");
		case INVALID_COND_EXPR_OP:
			break;
		case LOGICAL_TERNARY_OP:
			throw(std::string(__func__) + "(): " + "unfinished...");
	}
	return n;
}


struct eval_node c_expr_eval::eval_type_expr(struct expr * p)
{
	/*
	   expression
	   : conditional_expression
	   | unary_expression assignment_operator expression
	   ;

	   assignment_operator
	   : '='
	   | MUL_ASSIGN
	   | DIV_ASSIGN
	   | MOD_ASSIGN
	   | ADD_ASSIGN
	   | SUB_ASSIGN
	   | LEFT_ASSIGN
	   | RIGHT_ASSIGN
	   | AND_ASSIGN
	   | XOR_ASSIGN
	   | OR_ASSIGN
	   ;
	   */
	struct eval_node n;
	n = eval_type_cond_expr(p->cond_expr);
	switch (p->op)
	{
		default:
			throw(std::string(__func__) + "(): " + "unfinished/bad operator");
		case INVALID_EXPR_OP:
			break;
	}
	return n;

}

/*
 *
 * target data cooking routines
 *
 */

std::string c_expr_eval::get_scalar_value(struct dtype_data * t, uint8_t * data, unsigned dlen)
{
union
{
	int8_t		i8;
	uint8_t		u8;
	int16_t		i16;
	uint16_t	u16;
	int32_t		i32;
	uint32_t	u32;
	int64_t		i64;
	uint64_t	u64;
	float		f;
	double		d;
	long double	ld;

	ARM_CORE_WORD	ptr_val;
}
scalar;
std::stringstream ss;
unsigned int dsize;

	if (!dtype_access_is_ctype(CTYPE_SCALAR, t))
		throw(std::string(__func__) + "(): " + "bad type: scalar type expected");
	dsize = dtype_access_sizeof((struct dwarf_head_struct *) t);
	if (dlen < dsize)
		throw(std::string(__func__) + "(): " + "insufficient data amount available");

	memset(& scalar, 0, sizeof scalar);
	memcpy(& scalar, data, dlen > sizeof scalar ? sizeof scalar : dlen);

	if (dtype_access_is_ctype(CTYPE_INTEGER, t))
	{
		ss.width(10);
		if (dtype_access_is_ctype(CTYPE_SIGNED, t))
		{
			switch (dsize)
			{
				case sizeof(int8_t): ss << (uint16_t) scalar.i8; break;
				case sizeof(int16_t): ss << scalar.i16; break;
				case sizeof(int32_t): ss << scalar.i32; break;
				case sizeof(int64_t): ss << scalar.i64; break;
				default: throw(std::string(__func__) + "(): " + "unsupported integer data type size");
			}
		}
		else
		{
			/* unsigned integer */
			switch (dsize)
			{
				case sizeof(uint8_t): ss << (uint16_t) scalar.u8; break;
				case sizeof(uint16_t): ss << scalar.u16; break;
				case sizeof(uint32_t): ss << scalar.u32; break;
				case sizeof(uint64_t): ss << scalar.u64; break;
				default: throw(std::string(__func__) + "(): " + "unsupported integer data type size");
			}
		}
	}
	else if (dtype_access_is_ctype(CTYPE_FLOAT, t))
	{
		switch (dsize)
		{
			case sizeof(float): ss << scalar.f; break;
			case sizeof(double): ss << scalar.d; break;
			case sizeof(long double): ss << scalar.ld; break;
			default: throw(std::string(__func__) + "(): " + "unsupported floating point data type size");
		}
	}
	else if (dtype_access_is_ctype(CTYPE_POINTER, t))
	{
		ss << "0x" << std::hex << scalar.ptr_val;
	}
	else
		throw(std::string(__func__) + "(): " + "unknown scalar type");
	return ss.str();
}

void c_expr_eval::dump_value(struct dtype_data * t, uint8_t * data, unsigned dlen, std::string prefix)
{
	std::stringstream ss;
	struct type_dump_seq seq;
	seq.indent_level = 0;
	seq.is_prefix_printing = true;
	type_data_dump_int(t, false, false, & seq, ss);
	seq.is_prefix_printing = false;
	type_data_dump_int(t, false, false, & seq, ss);

	std::string stype = ss.str();
	std::replace(stype.begin(), stype.end(), '$', '\t');
	std::replace(stype.begin(), stype.end(), '@', '\n');

	std::cout << "type is: " << stype << std::endl;



	t = dtype_access_get_unqualified_base_type(t);
	if (dtype_access_is_ctype(CTYPE_SCALAR, t))
	{
		std::cout << prefix + "\t";
		std::cout << get_scalar_value(t, data, dlen) << std::endl;
	}
	else if (dtype_access_is_ctype(CTYPE_AGGREGATE, t))
	{
		if (dtype_access_is_ctype(CTYPE_STRUCT_OR_UNION_OR_CXX_CLASS, t))
		{

			/* process a structure/union/class aggregate */
			t = t->struct_data.data_members;
			while(t)
			{
				std::stringstream ss;

				ss << "." << t->name << " ";

				dump_value(t->member_data.member_type,
						data + t->member_data.member_location,
						dlen - t->member_data.member_location,
						prefix + ss.str());
				t = t->member_data.sib_ptr;
			}
		}
		else
		{
			/* process an array aggregate */
			if (t->head.tag == DW_TAG_array_type)
				dump_value(t->arr_data.subranges, data, dlen, prefix);
			else
			{
				/* an array subrange */
				unsigned int i, sz;
				sz = dtype_access_sizeof((struct dwarf_head_struct *) dtype_access_get_deref_type(t));
				for (i = 0; i <= t->arr_subrange_data.upper_bound; i ++)
				{
					std::stringstream ss;
					ss << "[" << i << "]";
					if (t->arr_subrange_data.sib_ptr)
						dump_value(t->arr_subrange_data.sib_ptr,
								data + i * sz,
								dlen - i * sz,
								prefix + ss.str());
					else
						dump_value(dtype_access_get_deref_type(t),
								data + i * sz,
								dlen - i * sz,
								prefix + ss.str());
				}
			}
		}
	}
	else
		throw(std::string(__func__) + "(): " + "finish this");
}

void c_expr_eval::debug_dump_value(struct eval_node & p)
{
uint8_t * data;
unsigned int dlen;

	if (p.objkind != eval_node::OBJ_DTYPE)
		throw(std::string(__func__) + "(): " + "unsupported object kind");
	fetch_data(p);
	if (p.data == 0)
		data = (uint8_t *) & p.ld, dlen = sizeof p.ld;
	else
		data = p.data.get(), dlen = p.dlen;
	dump_value(p.dtype, data, dlen);
}


std::tr1::shared_ptr<struct data_contents> c_expr_eval::get_scalar_value_1(struct dtype_data * t, uint8_t * data, unsigned dlen)
{
union
{
	int8_t		i8;
	uint8_t		u8;
	int16_t		i16;
	uint16_t	u16;
	int32_t		i32;
	uint32_t	u32;
	int64_t		i64;
	uint64_t	u64;
	float		f;
	double		d;
	long double	ld;

	ARM_CORE_WORD	ptr_val;
}
scalar;
std::stringstream ss;
unsigned int dsize;
std::tr1::shared_ptr<struct data_contents> res(new struct data_contents);

	if (!dtype_access_is_ctype(CTYPE_SCALAR, t))
		throw(std::string(__func__) + "(): " + "bad type: scalar type expected");
	dsize = dtype_access_sizeof((struct dwarf_head_struct *) t);
	if (dlen < dsize)
		throw(std::string(__func__) + "(): " + "insufficient data amount available");

	memset(& scalar, 0, sizeof scalar);
	memcpy(& scalar, data, dlen > sizeof scalar ? sizeof scalar : dlen);

	if (dtype_access_is_ctype(CTYPE_INTEGER, t))
	{
		ss.width(10);
		if (dtype_access_is_ctype(CTYPE_SIGNED, t))
		{
			switch (dsize)
			{
				case sizeof(int8_t): ss << (uint16_t) scalar.i8; break;
				case sizeof(int16_t): ss << scalar.i16; break;
				case sizeof(int32_t): ss << scalar.i32; break;
				case sizeof(int64_t): ss << scalar.i64; break;
				default: throw(std::string(__func__) + "(): " + "unsupported integer data type size");
			}
		}
		else
		{
			/* unsigned integer */
			switch (dsize)
			{
				case sizeof(uint8_t): ss << (uint16_t) scalar.u8; break;
				case sizeof(uint16_t): ss << scalar.u16; break;
				case sizeof(uint32_t): ss << scalar.u32; break;
				case sizeof(uint64_t): ss << scalar.u64; break;
				default: throw(std::string(__func__) + "(): " + "unsupported integer data type size");
			}
		}
	}
	else if (dtype_access_is_ctype(CTYPE_FLOAT, t))
	{
		switch (dsize)
		{
			case sizeof(float): ss << scalar.f; break;
			case sizeof(double): ss << scalar.d; break;
			case sizeof(long double): ss << scalar.ld; break;
			default: throw(std::string(__func__) + "(): " + "unsupported floating point data type size");
		}
	}
	else if (dtype_access_is_ctype(CTYPE_POINTER, t))
	{
		ss << "0x" << std::hex << scalar.ptr_val;
	}
	else
		throw(std::string(__func__) + "(): " + "unknown scalar type");

	res->value = ss.str();
{
	std::stringstream ss;
	struct type_dump_seq seq;
	seq.indent_level = 0;
	seq.is_prefix_printing = true;
	type_data_dump_int(t, false, false, & seq, ss);
	seq.is_prefix_printing = false;
	type_data_dump_int(t, false, false, & seq, ss);

	std::string stype = ss.str();
	std::replace(stype.begin(), stype.end(), '$', '\t');
	std::replace(stype.begin(), stype.end(), '@', '\n');
	res->type = ss.str();
}

	return res;
}

std::tr1::shared_ptr<struct data_contents> c_expr_eval::dump_value_1(struct dtype_data * t, uint8_t * data, unsigned dlen, std::string prefix)
{
	std::stringstream ss;
	struct type_dump_seq seq;
	seq.indent_level = 0;
	seq.is_prefix_printing = true;
	type_data_dump_int(t, false, false, & seq, ss);
	seq.is_prefix_printing = false;
	type_data_dump_int(t, false, false, & seq, ss);

	std::string stype = ss.str();
	std::replace(stype.begin(), stype.end(), '$', '\t');
	std::replace(stype.begin(), stype.end(), '@', '\n');

	std::cout << "type is: " << stype << std::endl;

	t = dtype_access_get_unqualified_base_type(t);

	std::tr1::shared_ptr<struct data_contents> res(new struct data_contents);

	if (dtype_access_is_ctype(CTYPE_SCALAR, t))
	{
		res = get_scalar_value_1(t, data, dlen);
		if (dtype_access_is_ctype(CTYPE_POINTER, t))
			res->node_kind = data_contents::NODE_POINTER;
		else
			res->node_kind = data_contents::NODE_ARITHMETIC;
	}
	else if (dtype_access_is_ctype(CTYPE_AGGREGATE, t))
	{

		res->type = ss.str();
		res->node_kind = data_contents::NODE_STRUCT_OR_UNION;
		if (dtype_access_is_ctype(CTYPE_STRUCT_OR_UNION_OR_CXX_CLASS, t))
		{
			/* process a structure/union/class aggregate */
			t = t->struct_data.data_members;
			while(t)
			{
				std::tr1::shared_ptr<struct data_contents> m;

				m = dump_value_1(t->member_data.member_type,
						data + t->member_data.member_location,
						dlen - t->member_data.member_location,
						prefix + ss.str());

				m->designator = std::string(".") + t->name;
				res->children.push_back(m);

				t = t->member_data.sib_ptr;
			}
		}
		else
		{
			/* process an array aggregate */
			res->node_kind = data_contents::NODE_ARRAY;
			if (t->head.tag == DW_TAG_array_type)
				res = dump_value_1(t->arr_data.subranges, data, dlen, prefix);
			else
			{
				/* an array subrange */
				unsigned int i, sz;
				sz = dtype_access_sizeof((struct dwarf_head_struct *) dtype_access_get_deref_type(t));
				for (i = 0; i <= t->arr_subrange_data.upper_bound; i ++)
				{
					std::stringstream ss;
					ss << "[" << i << "]";
					std::tr1::shared_ptr<struct data_contents> m;

					if (t->arr_subrange_data.sib_ptr)
						m = dump_value_1(t->arr_subrange_data.sib_ptr,
								data + i * sz,
								dlen - i * sz,
								prefix + ss.str());
					else
						m = dump_value_1(dtype_access_get_deref_type(t),
								data + i * sz,
								dlen - i * sz,
								prefix + ss.str());

					m->designator = ss.str();
					res->children.push_back(m);
				}
			}
		}
	}
	else
		throw(std::string(__func__) + "(): " + "finish this");

	return res;
}

std::tr1::shared_ptr<struct data_contents> c_expr_eval::debug_dump_value_1(struct eval_node & p)
{
uint8_t * data;
unsigned int dlen;
std::tr1::shared_ptr<struct data_contents> res;

	if (p.objkind != eval_node::OBJ_DTYPE)
		throw(std::string(__func__) + "(): " + "unsupported object kind");
	fetch_data(p);
	if (p.data == 0)
		data = (uint8_t *) & p.ld, dlen = sizeof p.ld;
	else
		data = p.data.get(), dlen = p.dlen;
	res = dump_value_1(p.dtype, data, dlen);

	return res;
}


