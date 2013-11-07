#include <QMessageBox>
#include <QTextCharFormat>
#include <QPushButton>

#include <queue>
#include <list>
#include <map>
#include <string>
#include <string.h>
#include <malloc.h>

#include <stdio.h>
#include <panic.h>


using namespace std;

#include "src-cache.hxx"
#include "exec.hxx"
#include "c-lex-tokens.h"

#define gwprintf(...)


void exec_display_class::win_set_color(XCURSES_COLOR_ENUM color)
{
QTextCharFormat f(currentCharFormat());

	switch (color)
	{
                default:
                case XCOL_BLACK: f.setForeground(Qt::black); break;
                case XCOL_BLUE: f.setForeground(Qt::blue); break;
                case XCOL_GREEN: f.setForeground(Qt::green); break;
                case XCOL_CYAN: f.setForeground(Qt::cyan); break;
                case XCOL_RED: f.setForeground(Qt::red); break;
                case XCOL_MAGENTA: f.setForeground(Qt::magenta); break;
                case XCOL_YELLOW: f.setForeground(Qt::yellow); break;
                case XCOL_WHITE: f.setForeground(Qt::white); break;
	}
	setCurrentCharFormat(f);
}

void exec_display_class::dump_srclines(void)
{
struct src_cache_class::srcfile_data * src;
int i, j;
enum XCURSES_COLOR_ENUM cur_col;
enum C_LEX_TOKEN_ENUM cur_token;
struct srclist_type_struct * srcdata;
struct custom_srcdata * udata;
QTreeWidgetItem * titem, * dis_titem;
QString item_text;

	src = 0;

	clear();
	tree_widget->clear();

	if (!parse_head)
		return;
	if (parse_head->flags.is_srcfile_valid)
                try
                {
                        src = srcs->get_srcfile(parse_head->srcfile_name);
                }
                catch (const char * s)
                {
                        QMessageBox::critical(0, "file not found", QString(s) + " not found");
                        src = 0;
                }
	if (!src
			|| !parse_head->flags.is_srcfile_valid
			|| !parse_head->flags.is_srcline_valid)
	{
		if (parse_head->flags.is_halt_addr_valid)
			gwprintf("address: 0x%08x\n", parse_head->halt_addr);
		if (parse_head->flags.is_comp_unit_valid)
			gwprintf("compilation unit: %s\n", parse_head->comp_unit_name);
		if (parse_head->flags.is_srcfile_valid)
			gwprintf("source: %s\n", parse_head->srcfile_name);
		if (parse_head->flags.is_subprogram_valid)
			gwprintf("function: %s\n", parse_head->subprogram_name);
		if (parse_head->flags.is_srcline_valid)
			gwprintf("line: %i\n", parse_head->srcline);
		return;
	}

	udata = (struct custom_srcdata *) src->udata;
	if (udata)
	{
		srcdata = udata->srcdata;
	}
	else
		srcdata = 0;

	/* this causes the current token to be always updated on the first
	 * iteration of the loop below */
	cur_token = (C_LEX_TOKEN_ENUM) 0;
	i = 0;

	while (i < src->nr_lines)
	{
		char c;
		char * s, * t;
		enum
		{
			SRCLINE_STATE_NORMAL,
			SRCLINE_STATE_BREAKPOINTABLE,
			SRCLINE_STATE_BREAKPOINTED,
		}
		srcline_state;
		s = src->lines[i];
		t = src->lex_token_lines[i];

#if 1
{
		/* see if the target is halted at the line just to be printed, and
		 * if so - highlight the line by drawing a frame arount it */
		QTextCharFormat f(currentCharFormat());
		if (parse_head->srcline == i + 1)
		{
			////draw_frame_around_srcline(win_line_nr);
			f.setFontWeight(QFont::Bold);
		}
		else
			f.setFontWeight(QFont::Normal);
		setCurrentCharFormat(f);
}
#endif
		////gwprintf("%i\t", i + 1);
		insertPlainText(item_text = QString().number(i + 1) + "\t");
		/* see if the current line is breakpointable */
		srcline_state = SRCLINE_STATE_NORMAL;
		if (srcdata)
		{
			if (udata->srcline_info[i] & SRCINFO_LINE_BREAKPOINTABLE)
				srcline_state = SRCLINE_STATE_BREAKPOINTABLE;
			if (udata->srcline_info[i] & SRCINFO_LINE_BREAKPOINTED)
				srcline_state = SRCLINE_STATE_BREAKPOINTED;
		}
		switch (srcline_state)
		{
			case SRCLINE_STATE_BREAKPOINTABLE:
				if (1) c = '-'; else if (0)
			case SRCLINE_STATE_BREAKPOINTED:
				c = '*';
				////gwprintf(".");
				insertPlainText(".");
				item_text.append(".");
				break;
			default:
				////gwprintf(" ");
				insertPlainText(" ");
				item_text.append(" ");
				c = ' ';
		}
		////gwprintf("%c", c);
		insertPlainText(QChar(c));
		item_text.append(QChar(c));
		while (*s)
		{
			if (*t + C_TOKEN_BASE != cur_token)
				/* choose new color */
				switch (cur_token = (enum C_LEX_TOKEN_ENUM)(*t + C_TOKEN_BASE))
				{
					default:
					case C_TOKEN_INVALID:
						win_set_color(XCOL_BLACK);
						break;
					case C_TOKEN_TYPE_RELATED_KEYWORD:
						win_set_color(XCOL_GREEN);
						break;
					case C_TOKEN_KEYWORD:
						win_set_color(XCOL_YELLOW);
						break;
					case C_TOKEN_IDENTIFIER:
						win_set_color(XCOL_BLACK);
						break;
					case C_TOKEN_CONSTANT:
						win_set_color(XCOL_MAGENTA);
						break;
					case C_TOKEN_STRING_LITERAL:
						win_set_color(XCOL_MAGENTA);
						break;
					case C_TOKEN_PUNCTUATOR:
						win_set_color(XCOL_BLACK);
						break;
					case C_TOKEN_CHAR_CONSTANT:
						win_set_color(XCOL_MAGENTA);
						break;
					case C_TOKEN_COMMENT:
						win_set_color(XCOL_CYAN);
						break;
					case C_TOKEN_PREPROCESSOR:
						win_set_color(XCOL_BLUE);
						break;
				}
			////gwprintf("%c", *s++);
			insertPlainText(QChar(*s));
			item_text.append(QChar(*s));
			s++;
			t++;
		}
		////gwprintf("\n");
		insertPlainText("\n");
		titem = new QTreeWidgetItem;
		titem->setText(1, item_text);
		//titem->setCheckState(0, Qt::Checked);
		tree_widget->addTopLevelItem(titem);
		if (0) {
		QPushButton * b = new QPushButton(this);
		b->setText("b");
		tree_widget->setItemWidget(titem, 0, b);
		}

dump_disassembly:

		/* see if disassembly should be printed */
		if (srcdata && udata->srcline_info[i] & SRCINFO_LINE_BREAKPOINTABLE
				//&& udata->srcline_info[i] & SRCINFO_LINE_DISASM_SHOWN
				&& udata->disasm_addrs)
		{
			/* also dump disassembly */
			int nr_disasm_lines;
			unsigned int * dis_data = udata->disasm_addrs;
			struct disasm_text_node * disnode;
			int j;

			j = i;

			ARM_CORE_WORD addr_hi, addr_low;
			addr_low = srcdata->srcaddr_pairs[dis_data[i]].addr;
			addr_hi = srcdata->srcaddr_pairs[dis_data[i] + 1].addr;
			nr_disasm_lines = addr_hi - addr_low;
			/*! \todo	this is completely wrong */
			if (nr_disasm_lines <= 0)
				panic("");
			if (udata->disasm)
			{
				j = udata->disasm_addrs[i];
				disnode = (struct disasm_text_node*) udata->srcdata->srcaddr_pairs
					[j].pextend;
                                if (udata->nr_printed_disasm_lines[i])
                                        nr_disasm_lines = udata->nr_printed_disasm_lines[i];
			}
			else
				disnode = 0;
			while (nr_disasm_lines--)
			{
				if (disnode)
				{
					if (disnode->dis_type != disasm_text_node::DIS_TYPE_DISASSEMBLY)
					{
						/* most probably dumping multiple machine
						 * code address ranges corresponding to a
						 * single source code line number - skip
						 * to the next line of disassembly */
						j = udata->srcdata->srcaddr_pairs[j].iextend;
						disnode = (struct disasm_text_node*) udata->srcdata->srcaddr_pairs
							[j].pextend;
						while (disnode &&
								disnode->dis_type != disasm_text_node::DIS_TYPE_DISASSEMBLY)
							disnode = disnode->next; 
					}
					if (disnode)
					{
						////gwprintf("0x%08x\t%s\n", disnode->num.addr, disnode->text);
						item_text = QString().number(disnode->num.addr) + "\t" + disnode->text;
                                                item_text = QString("0x") + QString().number(disnode->num.addr, 16) + "\t" + disnode->text;
                                                dis_titem = new QTreeWidgetItem;
						dis_titem->setText(1, item_text);
						titem->addChild(dis_titem);
						insertPlainText(item_text + "\n");

						disnode = disnode->next; 
						if (disnode && disnode->dis_type != disasm_text_node::DIS_TYPE_DISASSEMBLY)
						{
							/* most probably dumping multiple machine
							 * code address ranges corresponding to a
							 * single source code line number - distinguish
							 * such ranges by printing a header line */
							if (!nr_disasm_lines--)
								break;
							////gwprintf("...\n");
							insertPlainText(QString("...\n"));
							dis_titem = new QTreeWidgetItem;
							dis_titem->setText(1, "...");
							titem->addChild(dis_titem);
						}
					}
				}
				else
				{
					////gwprintf("<<< dump disassembly here >>>\n");
					insertPlainText(QString("<<< dump disassembly here >>>\n"));
					dis_titem = new QTreeWidgetItem;
                                        dis_titem->setText(1, QString("0x") + QString().number(addr_low, 16) + "\t<<< disassemble here >>>");
                                        addr_low ++;
					titem->addChild(dis_titem);
				}
			}
		}
		i++;
	}
}

