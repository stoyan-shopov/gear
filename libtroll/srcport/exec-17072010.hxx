extern "C" {
#include <libtroll.h>
};

#include "src-cache.hxx"
#include "c-lex-tokens.h"

#include <QPlainTextEdit>
#include <QIcon>
#include <QLabel>

/* ega color table */
enum XCURSES_COLOR_ENUM
{
	XCOL_BLACK	= 0,
	XCOL_BLUE	= 1,
	XCOL_GREEN	= 2,
	XCOL_CYAN	= 3,
	XCOL_RED	= 4,
	XCOL_MAGENTA	= 5,
	XCOL_YELLOW	= 6,
	XCOL_WHITE	= 7,
	XCOL_NR_COLORS	= 8,
};


class exec_display_class : public QPlainTextEdit
{
	Q_OBJECT
private:

	enum
	{
		SRCINFO_LINE_BREAKPOINTABLE = 1 << 0,
		SRCINFO_LINE_BREAKPOINTED = 1 << 1,
		SRCINFO_LINE_DISASM_SHOWN = 1 << 3,
	};
	/* outline of this structure's use:
	 *
	 * last_src	--->	nr_lines - number of source code
	 * 				text lines in the current file
	 * 			udata ---> points to this custom_srcdata
	 * 				structure, which contains:
	 *
	 * srcdata	--->	breakpointable lines bitmap, compilation
	 * 			unit address range(s), a list of
	 * 			subprograms in the compilation unit,
	 * 			and the srcaddr_pairs[] array:
	 *
	 * 				srcaddr_pairs[...] - contains
	 * 					the correspondence
	 * 					between source code
	 * 					line numbers - machine code
	 * 					addresses - disassembled
	 * 					instruction text; these here
	 *					are organized in an in-array
	 *					list - the list nodes contain
	 *					pointers to the disassembly
	 *					text corresponding to core addresses,
	 *					and is sorted by ascending
	 *					values of core addresses
	 *
	 * srcline_info	--->	[...] - for each line, contains a bitmap,
	 *				denoting if:
	 *				- the source code line is
	 *				breakpointable
	 *				- if there is a breakpoint
	 *				on this line
	 *				- if the breakpoint is on the
	 *				start of machine code generated
	 *				for the source code line
	 *				- the disassembly view for
	 *				this line is folded/unfolded
	 * disasm_addrs	--->	[...] - for each source code line,
	 *				contains an index in the
	 *				srcaddr_pairs array above -
	 *				this index is the head of the
	 *				list for machine code ranges
	 *				generated for this line (there
	 *				could be multiple of these, e.g.
	 *				in optimized code and c 'for' loops)
	 * nr_printed_disasm_lines ---> [...] - containing the number of
	 *					disassembly text lines printed for
	 *					unfolded source code lines;
	 *					used for various computations
	 *					when printing text
	 * disasm	--->	disassembly data, not used directly, but
	 *			rather indexed thru some of the fields(arrays)
	 *			already described */
	struct custom_srcdata
	{
		custom_srcdata(void) { srcdata = 0; srcline_info = 0; disasm_addrs = 0;
			nr_printed_disasm_lines = 0; xxx = 0; }
		struct srclist_type_struct * srcdata;
		/* this array holds a byte for each source code line
		 * in a source code file, this is used for various
		 * purposes, such as recording if there is a breakpoint
		 * at a given source code line number; the number of
		 * elements in this array equals the number of source
		 * code lines, and is not recorded here (it is to be
		 * found in the srcfile_data structure normally containing
		 * this custom_srcdata structure)
		 *
		 * the uses so far are:
		 * bit0 - 1, if the line is breakpointable
		 * bit1 - if 1, there is a breakpoint on this line
		 * bit2 - if 1, the breakpoint is on the exact
		 *	start of the machine code generated for
		 *	some source code construct (i.e. is not on
		 *	a core address that lies strictly inside a
		 *	machine code sequence generated for some
		 *	source code construct)
		 * bit3 - if 1, the source code line has machine
		 *	code generated for it, and the line 
		 *	is curently 'unfolded' for disassembly
		 *	viewing - that is, disassembly should be printed
		 *	for this line
		 *
		 *	\todo	maybe make this a bitfield
		 */
		char * srcline_info;
		/* this array holds - for each source code
		 * line number, an index in the srcdata->srcaddr_pairs
		 * field (defined above) for the corresponding
		 * core address of machine code generated for
		 * this line number; the indices start from 1
		 * (and thus must be decremented by one prior to
		 * their use), an index equal to zero means
		 * there is no machine code generated for
		 * the corresponding source code line number;
		 * whether an entry is valid can also be determined
		 * by inspecting the SRCINFO_LINE_BREAKPOINTABLE
		 * field in the corresponding entry in the
		 * srcline_info array above */
		unsigned int * disasm_addrs;
		/* this array holds - for each source code
		 * line number, the number of lines of the
		 * corresponding disassembly dump (if appropriate);
		 * used for various source view window line
		 * numbering computations; lines for which no
		 * disassembly is available should hold zero here;
		 * whether an entry is valid can also be determined
		 * by inspecting the SRCINFO_LINE_BREAKPOINTABLE
		 * field in the corresponding entry in the
		 * srcline_info array above */
		int * nr_printed_disasm_lines;
		/* the disassembly text data */
		std::list<struct disasm_type_struct *> disasm;
		int xxx;
	};

	struct exec_type_struct * exec_status;
	struct stackframe_type_struct * backtrace;
	int selected_frame;
	/* a flag, when true, the refresh routine shall
	 * recompute the first line visible so that
	 * the source line at which the target is halted
	 * will be visible; this is mainly needed to
	 * support scrolling the source code text buffer */
	bool recalc_line;

	class src_cache_class_checked	* srcs;
	struct src_cache_class::srcfile_data * last_src;
	void update_exec_context_from_backtrace(void);
	void build_custom_srcdata(struct srclist_type_struct * srclist);
	void build_disasm_data(struct custom_srcdata * udata, struct disasm_text_node * dislist);
	int count_lines_before_srcline(int srcline_nr, struct custom_srcdata * udata);

        void dump_srclines(struct src_cache_class::srcfile_data * src = 0, bool force_update = false);
	struct srclist_type_struct * srclist;
	void win_set_color(enum XCURSES_COLOR_ENUM color);

public slots:
	void displaySourceFile(const QString & srcname);
private:
	QIcon		icon_bkpt_set;
	QIcon		icon_bkpt_disabled;
	QLabel		* margin_area;
	QPixmap		* margin_pixmap;
        QTextCharFormat	disasm_format, src_format;

protected:
	void keyPressEvent(QKeyEvent *e);
	void paintEvent1(QPaintEvent *e);
	void resizeEvent(QResizeEvent *e);

public:	
	exec_display_class(QWidget * parent = 0);
	void parse_record_available(struct parse_type_common_struct * phead);
};

