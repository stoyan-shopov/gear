#include "dbg-record.hxx"


int main(void)
{
register_list xregs;
dbg_record * xxx = new dbg_record(dbg_record::DBG_RECORD_INVALID);
register_list * yyy = new register_list();
std::vector<register_list::reg> rlist;
std::vector<register_list::reg>::iterator i;
class stackframe_list frames;
class stackframe_list::stackframe frame;
int x;

	xregs.regs().push_back(register_list::reg("r0", 0));
	xregs.regs().push_back(register_list::reg("r1", 1));
	rlist = xregs.regs();
	for (i = rlist.begin(); i != rlist.end(); i ++)
		std::cout << i->name() << "\t=\t" << i->value() << "\n";
	for (x = 0; x < rlist.size(); x ++)
		std::cout << rlist[x].name() << "\t=\t" << rlist[x].value() << "\n";
	std::cout << xxx->asRegisterList();
	std::cout << yyy->asRegisterList();

	frame.set_pc_addr(123);
	frame.set_subprogram_name("test-sub");
	frame.set_comp_unit_name("test-cu");
	frame.set_srcfile_name("test-srcfile");
	frame.set_srcline_nr(223344);

	frames.stackframes().push_back(frame);

	return 0;
}