void exec_display_class::update_exec_context_from_backtrace(void)
{
struct stackframe_struct * p;
int i;

	troll_deallocate_record(parse_head);
	parse_head = (exec_type_struct *) troll_allocate_record(TROLL_RECORD_EXEC);
	if (!parse_head)
		panic("");
	p = backtrace->frame_list;
	if (!p)
		panic("");
	i = 0;
	while (i != selected_frame)
		p = p->older, i++;

	parse_head->srcline = p->srcline;

	parse_head->flags.is_halt_addr_valid = p->flags.is_pc_addr_valid;
	parse_head->flags.is_comp_unit_valid = p->flags.is_comp_unit_valid;
	parse_head->flags.is_subprogram_valid = p->flags.is_subprogram_valid;
	parse_head->flags.is_srcfile_valid = p->flags.is_srcfile_valid;
	parse_head->flags.is_srcline_valid = p->flags.is_srcline_valid;
	parse_head->halt_addr = p->pc_addr;
	if (p->flags.is_comp_unit_valid)
		parse_head->comp_unit_name = strdup(p->comp_unit_name);
	if (p->flags.is_subprogram_valid)
		parse_head->subprogram_name = strdup(p->subprogram_name);
	if (p->flags.is_srcfile_valid)
		parse_head->srcfile_name = strdup(p->srcfile_name);

	parse_head->target_state = TARGET_CORE_STATE_HALTED;
}

