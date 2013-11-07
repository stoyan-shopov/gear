extern "C"
{
#include <libdwarf.h>
#include "dwarf-common.h"
#include "gear-engine-context.h"
#include "engine-err.h"
#include "../engine/dwarf-expr.h"
#include "../engine/dwarf-loc.h"
#include "../engine/subprogram-access.h"
#include "../engine/dobj-access.h"
#include "../engine/type-access.h"
#include "c-parse-types.h"
#include "../engine/scope.h"
#include "../engine/dwarf-util.h"
#include "../engine/core-access.h"

#include <string.h>

};

#include <string>
#include <sstream>
#include <tr1/memory>
#include <vector>


struct data_contents
{
	std::string	designator;
	std::string	value;
	std::string	type;
	enum
	{
		NODE_INVALID	= 0,
		NODE_ARRAY,
		NODE_STRUCT_OR_UNION,
		NODE_ARITHMETIC,
		NODE_POINTER,
	}
	node_kind;
	std::vector<std::tr1::shared_ptr<struct data_contents> >	children;
};


struct eval_node
{
	enum OBJKIND
	{
		OBJ_INVALID	= 0,
		OBJ_DTYPE,
		OBJ_SUBPROGRAM,
		/* OBJ_DOBJ, */
	}
	objkind;
	union
	{
		struct dtype_data	* dtype;
		struct subprogram_data	* subp;
	};
	struct
	{
		bool	is_addr_applicable	: 1;
		bool	is_val_applicable	: 1;
		bool	is_addr_computed	: 1;
		bool	is_val_computed		: 1;
		struct dwarf_location	location;
	};
	uint32_t	addr;
	/* the length of the data buffer 'data' below */
	int dlen;
	std::tr1::shared_ptr<uint8_t>	data;
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
	};

	eval_node(void) { memset(this, 0, sizeof * this); }
	eval_node(enum OBJKIND kind) { memset(this, 0, sizeof * this); objkind = kind; }
};

class c_expr_eval
{
private:
	/* if true, the expression is being evaluated for its value; otherwise, it is being evaluated for its type */
	bool is_computing_value;
	enum
	{
		/*! maximum number of array dimensions supported by the type name building routine 'build_type_name()' */
		MAX_ARRAY_DIMENSIONS		= 16,
	};
	void type_data_dump_int(struct dtype_data * type, bool is_prev_deref, bool is_in_deref, struct type_dump_seq * seq, std::stringstream & ss);

private:

	void array_fixup(struct eval_node & p);
	int64_t get_int(struct eval_node & p);
	long double get_float(struct eval_node & p);

	unsigned int get_sizeof(struct eval_node & p);

	ARM_CORE_WORD get_addr(struct eval_node & p);
	void fetch_data(struct eval_node & p);
	struct gear_engine_context * ctx;

	struct eval_node resolve_id(const char * id);

	bool is_int_obj(struct eval_node & p);
	bool is_signed_int_obj(struct eval_node & p);
	bool is_unsigned_int_obj(struct eval_node & p);
	bool is_float_obj(struct eval_node & p);
	bool is_arith_obj(struct eval_node & p);
	bool is_scalar_obj(struct eval_node & p);
	bool is_struct_obj(struct eval_node & p);
	bool is_array_obj(struct eval_node & p);
	bool is_aggregate_obj(struct eval_node & p);
	bool is_ptr_obj(struct eval_node & p);
	bool is_func(struct eval_node & p);
	bool is_lvalue(struct eval_node & p) { return p.is_addr_applicable != false; }

	bool is_ptr_to_incomplete_or_void_obj(struct eval_node & p);

	bool is_nonzero(struct eval_node & p);

	struct eval_node make_float_obj(void);
	struct eval_node make_ptr_obj(struct eval_node & p);
	struct eval_node make_signed_int_obj(void);

