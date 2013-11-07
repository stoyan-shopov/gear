#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <map>
#include <list>
#include <string>

using namespace std;

#include "panic.h"
#include "src-cache.hxx"
#include "c-lex-tokens.h"

#include "c-lex.h"

struct src_cache_class::srcfile_data * src_cache_class::read_srcfile(const char * srcname)
	throw (const char * /* srcname - the file could not be opened */)
{
struct srcfile_data * p;
int srcfile;
off_t fsize;
int i, j, k, nr_newlines;
int cur_line;
char * s;
yyscan_t scanner;
char * tokenbuf;
YY_BUFFER_STATE bufstate;
/* this holds the number of additional space characters
 * that should be added to the original sorce code file
 * when converting all whitespace characters to space
 * characters */
int nr_wspace_chars;
int tab_pos;
char * tbuf;

	if ((srcfile = open(srcname, O_RDONLY | O_BINARY)) == -1)
	{
		throw srcname;
	}
	p = new struct srcfile_data;

	/* retrieve file size */
	fsize = 0;
	if ((fsize = lseek(srcfile, fsize, SEEK_END)) == -1)
		panic("lseek");
	{
	off_t off;
	off = 0;
	if (lseek(srcfile, off, SEEK_SET) == -1)
		panic("lseek");
	}

	/*! \bug	*/
	//p->name = strdup(srcname);
	if (!(tbuf = (char *) malloc(fsize)))
		panic("out of core");

	i = read(srcfile, tbuf, fsize);
	if (i != fsize)
	{
		panic("fread");
	}
	close(srcfile);

	/* ok, count the number of newlines and additional space
	 * characters that must be added when converting all of
	 * the whitespace characters to space characters */
	/*! \todo	merge this loop with the line building
	 *		loop below, this here is a quick addition, no
	 *		time to do it right now... */
	for (nr_wspace_chars = tab_pos = i = nr_newlines = 0, s = tbuf;
			i < fsize;
			i++, s++)
	{
		if (*s == '\n')
			nr_newlines++, tab_pos = 0;
		else if (*s == '\t')
		{
			nr_wspace_chars += NR_SPACES_IN_A_TAB
				- tab_pos;
			tab_pos = 0;
			nr_wspace_chars--;
		}
		else
			if (++tab_pos == NR_SPACES_IN_A_TAB)
				tab_pos = 0;
	}

	p->textbuf_size = fsize + 1 + nr_wspace_chars;
	/* allocate core for the text of the file */
	if (!(p->textbuf = (char*) malloc(p->textbuf_size)))
		panic("out of core");
	if (!(p->tokenbuf = (char*) malloc(p->textbuf_size)))
		panic("out of core");
	/* copy the program source code text */
	for (k = i = tab_pos = 0, s = tbuf; k < fsize; k++, i++, s++)
	{
		switch (*s)
		{
			case '\t': for (j = 0; j < NR_SPACES_IN_A_TAB - tab_pos; j++)
					   p->textbuf[i++] = ' ';
				   i--; tab_pos = 0; break;
			case '\r':	/* skip any carriage returns found */
				   p->textbuf[i] = ' ';
				   break;
#if 0
			case '\r':	/* skip any carriage returns found,
					 * inferring this is probably an
					 * msdos-formatted text file */
					/*! \todo	currently, this is
					 *		not very accurate... */
					s ++;
					k ++;
					/*
					if (k == fsize)
						k --;
						*/
					/* fall out */
#endif
			case '\n': tab_pos = 0;
				   p->textbuf[i] = '\n';
				   break;
			default:
				   p->textbuf[i] = *s;
				if (++tab_pos == NR_SPACES_IN_A_TAB)
					tab_pos = 0;
		}
	}
	free(tbuf);

	/* null terminate the last line */
	p->textbuf[p->textbuf_size - 1] = 0;

	/* tokenize the text - only c is supported right now */
	tokenbuf = p->tokenbuf;
	c_lex_init_extra(&tokenbuf, &scanner);
	bufstate = c__scan_string(p->textbuf, scanner);
        if (c_lex(scanner))
                panic("");

//memset(p->tokenbuf, C_TOKEN_INVALID - C_TOKEN_BASE, p->textbuf_size);

	c__delete_buffer(bufstate, scanner);
	c_lex_destroy(scanner);


	/* ok, get core for the lines data */
	/* there are one more lines than newlines in the file */
	nr_newlines++;
	p->nr_lines = nr_newlines;
	if (!(p->lines = (char **)malloc(nr_newlines * sizeof(char *))))
		panic("out of core");
	if (!(p->lex_token_lines = (char **)malloc(nr_newlines * sizeof(char *))))
		panic("out of core");

	/* build line pointers */
	p->lines[0] = p->textbuf;
	p->lex_token_lines[0] = p->tokenbuf;
	for (i = 0, cur_line = 1, s = p->textbuf; i < p->textbuf_size; i++, s++)
		if (*s == '\n')
		{
			*s = 0;
			p->lines[cur_line] = s + 1;
			p->lex_token_lines[cur_line] = p->tokenbuf + (s - p->textbuf) + 1;

			cur_line++;
		}

	printf("srcfile_read_file: ok, %i characters read in %i lines\n", p->textbuf_size - 1, p->nr_lines);
	return p;
}

struct src_cache_class::srcfile_data * src_cache_class::get_srcfile(const char * debug_srcname, const char * srcfile_name)
{
string s(debug_srcname);

	if (srcs[s] == 0)
	{
		srclist.push_front(srcs[s] = read_srcfile(srcfile_name ? srcfile_name : debug_srcname));
	}
	return srcs[s];
}