void exec_display_class::parse_record_available(struct parse_type_common_struct * phead)
{

switch (0)
{
	default:

	if (phead->type == TROLL_RECORD_STACKFRAME)
	{
		struct stackframe_type_struct * tf;
		tf = (struct stackframe_type_struct *) phead;

		if (tf->record_type == stackframe_type_struct::STACKFRAME_RECORD_TYPE_BACKTRACE)
		{
			troll_deallocate_record(backtrace);
			backtrace = (stackframe_type_struct *) troll_clone(tf);
			/*! \todo	this is incorrect */
			selected_frame = 0;
		}
		else if (tf->record_type == stackframe_type_struct::STACKFRAME_RECORD_TYPE_SELECTED_FRAME_NR)
		{
			selected_frame = tf->selected_frame_nr;
			if (!backtrace)
				panic("");
		}
		else
			panic("");
		update_exec_context_from_backtrace();
		dump_srclines();
	}
	else if (phead->type == TROLL_RECORD_EXEC)
	{
		troll_deallocate_record(parse_head);
		parse_head = (exec_type_struct *) troll_clone(phead);
		dump_srclines();
	}
	else if (phead->type == TROLL_RECORD_SRCFILE_LIST)
	{
		if (srclist)
			/* source file list already available - do nothing */
			break;
		build_custom_srcdata((struct srclist_type_struct *)phead);
	}
	else if (phead->type == TROLL_RECORD_BREAKPOINT)
	{
		struct bkpt_type_struct * bkpt = (struct bkpt_type_struct *) phead;
		struct src_cache_class::srcfile_data * src;
		struct custom_srcdata * p;

		if (!bkpt->flags.is_stmt_flag_valid
				|| !(src = srcs->get_srcfile(bkpt->srcfile))
				|| !(p = (struct custom_srcdata *) src->udata))
			break;
		if (bkpt->flags.srcline_valid
				&& (unsigned) (bkpt->srcline - 1) < src->nr_lines)
		{
			switch (bkpt->bkpt_record_type)
			{
				case BKPT_RECORD_BKPT_ADDED:
					p->srcline_info[bkpt->srcline - 1] |= SRCINFO_LINE_BREAKPOINTED;
					break;
				case BKPT_RECORD_BKPT_REMOVED:
					p->srcline_info[bkpt->srcline - 1] &=~ SRCINFO_LINE_BREAKPOINTED;
					break;
				default:
					panic("");
			}
		}
	}
	else if (phead->type == TROLL_RECORD_DISASSEMBLY_LIST)
	{
		struct disasm_type_struct * dis;
		if (!srcs)
			break;
		dis = (struct disasm_type_struct *) phead;
		/* see if the disassembly is referencing
		 * a compilation unit already in the source
		 * cache */
		list<struct src_cache_class::srcfile_data *>::iterator i;
		for (i = srcs->srclist.begin(); i != srcs->srclist.end(); i++)
		{
			struct custom_srcdata * udata;
			udata = (struct custom_srcdata *) ((*i)->udata);
			if (!udata || !udata->srcdata)
				continue;
#if 0
			if (udata->srcdata->low_pc != dis->start_addr
					/*! \todo	fix this
					  || udata->srcdata->hi_pc != dis->first_addr_past_disassembly)
					 */
				)
					continue;
#endif
                        if (udata->srcdata->low_pc > dis->first_addr_past_disassembly
                                || udata->srcdata->hi_pc <= dis->start_addr
                                )
					continue;
#if 0
                        if (udata->disasm)
				/* this can happen when refreshing
				 * a disassembly */
				troll_deallocate_record(udata->disasm);
#endif
			udata->disasm = (disasm_type_struct *) troll_clone(dis);
			build_disasm_data(udata, udata->disasm->disasm_list);
		}
		dump_srclines();
	}
}

}


