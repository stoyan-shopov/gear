/* note: this may well be the ugliest and dirtiest source file in the frontend... */

#include <queue>
#include <list>

using namespace std;

#include "engine-common.hxx"
#include "frontend-common.hxx"
#include "evt.hxx"
#include "window.hxx"
#include "err.hxx"
#include "mi-parse.hxx"

#include "vardisplay-parse.tab.hh"
#include "vardisplay.hxx"

#include "../widgets/textedit/textedit.hxx"


/* constructor */
vardisplay_display_class::vardisplay_display_class(int ncols, int nlines, int startx, int starty, class window_class * parent)
     : window_class(ncols, nlines, startx, starty, parent, WIN_TYPE_VARDISPLAY), parse_head(0),
	is_waiting_var_response(false)
{
	set_total_nr_lines(0);
}

bool vardisplay_display_class::is_folded(void)
{
int i;
	i = fold_addr >> 3;
	return (fold_flags[i] & (1 << (fold_addr & 7))) ? true : false;
}

inline void vardisplay_display_class::print_value(void) 
{
	if (var_data)
		/* data is available only for non-type-only expression data */
		gwprintf("\t\"%s\"", var_data[atom_nr]);
}

void vardisplay_display_class::dump_array(struct var_typedef * t, struct var_typedef * stride, bool is_there_more_siblings)
{
int i;
enum FOLD_ICON_ENUM ficon;
static char arr_idx_str[16];

	varname_list.push_back("[");
	for (i = 0; i <= stride->upper_bound; i++, varname_list.pop_back(), varname_list.pop_back())
	{
		snprintf(arr_idx_str, sizeof arr_idx_str, "%i", i);
		varname_list.push_back(arr_idx_str);
		varname_list.push_back("]");
		if (stride->arrdim_next)
		{
			ficon = FOLD_ICON_UNFOLDED;

			gwprintf("array entry [%i]", i);

			gwprintf("\n");
			line_nr++;

			if (is_folded())
			{
				fold_addr += stride->nr_arr_folds;
				deref_addr += stride->nr_arr_derefs;
				atom_nr += stride->nr_arr_atoms;
			}
			else
			{
				fold_addr++;
				idx -= 4;
				dump_array(t, stride->arrdim_next, i < stride->upper_bound);
			}
			idx -= 4;
		}
		else
		{
			if (t->flags.deref_point)
			{
				gwprintf("%s ", t->type_name);
				if (t->name)
					gwprintf("%s", t->name);
				gwprintf("[ ] array entry [%i]", i);
				print_value();
				gwprintf("\n");
				line_nr++;
				if (!is_folded())
				{
					/* a dereference has been requested - see
					 * if the dereferenced data object data
					 * has already been retrieved (by a previous
					 * query to the gear engine)
					 */
					if (!deref_items[deref_addr])
					{
						/* no - query the gear engine
						 * about the dereferenced data object
						 * and prepare to wait for a response */
						is_waiting_var_response = true;
						expected_response_token = deref_items + deref_addr;
						comm.miprintf("0x%x whatis ", (int) expected_response_token);
						dump_varname();
						/* dereference pointers by using
						 * the array subscript operator ([]),
						 * instead of the dereference (*) operator as
						 * it is easier to print via the machine
						 * interface */
						comm.miprintf("[0]\n");
					}
					else
					{
						/* the dereference data is already here */
					unsigned char	* saved_fold_flags = fold_flags;
					char ** saved_var_data = var_data;
					int saved_fold_addr = fold_addr, saved_deref_addr = deref_addr;
					int saved_atom_nr = atom_nr;
					struct vardisplay_type_struct ** saved_deref_items = deref_items;

						fold_addr = 0;
						deref_addr = 0;
						fold_flags = saved_deref_items[saved_deref_addr]->fold_flags;
						atom_nr = 0;
						var_data = saved_deref_items[saved_deref_addr]->valarray;
						deref_items = saved_deref_items[saved_deref_addr]->deref_items;
						varname_list.push_back("[0]");
						dump_type(saved_deref_items[saved_deref_addr]->vartype);
						idx -= 4;
						varname_list.pop_back();

						fold_flags = saved_fold_flags;
						var_data = saved_var_data;
						fold_addr = saved_fold_addr;
						deref_addr = saved_deref_addr;
						atom_nr = saved_atom_nr;
						deref_items = saved_deref_items;
					}
				}
				atom_nr++;
				idx -= 4;

				fold_addr++;
				deref_addr++;
				continue;
			}
			else
			{
				ficon = FOLD_ICON_NONE;
				if (t->children)
				{
					ficon = FOLD_ICON_UNFOLDED;
					if (is_folded())
						ficon = FOLD_ICON_FOLDED;
				}

			}

			gwprintf("%s ", t->type_name);
			if (t->name)
				gwprintf("%s", t->name);
			gwprintf("[ ] array entry [%i]", i);

			if (t->children)
			{
				gwprintf("\n");
				line_nr++;
				if (is_folded())
				{
					fold_addr += stride->nr_arr_folds;
					deref_addr += stride->nr_arr_derefs;
					atom_nr += t->nr_atoms;
				}
				else
				{
				struct var_typedef * child;
					fold_addr++;
					idx -= 4;

					child = t->children;
					varname_list.push_back(".");
					while (child)
					{
						dump_type(child);
						child = child->next;
					}
					idx -= 4;
					varname_list.pop_back();
				}
			}
			else
			{
				print_value();
				atom_nr++;
				gwprintf("\n");
				line_nr++;
			}
			idx -= 4;
		}
	}
	varname_list.pop_back();
}