	void make_nonlvalue(struct eval_node & p) { p.is_addr_applicable = false; }
	void make_lvalue(struct eval_node & p) { p.is_addr_applicable = true; p.is_val_applicable = true; }

	long double get_long_double(struct eval_node & p);
	ARM_CORE_WORD get_ptr_val(struct eval_node & p);

	struct eval_node get_deref_obj(struct eval_node & p) { struct eval_node x(p); x.dtype = dtype_access_get_deref_type(p.dtype); x.is_addr_applicable = true; if (is_computing_value) { x.addr = get_ptr_val(p); x.is_addr_computed = true; } x.is_val_computed = false; make_lvalue(x); return x; }
	struct eval_node get_struct_member_obj(struct eval_node & p, const char * id);

	struct eval_node get_arith_superior(struct eval_node & x1, struct eval_node & x2);

	bool are_types_equivalent(struct eval_node & x1, struct eval_node & x2) { return dtype_access_are_types_compatible(x1.dtype, x2.dtype); }

	struct eval_node ptr_arith_add(struct eval_node & n1, struct eval_node & n2);

	struct eval_node eval_type_primary_expr(struct primary_expr * p);
	struct eval_node eval_type_postfix_expr(struct postfix_expr * p);
	struct eval_node eval_type_arg_expr_list(struct arg_expr_list * p);
	struct eval_node eval_type_unary_expr(struct unary_expr * p);
	struct eval_node eval_type_cast_expr(struct cast_expr * p);
	struct eval_node eval_type_mult_expr(struct mult_expr * p);
	struct eval_node eval_type_additive_expr(struct additive_expr * p);
	struct eval_node eval_type_shift_expr(struct shift_expr * p);
	struct eval_node eval_type_rel_expr(struct rel_expr * p);
	struct eval_node eval_type_eq_expr(struct eq_expr * p);
	struct eval_node eval_type_and_expr(struct and_expr * p);
	struct eval_node eval_type_xor_expr(struct xor_expr * p);
	struct eval_node eval_type_incl_or_expr(struct incl_or_expr * p);
	struct eval_node eval_type_logical_and_expr(struct logical_and_expr * p);
	struct eval_node eval_type_logical_or_expr(struct logical_or_expr * p);
	struct eval_node eval_type_cond_expr(struct cond_expr * p);
	struct eval_node eval_type_type_spec(struct type_spec * p);
	struct eval_node eval_type_type_spec_list(struct type_spec_list * p);
	struct eval_node eval_type_pointer_spec(struct pointer_spec * p);
	struct eval_node eval_type_type_name(struct type_name * p);
	struct eval_node eval_type_abstract_decl(struct abstract_decl * p);
	struct eval_node eval_type_direct_abstract_decl(struct direct_abstract_decl * p);

	struct eval_node eval_type_expr(struct expr * p);


	/*
	 *
	 * target data cooking routines
	 *
	 */
	std::string get_scalar_value(struct dtype_data * t, uint8_t * data, unsigned dlen);
	void dump_value(struct dtype_data * t, uint8_t * data, unsigned dlen, std::string prefix = std::string(""));

	std::tr1::shared_ptr<struct data_contents> get_scalar_value_1(struct dtype_data * t, uint8_t * data, unsigned dlen);
	std::tr1::shared_ptr<struct data_contents> dump_value_1(struct dtype_data * t, uint8_t * data, unsigned dlen, std::string prefix = std::string(""));

public:
	struct eval_node compute_value(struct expr * p) { struct eval_node n; is_computing_value = true; n = eval_type_expr(p); fetch_data(n); return n; }
	struct eval_node evaluate_type(struct expr * p) { is_computing_value = false; return eval_type_expr(p); }

	void debug_dump_value(struct eval_node & p);
	std::tr1::shared_ptr<struct data_contents> debug_dump_value_1(struct eval_node & p);

	c_expr_eval(struct gear_engine_context * ctx) { this->ctx = ctx; }

};