void exec_display_class::build_custom_srcdata(struct srclist_type_struct * new_srclist)
{
struct srclist_type_struct * s;
int i;
	/* process the list of source code files */
	troll_deallocate_record(srclist);
	srclist = (struct srclist_type_struct *) troll_clone(new_srclist);
	s = srclist;
	while (s)
	{
		struct src_cache_class::srcfile_data * src;
                try
                {
                        src = srcs->get_srcfile(s->srcname);
                }
                catch (const char * s)
                {
                        QMessageBox::critical(0, "file not found", QString(s) + " not found");
                }
                if (src && !src->udata)
		{
			struct custom_srcdata * p = new struct custom_srcdata;
			p->srcdata = s;

#if SGS
			/*! \todo	clean this up */
			comm.miprintf("dis \"%i\", \"%i\"\n", s->low_pc, s->hi_pc);
#endif
                        appendPlainText("dis \"0x" + QString().number(s->low_pc, 16) + "\", \"0x" + QString().number(s->hi_pc, 16) + "\"\n");

			p->srcline_info = new char[src->nr_lines];
			memset(p->srcline_info, 0, src->nr_lines);
			src->udata = p;
			/* if disassembly is appropriate - process it */
			if (s->srcaddr_len)
			{
				p->disasm_addrs = new unsigned int[src->nr_lines];

				memset(p->disasm_addrs, 0, src->nr_lines * sizeof(unsigned int));
				for (i = /* zeroth element reserved/unused */ 1; i < s->srcaddr_len; i++)
				{
					unsigned int j;
					j = s->srcaddr_pairs[i].srcline_nr;
					if (j > src->nr_lines)
						panic("");
					if (!j)
						/* a special case - this is
						 * an end-of-sequence target
						 * core-address-only entry
						 * that is never referenced
						 * directly, but is instead
						 * only used to know when
						 * to stop disassembling -
						 * never put such entries
						 * in the disasm_addrs
						 * table; see comments in
						 * file srcfile.c from
						 * the gear engine source
						 * code package for details */
						continue;
					p->srcline_info[j - 1] |= SRCINFO_LINE_BREAKPOINTABLE;

					/* maintain a list of machine code ranges
					 * corresponding to a source code line */
					if (p->disasm_addrs[j - 1] == 0)
					{
						/* this is the first node on the list -
						 * just initialize the list head index */
						p->disasm_addrs[j - 1] = i;
					}
					else
					{
						/* link this new node at the end of the list */
						/* as each node is added just
						 * once to the lists, loops and
						 * cross-links in the lists
						 * are impossible */
						j = p->disasm_addrs[j - 1];
						while (s->srcaddr_pairs[j].iextend)
						{
							j = s->srcaddr_pairs[j].iextend;
							if (j >= i)
								panic("");
						}
						s->srcaddr_pairs[j].iextend = i;
					}
				}
			}
		}
		s = s->next;
	}
}

