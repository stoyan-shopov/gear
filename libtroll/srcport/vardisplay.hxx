
class vardisplay_display_class : public window_class
{
private:
	/* various variables used when printing/processing a variable
	 * expression */
	unsigned char	* fold_flags;
	char ** var_data;
	int fold_addr, deref_addr;
	int atom_nr;
	int line_nr;
	int focus_line;
	bool toggle_fold;

	enum FOLD_ICON_ENUM 
	{
		FOLD_ICON_INVALID = 0,
		FOLD_ICON_NONE,
		FOLD_ICON_FOLDED,
		FOLD_ICON_UNFOLDED,
	};
	int	idx;
	char header[1024];

	struct vardisplay_type_struct * parse_head, ** deref_items;
	/* this is the list that is used to build an lvalue
	 * expression to query the gear engine when dereferencing
	 * pointers ('unfolding' pointer nodes) in the frontend */
	list<string>	varname_list;
	bool	is_waiting_var_response;
	struct	vardisplay_type_struct	** expected_response_token;

private:

	class comm_class	* comm_object;
	inline void dump_type_header(void);
	inline void dump_type_fold_icon(enum FOLD_ICON_ENUM t);
	inline void put_margin(int is_vline_visible, char pseudographic_sym = '|');
	inline void print_value(void); 
	void dump_array(struct var_typedef * t, struct var_typedef * stride, bool is_there_more_siblings);
	void dump_type(struct var_typedef * t);
	bool is_folded(void);
	void dump_varname(void) { for (list<string>::iterator i = varname_list.begin(); i != varname_list.end(); i++) comm.miprintf("%s", (*i).c_str()); }

	class yy_var::parser * parser;

public:	
	vardisplay_display_class(int ncols, int nlines, int startx, int starty, class window_class * parent);
	void refresh(void);
	void event_handler(const struct event * evt);
};

