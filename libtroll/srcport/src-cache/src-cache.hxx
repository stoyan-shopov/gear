#ifndef __SRC_CACHE_HXX__
#define __SRC_CACHE_HXX__

#include <list>
#include <map>
#include <string>

#include <malloc.h>

/*! \todo	make this runtime adjustable - the
 * 		number of space characters in a tab
 * 		character */
#define NR_SPACES_IN_A_TAB		8

class src_cache_class
{
public:
/*! source file access helper data structure
 *
 * this structure provides quick access to a source file
 * by line number
 */
struct srcfile_data
{
	srcfile_data(void) { textbuf = tokenbuf = 0; textbuf_size = nr_lines = 0;
				lines = lex_token_lines = 0; udata = 0; }
	~srcfile_data(void)
	{
		if (textbuf) free(textbuf);
		if (lines) free(lines);
		if (lex_token_lines) free(lex_token_lines);
	}
	/*! buffer holding the source file text
	 *
	 * this holds a verbatim copy of the contents of the source file
	 * with the exception that newlines are replaced with null byte terminators;
	 * this is because the line numbers buffer ::lines below points to locations within this
	 * buffer, which are null terminated strings corresponding to the line numbers */
	char	* textbuf;
	/*! buffer holding the source file lexical tokenization data
	 *
	 * only c is supported right now; this holds a lexical token enumeration
	 * constant (constants defined in file c-lex-tokens.h) corresponding
	 * to each character in the source code file; this is used for syntax
	 * highlighting the source code when displaying it */
	char	* tokenbuf;
	/*! size of textbuf (and tokenbuf), in bytes */
	unsigned int	textbuf_size;
	/*! the line numbers buffer
	 *
	 * there is an entry here for each line in the source file, and it is a pointer
	 * to some location within the ::textbuf above, which is a null terminated string
	 * corresponding to the source code text for the appropriate line number; lines here
	 * start from zero
	 *
	 * \todo	maybe start lines from index 1 */
	char	**lines;
	/*! the source code lines lexical tokens buffers
	 *
	 * there is an entry here for each line in the source file, and it is a pointer
	 * to some location within the ::tokenbuf above; this is used for syntax
	 * highlighting the source code when displaying it */
	char ** lex_token_lines;
	/*! the number of elements in the ::lines buffer (and the ::lex_token_lines buffer); also, the number of source lines in the described source file */
	unsigned int	nr_lines;
	/*! a generic data field available to the user of this object */
	void	* udata;
};
	class std::list<struct srcfile_data *> srclist;
protected:
	src_cache_class(void) : srclist(), srcs() {}
	virtual ~src_cache_class(void) {}
	/* here, debug_srcname is the source file name as supplied by the
	 * debugging information, and srcfile_name (if supplied) is the
	 * file name actually used when locating the file for access;
	 * if srcfile_name is not supplied (i.e., it is null), then
	 * the debug_srcname string is used to locate the file
	 *
	 * in any case, when retrieving source file data for a file,
	 * the debug_srcname string is always used for locating the
	 * proper data in the map below - that is, srcfile_name is
	 * used only for physically locating the file, and debug_srcname
	 * is always used as a key when looking for source file data */ 
	struct src_cache_class::srcfile_data * get_srcfile(const char * debug_srcname, const char * srcfile_name = 0);

private:
	struct srcfile_data * read_srcfile(const char * srcname)
		throw (const char * /* srcname - the file could not be opened */);
	class std::map<std::string, struct srcfile_data *> srcs;

};

class src_cache_class_checked : public src_cache_class
{
public:
	src_cache_class_checked(void) : src_cache_class() {}
	~src_cache_class_checked(void) {}
	struct src_cache_class::srcfile_data * get_srcfile(const char * debug_srcname, const char * srcfile_name = 0)
#if 1
		{ return src_cache_class::get_srcfile(debug_srcname, srcfile_name); }
#else
	;
#endif
};

#endif /* __SRC_CACHE_HXX__ */

