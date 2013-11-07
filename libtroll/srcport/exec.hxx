#ifndef __EXEC_HXX__
#define __EXEC_HXX__


extern "C" {
#include <udis86.h>
#include <libtroll.h>

};

#include "src-cache.hxx"
#include "c-lex-tokens.h"
#include "targetmemcache.hxx"

#include <QTextDocument>
#include <QTextBlockUserData>
#include <QSharedPointer>
#include <QProcess>

#include <dbg-record.hxx>

class XTextDocument : public QTextDocument
{
	Q_OBJECT
private:
	/* this is an array which, for each source code line number
	 * in the document, returns the corresponding block number
	 * in the document; indices start from one - at index one
	 * there is information for source code line number 1 */
	int	* srcline_block_nrs;
	/* the number of source code lines (effectively,
	 * the size of the srcline_block_nrs array above) */
	int	nr_srclines;
public:
	XTextDocument(QObject * parent = 0) : QTextDocument(parent) { srcline_block_nrs = 0; nr_srclines = 0; }
	~XTextDocument(void) { if (srcline_block_nrs) delete[] srcline_block_nrs; }

	/* returns the QTextBlock number in the document
	 * corresponding to the passed source code line
	 * number srcline_nr; if the block cannot be determined
	 * returns -1; the source code line number parameter
	 * passed (srcline_nr) starts counting from 1, 0
	 * for this parameter is not allowed */
	int getBlockNumberForSrclineNumber(int srcline_nr);
};

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


enum ENUM_EXECUTION_REQUEST_TYPE
{
	EXECUTION_REQUEST_INVALID = 0,
	EXECUTION_REQUEST_SINGLE_STEP_SRC,
	EXECUTION_REQUEST_SINGLE_STEP_INSN,
	EXECUTION_REQUEST_STEP_OVER_SRC,
	EXECUTION_REQUEST_STEP_OVER_INSN,
	EXECUTION_REQUEST_CONTINUE,
};


struct SrcUserState
{
	int	is_src	: 1;
	int	has_disassembly	: 1;
	int	is_folded : 1;
	int	is_breakpointed : 1;
	int	srcline_nr	: 24;
};

/* ugly, ugly, ugly... */
union SrcUserUnion
{
	struct SrcUserState state;
	int	x;
};

class SrcUserData : public QTextBlockUserData
{
private:
	union
	{
		int	dis_addr;
		int	line_nr;
	}
	num;
	struct
	{
		int is_folded		: 1;
		int is_breakpointed	: 1;
		int has_disassembly	: 1;
		int is_src		: 1;
	}
	flags;
public:
        //~SrcUserData(void) { qDebug() << "user data destroyed"; }
	SrcUserData(void) :
		QTextBlockUserData()
		{
			num.line_nr = 0;
			flags.is_folded = 0;
			flags.is_breakpointed = 0;
			flags.has_disassembly = 0;
			flags.is_src = 1;
		}
	void setFolded(bool folded) { flags.is_folded = folded ? 1 : 0; }
	bool isFolded(void) { return flags.is_folded ? true : false; }
	void setHasDisassembly(bool hasDisassembly) { flags.has_disassembly = hasDisassembly ? 1 : 0; }
	bool hasDisassembly(void) { return flags.has_disassembly ? true : false; }
	bool isBreakpointed(void) { return flags.is_breakpointed ? true : false; }
	void setBreakpointed(bool breakpointed) { flags.is_breakpointed = breakpointed ? 1 : 0; }
	bool isSrc(void) { return flags.is_src ? true : false; }
	void setIsSrc(bool is_src) { flags.is_src = is_src ? 1 : 0; }
	int disAddr(void) { return num.dis_addr; }
	void setDisAddr(int dis_addr) { num.dis_addr = dis_addr; }
	int lineNumber(void) { return num.line_nr; }
	void setLineNumber(int line_number) { num.line_nr = line_number; }
};


class exec_display_class
{
private:

	enum
	{
		SRCINFO_LINE_BREAKPOINTABLE = 1 << 0,
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
			nr_printed_disasm_lines = 0; }
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
		/*! \todo	remove this, it is evil and useless */
		int * nr_printed_disasm_lines;
		/* the disassembly text data */
		std::list<struct disasm_type_struct *> disasm;
	};

	class src_cache_class_checked	* srcs;
	//struct srcinfo_type_struct * srcinfo;
	class srcinfo_data * srcinfo;

	void build_custom_srcdata(srcinfo_data *srcinfo);
	/* returns true if any modifications were made that requure updating the
	 * source view */
	bool build_disasm_data(struct custom_srcdata * udata, struct disasm_text_node * dislist);

	void win_set_color(enum XCURSES_COLOR_ENUM color);

	struct src_cache_class::srcfile_data * getSrc(const QString & srcfile_name);
        QString		last_srcdir;
        QStringList srcdir_list;

        QProcess	* openocd_disasm_process;

	QSharedPointer<TargetMemCache>	memcache;
	/* this is the udis86 disassembler library object;
	 * udis86 is a very cool library indeed... */
	ud_t		udis86_obj;

public:	
	QSharedPointer<XTextDocument> docFromSrcname(const QString & srcname, bool do_disassemble = true, bool do_fold_disassembly = true);
	QTextDocument * docFromSrcnameOld(const QString & srcname);
	void setTargetMemCache(QSharedPointer<TargetMemCache> memcache) { this->memcache = memcache; }

        void setSrcCache(class src_cache_class_checked * src_cache) { srcs = src_cache; }
	exec_display_class(void);
        ~exec_display_class(void) { delete openocd_disasm_process; delete srcinfo; }
        /* returns true if a document update (docFromSrcname()) is needed */
	bool parse_record_available(srcinfo_data *sinfo);
	class srcinfo_data * get_srcinfo_data(void) { return srcinfo; }
};

#endif /* __EXEC_HXX__ */


