/*! \todo	document this */	 


/* expression printing flags */	 


/*! expression evaluation and printing flags */
struct expr_eval_flags
{
	/*! request the type of the expression to be printed */
	bool	print_type	: 1;
	/*! request the value of the expression to be printed */
	bool	print_val	: 1;
};

void c_expression_eval_and_print(struct gear_engine_context * ctx, const char * exp_str, struct expr_eval_flags flags);
bool c_expression_eval_int(struct gear_engine_context * ctx, const char * exp_str, signed long long * val);

