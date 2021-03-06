#include <QMessageBox>
#include <QTextCharFormat>
#include <QTextCursor>
#include <QFileDialog>
#include <QTemporaryFile>
#include <QProgressDialog>

#include <windows.h>

#include <queue>
#include <list>
#include <map>
#include <string>
#include <string.h>
#include <malloc.h>

#include <stdio.h>
#include <panic.h>

using namespace std;

#include "exec.hxx"

#define gwprintf(...)

#include <QDebug>


void exec_display_class::win_set_color(XCURSES_COLOR_ENUM color)
{
#if 0
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
#endif
}

struct src_cache_class::srcfile_data * exec_display_class::getSrc(const QString &srcfile_name)
{
QString alt_srcname;
struct src_cache_class::srcfile_data * src;
bool already_tried;
int btn_pressed;

static bool look_for_files = true;

        src = 0;
        already_tried = false;

	if (!srcfile_name.isNull())
		while (1)
		{
			try
			{
				if (alt_srcname.isEmpty())
					src = srcs->get_srcfile((srcfile_name + '\0').toAscii().data());
				else
					src = srcs->get_srcfile((srcfile_name + '\0').toAscii().data(),
							(alt_srcname + '\0').toAscii().data());
				break;
			}
			catch (const char * s)
                        {
                                qDebug() << "looking for source code files...";
                                if (!already_tried)
                                {
                                        /* try to find the file in one of the directories
                                         * already specified by the user */
                                        int i;
                                        already_tried = true;
                                        for (i = 0; i < srcdir_list.size(); i++)
                                        {
                                                QFileInfo fi(QDir(srcdir_list.at(i)), QFileInfo(srcfile_name).fileName());
                                                if (fi.exists())
                                                {
                                                        alt_srcname = fi.canonicalFilePath();
                                                        break;
                                                }
                                                QFileInfo fi1(QDir(srcdir_list.at(i)), srcfile_name);
                                                qDebug() << "trying file: " << QString(srcdir_list.at(i)) + srcfile_name;
                                                if (fi1.exists())
                                                {
                                                        alt_srcname = fi1.canonicalFilePath();
                                                        break;
                                                }
                                        }
                                        if (i != srcdir_list.size())
                                        {
                                                /* a file with the name requested was
                                                 * found in one of the directories already
                                                 * specified by the user - attempt to load that file */
                                                 continue;
                                        }
                                }
				if (!look_for_files)
					return 0;
                                btn_pressed = QMessageBox::information(0, "file not found", srcfile_name + " not found", "load file", "do not prompt anymore", "skip", 1);
                                if (btn_pressed == 2)
                                        return 0;
				if (btn_pressed == 1 || btn_pressed == -1)
				{
					look_for_files = false;
					return 0;
				}
				src = 0;
				alt_srcname = QFileDialog::getOpenFileName(0,
					"select source code file",
                                        last_srcdir,
					QFileInfo(srcfile_name).fileName() + ";;*");
				if (alt_srcname.isEmpty())
					/* the user cancelled the file open dialog */
					break;
				else
                                {
                                        QFileInfo fi(alt_srcname);

                                        last_srcdir = fi.canonicalPath();
                                        srcdir_list.append(last_srcdir);
                                        srcdir_list.removeDuplicates();
                                }
			}
		}

	return src;
}