void exec_display_class::build_disasm_data(struct custom_srcdata * udata, struct disasm_text_node * dislist)
{
struct disasm_text_node * disnode;
/* counts the number of disassembly lines to print
 * for a given source code line */
int dis_run_len;
int i, t;
struct src_cache_class::srcfile_data * src;
ARM_CORE_WORD last_addr, first_addr;
int xxx;

	src = srcs->get_srcfile(udata->srcdata->srcname);
	if (!src)
		panic("");

#if 0
	if (udata->nr_printed_disasm_lines)
		/* this can happen when refreshing a disassembly -
		 * the disassembly data structures are already built,
		 * but now a disassembly has been received by the gear
		 * engine for the range(s) of addresses of a compilation
		 * unit this module knows about - destroy the old disassembly data
		 * structures and rebuild them based on the more recent
		 * data just received */
		delete[] udata->nr_printed_disasm_lines;
	udata->nr_printed_disasm_lines = new unsigned int[src->nr_lines];
#endif
	if (!udata->nr_printed_disasm_lines)
	{
		/*! \note	the nr_printed_disasm_lines
		 *		can have already been allocated, when
		 *		so, we are just adding new disassembly data
		 *		and not touching the already built data
		 *		structures... */
		udata->nr_printed_disasm_lines = new unsigned int[src->nr_lines];
		memset(udata->nr_printed_disasm_lines, 0, src->nr_lines * sizeof * udata->nr_printed_disasm_lines);
		for (i = 0; i < udata->srcdata->srcaddr_len; i++)
			udata->srcdata->srcaddr_pairs[i].pextend = 0;
	}

	disnode = dislist;
	if (udata->srcdata->srcaddr_len < 2 + /* zeroth element reserved/unused */ 1)
		panic("");
	xxx = 0;
	for (i = 1 /* zeroth element reserved/unused */; i < udata->srcdata->srcaddr_len - 1; i++)
	{
		if (!(t = udata->srcdata->srcaddr_pairs[i].srcline_nr))
			/* skip over end-of-sequence entries */
			continue;
		first_addr = udata->srcdata->srcaddr_pairs[i].addr;
		while (disnode && (disnode->dis_type != disasm_text_node::DIS_TYPE_DISASSEMBLY
					|| disnode->num.addr < first_addr))
			disnode = disnode->next;
		if (!disnode)
			/* nothing more to do */
			break;
		if (disnode->num.addr > first_addr)
			continue;
		if (first_addr < disnode->num.addr)
			continue;
		if (!disnode || disnode->num.addr != first_addr)
			/* impossible; the checks above make this impossible */
			panic("");
		udata->srcdata->srcaddr_pairs[i].pextend = disnode;
		dis_run_len = 0;
		last_addr = udata->srcdata->srcaddr_pairs[i + 1].addr;
		while (disnode)
		{
			if (disnode->dis_type != disasm_text_node::DIS_TYPE_DISASSEMBLY)
			{
				//panic("");
				break;
			}
			if (disnode->num.addr >= last_addr)
				break;
			dis_run_len++;
			disnode = disnode->next;
		}
		if (disnode && disnode->num.addr > last_addr)
			panic("");
		/* here t is strictly positive - already checked above */
		t--;
		if (udata->nr_printed_disasm_lines[t])
			/* multiple machine code ranges corresponding
			 * to a single source code line (e.g. c 'for' loops,
			 * optimized code, etc.) - add one more line (containing
			 * "..."), before each  to distinguish the different machine code
			 * address ranges */
			dis_run_len++;
		/* an increment rather than an assignment is used here
		 * because it is possible that more than one run of
		 * target machine instruction ranges have been generated
		 * for a single source code line (as, for example, is the
		 * case for c 'for' loops, and maybe some optimized code) */
		udata->nr_printed_disasm_lines[t] += dis_run_len;
		xxx += dis_run_len;
	}
	udata->xxx = 0 * xxx;
}

exec_display_class::exec_display_class(void) :
        srcs(new class src_cache_class_checked()),
        parse_head(0),
        backtrace(0),
        last_src(0),
        srclist(0),
        QPlainTextEdit(),
        tree_widget(new QTreeWidget(0))
{
        setReadOnly(true);
        tree_widget->show();
        tree_widget->setColumnCount(2);
        tree_widget->setColumnWidth(0, 20);
}