void vardisplay_display_class::dump_type(struct var_typedef * t)
{
enum FOLD_ICON_ENUM ficon;

	if (!t)
	{
		gwprintf("no variable data available");
		return;
	}

	if (t->name)
		/* the name will not be present in the case
		 * when processing a dereferenced pointer data
		 * object, nested in another data object - in this
		 * case the name will have been removed by the
		 * upper data object handling logic so that
		 * the complete data object name be easily
		 * constructible */
		varname_list.push_back(t->name);

	if (t->flags.array)
	{
		struct var_typedef * p;

		if (!t->arrdim_next)
			panic("");

		gwprintf("%s ", t->type_name);
		if (t->name)
			gwprintf("%s", t->name);
		gwprintf("[] array");

		p = t->arrdim_next;
		while (p)
		{
			gwprintf("[%i]", p->upper_bound + 1); 
			p = p->arrdim_next;
		}
		gwprintf("\n");
		line_nr++;

		if (is_folded())
		{
			fold_addr += t->nr_arr_folds;
			deref_addr += t->nr_arr_derefs;
			atom_nr += t->nr_arr_atoms;
		}
		else
		{
			fold_addr++;
		}
		if (t->name)
			/* the name will not be present in the case
			 * when processing a dereferenced pointer data
			 * object, nested in another data object - in this
			 * case the name will have been removed by the
			 * upper data object handling logic so that
			 * the complete data object name be easily
			 * constructible */
			varname_list.pop_back();
		return;
	}
	if (t->flags.deref_point)
	{
		if (is_folded())
			ficon = FOLD_ICON_FOLDED;
		else
			ficon = FOLD_ICON_UNFOLDED;

		gwprintf("%s ", t->type_name);
		if (t->name)
			gwprintf("%s", t->name);

		print_value();
		gwprintf("\n");
		line_nr++;
		if (!is_folded())
		{
			/* a dereference has been requested - see
			 * if the dereferenced data object data
			 * has already been retrieved (by a previous
			 * query to the gear engine)
			 */
			if (!deref_items[deref_addr])
			{
				/* no - query the gear engine
				 * about the dereferenced data object
				 * and prepare to wait for a response */
				is_waiting_var_response = true;
				expected_response_token = deref_items + deref_addr;
				comm.miprintf("0x%x whatis ", (int) expected_response_token);
				dump_varname();
				/* dereference pointers by using
				 * the array subscript operator ([]),
				 * instead of the dereference (*) operator as
				 * it is easier to print via the machine
				 * interface */
				comm.miprintf("[0]\n");
			}
			else
			{
				/* the dereference data is already here */
			unsigned char	* saved_fold_flags = fold_flags;
			char ** saved_var_data = var_data;
			int saved_fold_addr = fold_addr, saved_deref_addr = deref_addr;
			int saved_atom_nr = atom_nr;
			struct vardisplay_type_struct ** saved_deref_items = deref_items;

				fold_addr = 0;
				deref_addr = 0;
				fold_flags = saved_deref_items[saved_deref_addr]->fold_flags;
				atom_nr = 0;
				var_data = saved_deref_items[saved_deref_addr]->valarray;
				deref_items = saved_deref_items[saved_deref_addr]->deref_items;
				varname_list.push_back("[0]");
				dump_type(saved_deref_items[saved_deref_addr]->vartype);
				idx -= 4;
				varname_list.pop_back();

				fold_flags = saved_fold_flags;
				var_data = saved_var_data;
				fold_addr = saved_fold_addr;
				deref_addr = saved_deref_addr;
				atom_nr = saved_atom_nr;
				deref_items = saved_deref_items;
			}
		}

		atom_nr++;
		fold_addr++;
		deref_addr++;
		if (t->name)
			/* the name will not be present in the case
			 * when processing a dereferenced pointer data
			 * object, nested in another data object - in this
			 * case the name will have been removed by the
			 * upper data object handling logic so that
			 * the complete data object name be easily
			 * constructible */
			varname_list.pop_back();
		return;
	}
	else
	{
		ficon = FOLD_ICON_NONE;
		if (t->children)
		{
			ficon = FOLD_ICON_UNFOLDED;
			if (is_folded())
				ficon = FOLD_ICON_FOLDED;
		}
	}

	gwprintf("%s ", t->type_name);
	if (t->name)
		gwprintf("%s", t->name);

	if (t->children)
	{
		gwprintf("\n");
		line_nr++;
		if (is_folded())
		{
			fold_addr += t->nr_folds;
			atom_nr += t->nr_atoms;
		}
		else
		{
			struct var_typedef * child;
			fold_addr++;

			child = t->children;
			varname_list.push_back(".");
			while (child)
			{
				dump_type(child);
				child = child->next;
			}
			idx -= 4;
			varname_list.pop_back();
		}
	}
	else
	{
		print_value();
		atom_nr++;
		gwprintf("\n");
		line_nr++;
	}
	if (t->name)
		/* the name will not be present in the case
		 * when processing a dereferenced pointer data
		 * object, nested in another data object - in this
		 * case the name will have been removed by the
		 * upper data object handling logic so that
		 * the complete data object name be easily
		 * constructible */
		varname_list.pop_back();
}