QSharedPointer<XTextDocument> exec_display_class::docFromSrcname(const QString &srcname, bool do_disassemble, bool do_fold_disassembly)
{
	int i;
	enum XCURSES_COLOR_ENUM cur_col;
	enum C_LEX_TOKEN_ENUM cur_token;
	struct srclist_type_struct * srcdata;
	struct custom_srcdata * udata;
	XTextDocument	* newdoc = new XTextDocument;
	QTextCursor	cursor(newdoc);
	struct src_cache_class::srcfile_data * src;
	QTemporaryFile	tmpfile;
	bool must_close_font_tag;

	QSharedPointer<XTextDocument> shared_doc(newdoc);

	src = getSrc(srcname);

	newdoc->setUndoRedoEnabled(false);

	qDebug() << __func__ << "start";
	qDebug() << "document is" << srcname;

	if (!src)
	{
		cursor.insertText("<<< no source code available for file " + srcname + ">>>\n");
		return shared_doc;
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

	QProgressDialog progress;

	progress.setMinimum(0);
	progress.setMaximum(src->nr_lines);
	progress.setLabelText("generating html...");
	progress.setValue(0);
	progress.show();

	tmpfile.open();
        if (0) tmpfile.write("<style type=\"text/css\"> p {white-space: pre; color: \"white\"; \
		margin-top: 0px; margin-bottom: 0px; font-family: 'Fixedsys,fixedsys'; } </style>");

        tmpfile.write("<style type=\"text/css\"> p {white-space: pre; color: \"white\"; \
                margin-top: 0px; margin-bottom: 0px; } </style>");

	//newdoc->setDefaultStyleSheet("<p style=\" margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; -qt-block-indent:0; text-indent:0px; -qt-user-state:401;\">");


	do_fold_disassembly = false;

	must_close_font_tag = false;
	while (i < src->nr_lines)
	{

		char * s, * t;
		s = src->lines[i];
		t = src->lex_token_lines[i];

		/* this causes the current token to be always updated on the first
		 * iteration of the loop below */
		cur_token = (C_LEX_TOKEN_ENUM) 0;

		cur_col = XCOL_WHITE;

		qDebug() << i << (int) *s;


		if (!*s)
			/* blank line */
			/*! \todo	fix this... */
			//tmpfile.write("&nbsp;");
			tmpfile.write("<p> </p>");
		else
		{
			tmpfile.write("<p>");
			while (*s)
			{
				if (*t + C_TOKEN_BASE != cur_token)
					/* choose new color */
					switch (cur_token = (enum C_LEX_TOKEN_ENUM)(*t + C_TOKEN_BASE))
					{
						default:
						case C_TOKEN_INVALID:
					if (!must_close_font_tag)
						/* already at default font color */
						break;
					if (cur_col == XCOL_WHITE)
						break;
					cur_col = XCOL_WHITE;
					win_set_color(XCOL_BLACK);
					tmpfile.write("</font><font color=\"white\">");
					cur_col = XCOL_BLACK;
					must_close_font_tag = true;
					break;
						case C_TOKEN_TYPE_RELATED_KEYWORD:
					if (cur_col == XCOL_GREEN)
						break;
					cur_col = XCOL_GREEN;
					win_set_color(XCOL_GREEN);
					if (must_close_font_tag) tmpfile.write("</font>");
					tmpfile.write("<font color=\"green\">");
					must_close_font_tag = true;
					break;
						case C_TOKEN_KEYWORD:
					if (cur_col == XCOL_YELLOW)
						break;
					cur_col = XCOL_YELLOW;
					win_set_color(XCOL_YELLOW);
					if (must_close_font_tag) tmpfile.write("</font>");
					tmpfile.write("<font color=\"yellow\">");
					must_close_font_tag = true;
					break;
						case C_TOKEN_IDENTIFIER:
					if (!must_close_font_tag)
						/* already at default font color */
						break;
					if (cur_col == XCOL_WHITE)
						break;
					cur_col = XCOL_WHITE;
					win_set_color(XCOL_BLACK);
					tmpfile.write("</font><font color=\"white\">");
					must_close_font_tag = true;
					break;
						case C_TOKEN_CONSTANT:
					if (cur_col == XCOL_MAGENTA)
						break;
					cur_col = XCOL_MAGENTA;
					win_set_color(XCOL_MAGENTA);
					if (must_close_font_tag) tmpfile.write("</font>");
					tmpfile.write("<font color=\"magenta\">");
					must_close_font_tag = true;
					break;
						case C_TOKEN_STRING_LITERAL:
					if (cur_col == XCOL_MAGENTA)
						break;
					cur_col = XCOL_MAGENTA;
					win_set_color(XCOL_MAGENTA);
					if (must_close_font_tag) tmpfile.write("</font>");
					tmpfile.write("<font color=\"magenta\">");
					must_close_font_tag = true;
					break;
						case C_TOKEN_PUNCTUATOR:
					if (!must_close_font_tag)
						/* already at default font color */
						break;
					if (cur_col == XCOL_WHITE)
						break;
					cur_col = XCOL_WHITE;
					win_set_color(XCOL_BLACK);
					tmpfile.write("</font><font color=\"white\">");
					must_close_font_tag = true;
					break;
						case C_TOKEN_CHAR_CONSTANT:
					if (cur_col == XCOL_MAGENTA)
						break;
					cur_col = XCOL_MAGENTA;
					win_set_color(XCOL_MAGENTA);
					if (must_close_font_tag) tmpfile.write("</font>");
					tmpfile.write("<font color=\"magenta\">");
					must_close_font_tag = true;
					break;
						case C_TOKEN_COMMENT:
					if (cur_col == XCOL_CYAN)
						break;
					cur_col = XCOL_CYAN;
					win_set_color(XCOL_CYAN);
					if (must_close_font_tag) tmpfile.write("</font>");
					tmpfile.write("<font color=\"cyan\">");
					must_close_font_tag = true;
					break;
						case C_TOKEN_PREPROCESSOR:
					if (cur_col == XCOL_BLUE)
						break;
					cur_col = XCOL_BLUE;
					win_set_color(XCOL_BLUE);
					if (must_close_font_tag) tmpfile.write("</font>");
					tmpfile.write("<font color=\"blue\">");
					must_close_font_tag = true;
					break;
				}
				switch (*s)
				{
				case '&': tmpfile.write("&amp;"); break;
				case '<': tmpfile.write("&lt;"); break;
				case '>': tmpfile.write("&gt;"); break;
				default: tmpfile.write(s, 1); break;
				 }
				s++;
				t++;
			}
			if (must_close_font_tag) tmpfile.write("</font>");
			tmpfile.write("<p>");
			must_close_font_tag = false;
		}

		i++;
		progress.setValue(i);
		cur_col = XCOL_WHITE;
	}

	tmpfile.seek(0);
	progress.setValue(0);
	progress.setLabelText("loading html...");
        newdoc->setHtml(tmpfile.readAll());
        //newdoc->setPlainText(tmpfile.readAll());
	cursor.setPosition(0);
	i = 0;

	progress.setValue(0);
	if (!srcdata || !do_disassemble)
		progress.setLabelText("generating line information");
	else
		progress.setLabelText("generating disassembly listing");

	QTextCharFormat	disasm_format;
        disasm_format.setForeground(Qt::gray);

	while (!cursor.atEnd())
	{
		union SrcUserUnion	ustate;
		QTextDocument ddoc;
		QTextCursor ccursor(&ddoc);
		enum
		{
			SRCLINE_STATE_NORMAL,
			SRCLINE_STATE_BREAKPOINTABLE,
			SRCLINE_STATE_BREAKPOINTED,
		}
		srcline_state;

		ustate.x = 0;
		ustate.state.is_src = 1;
		ustate.state.srcline_nr = i + 1;

		/* see if the current line is breakpointable */
		srcline_state = SRCLINE_STATE_NORMAL;



		cursor.block().setUserState(ustate.x);



		if (srcdata)
		{
			if (udata->srcline_info[i] & SRCINFO_LINE_BREAKPOINTABLE)
			{
				if (do_fold_disassembly)
					ustate.state.is_folded = 1;
				ustate.state.has_disassembly = 1;
				cursor.block().setUserState(ustate.x);

				if (do_disassemble)
				{
					int t;

					ddoc.clear();
					ccursor.setPosition(0);

					t = udata->disasm_addrs[i];
					while (t)
					{
                                        const unsigned char * mem;
					unsigned int memlen, addr;

						addr = srcdata->srcaddr_pairs[t].addr;


#if 1
                                                        cursor.movePosition(QTextCursor::EndOfBlock);
							for (int xxx = 0; xxx < 1; xxx ++)
							{
                                                        cursor.insertText("\n");
ccursor.insertText("\n");
                                                        cursor.setCharFormat(disasm_format);
#if 1
                                                        cursor.insertText("\t\t-- compilation unit: ");
                                                        cursor.insertText(srcinfo->srcinfo()->comp_units[srcdata->srcaddr_pairs[t].cuname_idx].name);
                                                        cursor.insertText("; subprogram: ");
                                                        cursor.insertText(srcdata->subprogram_arr[srcdata->srcaddr_pairs[t].subarr_idx].name);


ccursor.insertText("\t\t-- compilation unit: ");
ccursor.insertText(srcinfo->srcinfo()->comp_units[srcdata->srcaddr_pairs[t].cuname_idx].name);
ccursor.insertText("; subprogram: ");
ccursor.insertText(srcdata->subprogram_arr[srcdata->srcaddr_pairs[t].subarr_idx].name);

#endif

                                                        ustate.x = 0;
                                                        cursor.block().setUserState(ustate.x);

                                                        if (do_fold_disassembly)
                                                                cursor.block().setVisible(false);
							}
#endif
                                                /* see if disassembling is at all possible */
                                                if (memcache.isNull() || !(mem = (const unsigned char *) memcache->getMemDataPtr(addr, &memlen)))
						{
							/* disassembly unavailable - only print address */
							cursor.movePosition(QTextCursor::EndOfBlock);
							cursor.insertText("\n");
							cursor.setCharFormat(disasm_format);
							cursor.insertText("0x");
							cursor.insertText(QString().number(addr, 16));
							cursor.insertText("\t<<< disassembly unavailable >>>");

							ustate.x = 0;
							ustate.state.has_disassembly = 1;
							cursor.block().setUserState(ustate.x);
							cursor.block().setRevision(addr);

							if (do_fold_disassembly)
								cursor.block().setVisible(false);
						}
						else if (0)
						{
							/* try to disassemble using the udis86 library */
						int insn_len;
						uint64_t pc;
							pc = addr;
							ud_set_input_buffer(&udis86_obj, (uint8_t *) mem, memlen);
							do
							{
								ud_set_pc(&udis86_obj, pc);
								insn_len = ud_disassemble(&udis86_obj);
								if (insn_len == 0)
									/* end of input */
									break;
								cursor.movePosition(QTextCursor::EndOfBlock);
								cursor.insertText("\n");
								cursor.setCharFormat(disasm_format);
								cursor.insertText("\t0x");
								cursor.insertText(QString().number(pc, 16));
								cursor.insertText("\t");
								cursor.insertText(ud_insn_hex(&udis86_obj));
								cursor.insertText("\t");
								cursor.insertText(ud_insn_asm(&udis86_obj));

								ustate.x = 0;
								ustate.state.has_disassembly = 1;
								cursor.block().setUserState(ustate.x);
								cursor.block().setRevision(pc);

								if (do_fold_disassembly)
									cursor.block().setVisible(false);

								pc += insn_len;
							}
							while (pc < srcdata->srcaddr_pairs[t + 1].addr);

						}
						else
						{
                                                /* try to disassemble using the pipelined openocd arm disassembler */
						int insn_len;
						uint64_t pc;
							pc = addr;
							if (0)
							{
								cursor.insertText("\n");
ccursor.insertText("\n");
								ustate.x = 0;
								ustate.state.has_disassembly = 1;
								cursor.block().setUserState(ustate.x);
							}
							do
							{
                                                                QString insn_text;
								uint32_t opc;

								opc = 0;
								if (memlen >= 4)
									opc |= (unsigned) mem[3] << 24, opc |= (unsigned) mem[2] << 16;
								if (memlen >= 2)
                                                                        opc |= (unsigned) mem[1] << 8, opc |= (unsigned) mem[0] << 0;
                                                                if (openocd_disasm_process->state() == QProcess::Running)
                                                                {
									if (1)
									{
                                                                        openocd_disasm_process->write(QString("%1 %2\n").arg(pc).arg(opc).toAscii());
									openocd_disasm_process->waitForBytesWritten();
									while (!openocd_disasm_process->canReadLine())
										openocd_disasm_process->waitForReadyRead(-1);
									//while (openocd_disasm_process->canReadLine())
										//qDebug() << "incoming line:" << openocd_disasm_process->readLine();
                                                                        insn_len = QString(openocd_disasm_process->readLine()).toUInt();
									while (!openocd_disasm_process->canReadLine())
										openocd_disasm_process->waitForReadyRead(-1);
                                                                        insn_text = QString(openocd_disasm_process->readLine());
									insn_text.replace(QRegExp("[\n\r]"), QString());
									}
									else
									{
										insn_text = "nop";
										insn_len = 2;
									}
                                                                        //insn_len = 2;
                                                                        //insn_text = "xyz aaa";
                                                                }
                                                                else
                                                                {
                                                                        qDebug() << "openocd disassembler not running";
                                                                        insn_len = 0;
                                                                }

                                                                memlen -= insn_len;
                                                                mem += insn_len;
								if (insn_len == 0)
									/* end of input */
									break;
								cursor.movePosition(QTextCursor::EndOfBlock);
								cursor.setCharFormat(disasm_format);

								cursor.insertText("\n");
								cursor.insertText(insn_text);
ccursor.insertText("\n");
ccursor.insertText(insn_text);

								ustate.x = 0;
								ustate.state.has_disassembly = 1;
								cursor.block().setUserState(ustate.x);
								cursor.block().setRevision(pc);

								if (do_fold_disassembly)
									cursor.block().setVisible(false);

								pc += insn_len;
							}
							while (pc < srcdata->srcaddr_pairs[t + 1].addr);

						}


						/* skip to next disassembly run corresponding
						 * to this line number (if any) */
						t = srcdata->srcaddr_pairs[t].iextend;
						if (t)
						{
							/* more disassembly to follow, add some visual separation */
							cursor.movePosition(QTextCursor::EndOfBlock);
							cursor.insertText("\n");
							cursor.setCharFormat(disasm_format);
							cursor.insertText("<<< more disassembly, address range discontiguous >>>");



ccursor.insertText("\n");
ccursor.insertText("<<< more disassembly, address range discontiguous >>>");


							ustate.x = 0;
							cursor.block().setUserState(ustate.x);

							if (do_fold_disassembly)
								cursor.block().setVisible(false);
						}
					}
				}
			}
			if (!ddoc.isEmpty())
			{
				qDebug() << "---------------------------------------------------";
				qDebug() << ddoc.toPlainText();
				qDebug() << "---------------------------------------------------";
				if (0) cursor.insertText(ddoc.toPlainText());
			}
		}

		i ++;
		progress.setValue(i);
		if (!cursor.movePosition(QTextCursor::NextBlock))
			break;
	}

	qDebug() << __func__ << "end";
	return shared_doc;
}
#if 0
QSharedPointer<QTextDocument> exec_display_class::docFromSrcname(const QString & srcname)
{
	int i;
	enum XCURSES_COLOR_ENUM cur_col;
	enum C_LEX_TOKEN_ENUM cur_token;
	struct srclist_type_struct * srcdata;
	struct custom_srcdata * udata;
	QTextDocument	* newdoc = new QTextDocument;
	QTextCursor	cursor(newdoc);
	struct src_cache_class::srcfile_data * src;
	//QTemporaryFile	tmpfile;
	bool must_close_font_tag;

        QSharedPointer<QTextDocument> shared_doc(newdoc);

	src = getSrc(srcname);

	newdoc->setUndoRedoEnabled(false);

	qDebug() << __func__ << "start";
	qDebug() << "document is" << srcname;

	if (!src)
	{
		cursor.insertText("<<< no source code available for file " + srcname + ">>>\n");
                return shared_doc;
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

	QProgressDialog progress;

	progress.setMinimum(0);
	progress.setMaximum(src->nr_lines);
	progress.setLabelText("generating html...");
	progress.setValue(0);
	progress.show();

	//tmpfile.open();
	//tmpfile.write("<style type=\"text/css\"> p {white-space: pre; color: \"white\"; } </style>");
	cursor.insertHtml("<style type=\"text/css\"> p {white-space: pre; color: \"white\"; } </style>");

	must_close_font_tag = false;
	while (i < src->nr_lines)
	{

		char * s, * t;
		s = src->lines[i];
		t = src->lex_token_lines[i];

		/* this causes the current token to be always updated on the first
		 * iteration of the loop below */
		cur_token = (C_LEX_TOKEN_ENUM) 0;

		cur_col = XCOL_WHITE;

		if (!*s)
			/* blank line */
			/*! \todo	fix this... */
			//tmpfile.write("&nbsp;");
			//tmpfile.write("<p> </p>");
			cursor.insertHtml("<p> </p>");
		else
		{
			//tmpfile.write("<p>");
			cursor.insertHtml("<p>");
			while (*s)
			{
				if (*t + C_TOKEN_BASE != cur_token)
					/* choose new color */
					switch (cur_token = (enum C_LEX_TOKEN_ENUM)(*t + C_TOKEN_BASE))
					{
						default:
						case C_TOKEN_INVALID:
					if (!must_close_font_tag)
						/* already at default font color */
						break;
					if (cur_col == XCOL_WHITE)
						break;
					cur_col = XCOL_WHITE;
					win_set_color(XCOL_BLACK);
					//tmpfile.write("</font><font color=\"white\">");
					cursor.insertHtml("</font><font color=\"white\">");
					cur_col = XCOL_BLACK;
					must_close_font_tag = true;
					break;
						case C_TOKEN_TYPE_RELATED_KEYWORD:
					if (cur_col == XCOL_GREEN)
						break;
					cur_col = XCOL_GREEN;
					win_set_color(XCOL_GREEN);
					//if (must_close_font_tag) tmpfile.write("</font>");
					//tmpfile.write("<font color=\"green\">");
					if (must_close_font_tag) cursor.insertHtml("</font>");
					cursor.insertHtml("<font color=\"green\">");
					must_close_font_tag = true;
					break;
						case C_TOKEN_KEYWORD:
					if (cur_col == XCOL_YELLOW)
						break;
					cur_col = XCOL_YELLOW;
					win_set_color(XCOL_YELLOW);
					//if (must_close_font_tag) tmpfile.write("</font>");
					if (must_close_font_tag) cursor.insertHtml("</font>");
					//tmpfile.write("<font color=\"yellow\">");
					cursor.insertHtml("<font color=\"yellow\">");
					must_close_font_tag = true;
					break;
						case C_TOKEN_IDENTIFIER:
					if (!must_close_font_tag)
						/* already at default font color */
						break;
					if (cur_col == XCOL_WHITE)
						break;
					cur_col = XCOL_WHITE;
					win_set_color(XCOL_BLACK);
					//tmpfile.write("</font><font color=\"white\">");
					cursor.insertHtml("<font color=\"white\">");
					must_close_font_tag = true;
					break;
						case C_TOKEN_CONSTANT:
					if (cur_col == XCOL_MAGENTA)
						break;
					cur_col = XCOL_MAGENTA;
					win_set_color(XCOL_MAGENTA);
					//if (must_close_font_tag) tmpfile.write("</font>");
					if (must_close_font_tag) cursor.insertHtml("</font>");
					//tmpfile.write("<font color=\"magenta\">");
					cursor.insertHtml("<font color=\"magenta\">");
					must_close_font_tag = true;
					break;
						case C_TOKEN_STRING_LITERAL:
					if (cur_col == XCOL_MAGENTA)
						break;
					cur_col = XCOL_MAGENTA;
					win_set_color(XCOL_MAGENTA);
					//if (must_close_font_tag) tmpfile.write("</font>");
					if (must_close_font_tag) cursor.insertHtml("</font>");
					//tmpfile.write("<font color=\"magenta\">");
					cursor.insertHtml("<font color=\"magenta\">");
					must_close_font_tag = true;
					break;
						case C_TOKEN_PUNCTUATOR:
					if (!must_close_font_tag)
						/* already at default font color */
						break;
					if (cur_col == XCOL_WHITE)
						break;
					cur_col = XCOL_WHITE;
					win_set_color(XCOL_BLACK);
					//tmpfile.write("</font><font color=\"white\">");
					cursor.insertHtml("<font color=\"white\">");
					must_close_font_tag = true;
					break;
						case C_TOKEN_CHAR_CONSTANT:
					if (cur_col == XCOL_MAGENTA)
						break;
					cur_col = XCOL_MAGENTA;
					win_set_color(XCOL_MAGENTA);
					//if (must_close_font_tag) tmpfile.write("</font>");
					if (must_close_font_tag) cursor.insertHtml("</font>");
					//tmpfile.write("<font color=\"magenta\">");
					cursor.insertHtml("<font color=\"magenta\">");
					must_close_font_tag = true;
					break;
						case C_TOKEN_COMMENT:
					if (cur_col == XCOL_CYAN)
						break;
					cur_col = XCOL_CYAN;
					win_set_color(XCOL_CYAN);
					//if (must_close_font_tag) tmpfile.write("</font>");
					if (must_close_font_tag) cursor.insertHtml("</font>");
					//tmpfile.write("<font color=\"cyan\">");
					cursor.insertHtml("<font color=\"cyan\">");
					must_close_font_tag = true;
					break;
						case C_TOKEN_PREPROCESSOR:
					if (cur_col == XCOL_BLUE)
						break;
					cur_col = XCOL_BLUE;
					win_set_color(XCOL_BLUE);
					//if (must_close_font_tag) tmpfile.write("</font>");
					if (must_close_font_tag) cursor.insertHtml("</font>");
					//tmpfile.write("<font color=\"blue\">");
					cursor.insertHtml("<font color=\"blue\">");
					must_close_font_tag = true;
					break;
				}
				switch (*s)
				{
#if 0
				case '&': tmpfile.write("&amp;"); break;
				case '<': tmpfile.write("&lt;"); break;
				case '>': tmpfile.write("&gt;"); break;
				default: tmpfile.write(s, 1); break;
#endif
				case '&': cursor.insertHtml("&amp;"); break;
				case '<': cursor.insertHtml("&lt;"); break;
				case '>': cursor.insertHtml("&gt;"); break;
				default: cursor.insertHtml(QChar(*s)); break;
				 }
				s++;
				t++;
			}
			//if (must_close_font_tag) tmpfile.write("</font>");
			//tmpfile.write("<p>");
			if (must_close_font_tag) cursor.insertHtml("</font>");
			cursor.insertHtml("<p>");
			must_close_font_tag = false;
		}

		i++;
		progress.setValue(i);
		cur_col = XCOL_WHITE;
	}

#if 0
	tmpfile.seek(0);
	progress.setValue(0);
	progress.setLabelText("loading html...");
	newdoc->setHtml(tmpfile.readAll());
	//newdoc->setPlainText(tmpfile.readAll());
#endif
	cursor.setPosition(0);
	i = 0;

	progress.setValue(0);
	progress.setLabelText("generating line information");

	while (!cursor.atEnd())
	{
		union SrcUserUnion	ustate;
		enum
		{
			SRCLINE_STATE_NORMAL,
			SRCLINE_STATE_BREAKPOINTABLE,
			SRCLINE_STATE_BREAKPOINTED,
		}
		srcline_state;

		ustate.x = 0;
		ustate.state.is_src = 1;

		/* see if the current line is breakpointable */
		srcline_state = SRCLINE_STATE_NORMAL;
		if (srcdata)
		{
			if (udata->srcline_info[i] & SRCINFO_LINE_BREAKPOINTABLE)
				ustate.state.has_disassembly = 1;
		}

		ustate.state.srcline_nr = i + 1;
		cursor.block().setUserState(ustate.x);

		i ++;
		progress.setValue(i);
		if (!cursor.movePosition(QTextCursor::NextBlock))
			break;
	}

	qDebug() << __func__ << "end";
        return shared_doc;
}
#endif


#if 0
QTextDocument * exec_display_class::docFromSrcnameOld(const QString & srcname)
{
	int i;
	enum XCURSES_COLOR_ENUM cur_col;
	enum C_LEX_TOKEN_ENUM cur_token;
	struct srclist_type_struct * srcdata;
	struct custom_srcdata * udata;
	QString item_text;
	QTextDocument	* newdoc = new QTextDocument;
	QTextCursor	cursor(newdoc);
	struct src_cache_class::srcfile_data * src;
	SrcUserData	* textblock_data;


	qDebug() << "document is" << srcname;

	src = getSrc(srcname);

	if (!src)
	{
		cursor.insertText("<<< no source code available for file " + srcname + ">>>\n");
		return newdoc;
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

		if (0) cursor.insertText(item_text = QString().number(i + 1) + "\t");
#if 0
		{
			/* see if the target is halted at the line just to be printed, and
				 * if so - highlight the line by drawing a frame arount it */
			QTextCharFormat f(currentCharFormat());
			if (exec_status->srcline == i + 1)
			{
				////draw_frame_around_srcline(win_line_nr);
				f.setFontWeight(QFont::Bold);
			}
			else
				f.setFontWeight(QFont::Normal);
			setCurrentCharFormat(f);
		}
#endif
#if SYNTAX_HILITING && 0
		////gwprintf("%i\t", i + 1);
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
			//item_text.append(".");
			break;
				default:
			////gwprintf(" ");
			insertPlainText(" ");
			//item_text.append(" ");
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
#else
		cursor.insertText(s);
		item_text.append(s);
#endif /* SYNTAX_HILITING */


		textblock_data = new SrcUserData();

		if (1)
		{
			/* see if the current line is breakpointable */
			srcline_state = SRCLINE_STATE_NORMAL;
			if (srcdata)
			{
				if (udata->srcline_info[i] & SRCINFO_LINE_BREAKPOINTABLE)
					srcline_state = SRCLINE_STATE_BREAKPOINTABLE;
			}
			if (srcline_state & SRCINFO_LINE_BREAKPOINTABLE)
			{
				textblock_data->setHasDisassembly(true);
			}
			else
				textblock_data->setHasDisassembly(false);

		}

		textblock_data->setLineNumber(i + 1);
		cursor.block().setUserData(textblock_data);

		cursor.insertText("\n");

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

			disnode = 0;
			if (!udata->disasm.empty())
			{
				j = udata->disasm_addrs[i];
				disnode = (struct disasm_text_node*) udata->srcdata->srcaddr_pairs
					  [j].pextend;

				qDebug() << i;
#if 0
				if (!udata->nr_printed_disasm_lines)
				{
					qDebug() << "<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< nr_printed_disasm_lines is null!";
				}
				else
#endif
				{
					nr_disasm_lines = udata->nr_printed_disasm_lines[i];
				}
			}

			if (!disnode)
			{
				/* no disassembly text present for this line - only
				 * print the starting address(es) of machine code
				 * generated for this line */
				j = udata->disasm_addrs[i];
dump_only_dis_addrs:
				while (j)
				{
				unsigned long x = udata->srcdata->srcaddr_pairs[j].addr;
					class SrcUserData * sd = new SrcUserData;
					cursor.insertText(QString().number(x, 16) + "\t<<< dump disassembly here >>>");
					sd->setHasDisassembly(true);
					sd->setDisAddr(x);
					sd->setIsSrc(false);
					cursor.block().setUserData(sd);
					cursor.insertText("\n...\n");
					j = udata->srcdata->srcaddr_pairs[j].iextend;
				}
			}
			else
				/* disassembly text present for this line - dump it */
				while (nr_disasm_lines--)
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
					class SrcUserData * sd = new SrcUserData;
					item_text = QString().number(disnode->num.addr) + "\t" + disnode->text;
					item_text = QString("0x") + QString().number(disnode->num.addr, 16) + "\t" + disnode->text;
					cursor.insertText(item_text);
					sd->setHasDisassembly(true);
					sd->setDisAddr(disnode->num.addr);
					sd->setIsSrc(false);
					cursor.block().setUserData(sd);
					cursor.insertText("\n");

					disnode = disnode->next;
					if (!disnode)
					{
						j = udata->srcdata->srcaddr_pairs[j].iextend;
						disnode = (struct disasm_text_node*) udata->srcdata->srcaddr_pairs
							  [j].pextend;
						if (!nr_disasm_lines--)
						{
							if (j)
							{
								cursor.insertText("...\n");
								goto dump_only_dis_addrs;
							}
							break;
						}
						cursor.insertText("...\n");

					}
					if (disnode && disnode->dis_type != disasm_text_node::DIS_TYPE_DISASSEMBLY)
					{
						/* most probably dumping multiple machine
						 * code address ranges corresponding to a
						 * single source code line number - distinguish
						 * such ranges by printing a header line */
						if (!nr_disasm_lines--)
							break;
						cursor.insertText(QString("...\n"));
					}
				}
				if (!disnode)
					goto dump_only_dis_addrs;
			}
		}
		i++;
	}

	return newdoc;
}
#endif