/***************************************************************
 ***************************************************************
 ***************************************************************
 ***************************************************************
 ***************************************************************/

void vardisplay_display_class::refresh(void)
{
	/*! \todo	this must be finer-grained */
	if (is_waiting_var_response)
		return;
	init_line();

	if (!parse_head)
	{
		set_total_nr_lines(1);
		gwprintf("no expression\n");
		return;
	}

	focus_line = window_class::focus_line;
	/* initialize */
	fold_addr = 0;
	deref_addr = 0;
	fold_flags = parse_head->fold_flags;
	idx = 0;
	line_nr = 0;
	atom_nr = 0;
	var_data = parse_head->valarray;
	deref_items = parse_head->deref_items;
	if (!varname_list.empty())
		panic("");
	dump_type(parse_head->vartype);
	if (!varname_list.empty())
		panic("");

	/* !!! it is very important that this is here, below
	 * the dump_type() call above, otherwise folding will not work */
	toggle_fold = false;

	set_total_nr_lines(line_nr);
}

void vardisplay_display_class::event_handler(const struct event * evt)
{
struct event evt_gen;

	switch (evt->code)
	{
		case EVENT_KEYPRESSED:
			if (is_waiting_var_response)
				/* do not allow interaction when
				 * waiting for a gear engine response */
				break;
			if (!has_focus())
				break;
			switch (evt->param1)
			{
				/* HACK HACK HACK */
				case ' ':
					toggle_fold = true;
					evt_gen.code = EVENT_WIN_REFRESH;
					evt_gen.param1 = (int) (window_class *) this;
					event_dispatcher.enque(evt_gen);
					break;
				case 'k':
					if (focus_line)
					{
						move_focus_line(-1);
						evt_gen.code = EVENT_WIN_REFRESH;
						evt_gen.param1 = (int) (window_class *) this;
						event_dispatcher.enque(evt_gen);
					}
					break;
				case 'j':
					if (line_nr && focus_line < line_nr - 1)
					{
						move_focus_line(1);
						evt_gen.code = EVENT_WIN_REFRESH;
						evt_gen.param1 = (int) (window_class *) this;
						event_dispatcher.enque(evt_gen);
					}
					break;
				default: window_class::event_handler(evt); break;
			}
			break;
		case EVENT_GEAR_ENGINE_PARSE_RECORD_AVAILABLE:
			if (evt->param1 == PARSE_RECORD_VARDISPLAY_INFO)
			{
				/* check for errors */
				struct vardisplay_type_struct * p = (struct vardisplay_type_struct *) evt->param2;

				toggle_fold = false;

				if (p->err.errcode != GEAR_ERR_NO_ERROR)
				{
					/* display error message in a window */
					class textedit_class * te = new class textedit_class;
					te->append(p->err.err_str);
					break;
				}
				/* fold everything in the view */
				memset(p->fold_flags, 0xff, p->fold_flags_len);
				if (is_waiting_var_response)
				{
					/* sanity checks
					 *
					 * \todo	this is incorrect - debugging only */
					if ((int) expected_response_token != *p->tokens)
					{
						printf("unexpected token: %i\n", p->tokens);
						panic("");
					}
					is_waiting_var_response = false;
					p->clone_to(*expected_response_token = new struct vardisplay_type_struct);
					/* here, a dereferenced pointer data
					 * object, nested in another data object,
					 * is being handled - in this
					 * case the name used to access this data
					 * object is removed so that
					 * the complete data object name be easily
					 * constructible by the variable displaying code
					 * in function dump_type() */
					free((*expected_response_token)->vartype->name);
					(*expected_response_token)->vartype->name = 0;
					expected_response_token = 0;
				}
				else
				{
					if (parse_head)
						parse_head->destroy_self();
					parse_head = new struct vardisplay_type_struct;
					p->clone_to(parse_head);
				}
				/* refresh */
				evt_gen.code = EVENT_WIN_REFRESH;
				evt_gen.param1 = (int) (window_class *) this;
				event_dispatcher.enque(evt_gen);

			}
			break;
		default:
			window_class::event_handler(evt);
			break;
	}
}