bool exec_display_class::parse_record_available(class srcinfo_data *sinfo)
{
bool res;

	res = false;

switch (0)
{
default:
        {
                int i;
                /*! \todo	update source code information data structures here - don't simply ignore the newly arrived data */
                if (srcinfo->srcinfo())
			/* source file list already available - do nothing */
                        break;

                srcdir_list = QStringList();
                qDebug() << "nr_comp_units: " << sinfo->srcinfo()->nr_comp_units;
                for (i = 0; i < sinfo->srcinfo()->nr_comp_units; i++)
                {
                        srcdir_list.append(QString(sinfo->srcinfo()->comp_units[i].compdir));
                }
                srcdir_list.removeDuplicates();

                build_custom_srcdata(sinfo);

                res = true;
        }
#if 0
	else if (0 && phead->type == TROLL_RECORD_DISASSEMBLY_LIST)
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
#if 0
			if (udata->srcdata->low_pc > dis->start_addr
			    || udata->srcdata->hi_pc <= dis->start_addr
			    )
				continue;
#endif
#if 0
			if (udata->disasm)
				/* this can happen when refreshing
						 * a disassembly */
				troll_deallocate_record(udata->disasm);
#endif

			udata->disasm.push_back(0);
			udata->disasm.push_back((disasm_type_struct *) troll_clone(dis));
			res = build_disasm_data(udata, (udata->disasm.back())->disasm_list) || res;
		}
	}
#endif
}

	return res;
}


volatile int xxx;

void exec_display_class::build_custom_srcdata(class srcinfo_data * new_srclist)
{
	struct srclist_type_struct * s;
	int i;
	/* process the list of source code files */
	* srcinfo = * new_srclist;
	s = srcinfo->srcinfo()->srclist;
	bool is_warning_printed;
	int file_cnt, file_nr;
	struct srclist_type_struct * slist;

	/* count the number of files */
	for (slist = s, file_cnt = 0; slist; slist = slist->next, file_cnt ++)
		;

	QProgressDialog	progress;
	progress.setWindowTitle("loading source files");

	file_nr = 0;
	progress.setMinimum(0);
	progress.setMaximum(file_cnt);
	progress.show();

	while (s)
	{

		if (strstr(s->srcname, "main.c"))
			xxx;

		progress.setLabelText(s->srcname);
		progress.setValue(file_nr);
		struct src_cache_class::srcfile_data * src;

		src = getSrc(s->srcname);
		if (src && !src->udata)
		{
			struct custom_srcdata * p = new struct custom_srcdata;
			p->srcdata = s;

			p->srcline_info = new char[src->nr_lines];
			memset(p->srcline_info, 0, src->nr_lines);
			src->udata = p;
			/* if disassembly is appropriate - process it */
			if (s->srcaddr_len)
			{
				p->disasm_addrs = new unsigned int[src->nr_lines];

				memset(p->disasm_addrs, 0, src->nr_lines * sizeof(unsigned int));
                                is_warning_printed = false;
				for (i = /* zeroth element reserved/unused */ 0; i < s->srcaddr_len; i++)
				//for (i = 0; i < s->srcaddr_len; i++)
				{
					unsigned int j;
					j = s->srcaddr_pairs[i].srcline_nr;
					if (j > src->nr_lines)
                                        {
                                                /* a source code file is out of sync, or probably
                                                 * another one... this really needs better handling... */
                                                /*! \todo	handle this better, this below
                                                 *		is very evil... */
                                                if (!is_warning_printed)
                                                        QMessageBox::critical(0,
                                                                "bad source code file",
                                                                QString("the source code file for the source code name\n\n") + s->srcname
                                                                        + "\n\nseems to have been picked up wrong "
                                                                        "as it disagrees with the debug information "
                                                                        "for this file reported by the gear; consider "
                                                                        "supplying an alternate physical file "
                                                                        "to map for the debug information filename \"" + s->srcname
                                                                        + "\", as this file appears to be picked up wrong "
                                                                        "in the frontend, and the line number information "
                                                                        "for this file is wrong and useless and can only "
                                                                        "bring confusion...", "shoot!");
                                                is_warning_printed = true;
                                                continue;
                                        }
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
		file_nr ++;
	}
}

#if 0
bool exec_display_class::build_disasm_data(struct custom_srcdata * udata, struct disasm_text_node * dislist)
{
bool is_update_needed;
struct disasm_text_node * disnode;
/* counts the number of disassembly lines to print
 * for a given source code line */
int dis_run_len;
int i, t;
struct src_cache_class::srcfile_data * src;
ARM_CORE_WORD last_addr, first_addr;

	is_update_needed = false;

	src = getSrc(udata->srcdata->srcname);
	if (!src)
		panic("");

	if (!udata->nr_printed_disasm_lines)
	{
		/*! \note	the nr_printed_disasm_lines
		 *		can have already been allocated, when
		 *		so, we are just adding new disassembly data
		 *		and not touching the already built data
		 *		structures... */
		udata->nr_printed_disasm_lines = new int[src->nr_lines];
		memset(udata->nr_printed_disasm_lines, 0, src->nr_lines * sizeof * udata->nr_printed_disasm_lines);
		for (i = 0; i < udata->srcdata->srcaddr_len; i++)
			udata->srcdata->srcaddr_pairs[i].pextend = 0;
	}

	disnode = dislist;
	if (udata->srcdata->srcaddr_len < 2 + /* zeroth element reserved/unused */ 1)
		panic("");
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
		/* if a previous disassembly is present - attempt
		 * to undo the number of disassembly lines corresponding
		 * to it... */
		if (udata->srcdata->srcaddr_pairs[i].pextend)
		{
			/* scoped shadow */
			struct disasm_text_node * disnode = (struct disasm_text_node *) udata->srcdata->srcaddr_pairs[i].pextend;
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
			udata->nr_printed_disasm_lines[t - 1] -= dis_run_len;
			if (udata->nr_printed_disasm_lines[t - 1] < 0)
				panic("");
			if (udata->nr_printed_disasm_lines[t - 1])
				udata->nr_printed_disasm_lines[t - 1] --;
		}

		is_update_needed = true;

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
	}

	return is_update_needed;
}
#endif

exec_display_class::exec_display_class(void) :
        srcs(0),
        srcinfo(new srcinfo_data(0))
{
	/* initialize the udis86 disassembler library */
	ud_init(&udis86_obj);
	ud_set_syntax(&udis86_obj, UD_SYN_ATT);
        ud_set_mode(&udis86_obj, 32);

        openocd_disasm_process = new QProcess();
        openocd_disasm_process->start("C:/shopov/src/openocd-arm-disassembler/openocd-arm-dis.exe");
        if (!openocd_disasm_process->waitForStarted())
        {
                /* this dumps core... eventloop related? */
                if (1) QMessageBox::critical(0, "the openocd arm disassembler failed to start",
                                      "the openocd arm disassembler failed to start\n"
                                      "you may be missing the openocd arm disassembler executable");
        }
}


int XTextDocument::getBlockNumberForSrclineNumber(int srcline_nr)
{
	if (srcline_block_nrs == 0)
	{
		/* first time call */
		/* build the list of block numbers */
		/*! \todo	this is currently being done
		 *		in two passes - first the
		 *		source code line numbers
		 *		are counted, and then
		 *		the list is being built;
		 *		the first pass should probably
		 *		be eliminated... */
		QTextBlock bl(firstBlock());
		union SrcUserUnion ustate;
		int i;

		i = 0;
		while (bl.isValid())
		{
			ustate.x = bl.userState();
			if (ustate.state.is_src)
				i ++;
			bl = bl.next();
		}
		if (i != 0)
		{
			bl = firstBlock();
			nr_srclines = i;
			srcline_block_nrs = new int[i];
			i = 0;
			while (bl.isValid())
			{
				ustate.x = bl.userState();
				if (ustate.state.is_src)
				{
					srcline_block_nrs[i++] = bl.blockNumber();
				}
				bl = bl.next();
			}
		}
		else
			/* no source code lines found - avoid
			 * further document scanning and just
			 * return an error (-1) below */
			nr_srclines = -1;
	}

	if (nr_srclines <= 0)
		return -1;
	srcline_nr --;
	if (srcline_nr < 0 || srcline_nr >= nr_srclines)
		return -1;
	return srcline_block_nrs[srcline_nr];
}

