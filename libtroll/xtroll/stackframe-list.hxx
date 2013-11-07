#ifndef __STACKFRAME_LIST_HXX__
#define __STACKFRAME_LIST_HXX__

#include <vector>
#include <string>
#include <stdint.h>
#include <iostream>

class stackframe_list : public dbg_record
{
	public:
		class stackframe
		{
			private:
				struct
				{
					int	pc_value_valid	: 1;
					int	comp_unit_valid	: 1;
					int	subprogram_valid	: 1;
					int	srcfile_valid	: 1;
					int	srcline_nr_valid	: 1;
				}
				flags;

				uint32_t	pc_value;
				std::string	comp_unit_name;
				std::string	subprogram_name;
				std::string	srcfile_name;
				int		srcline_nr;


			public:
				stackframe(void) { std::cout << "stackframe constructed\n"; 
					flags.pc_value_valid = flags.comp_unit_valid = flags.subprogram_valid = 
						flags.srcfile_valid = flags.srcline_nr_valid = 0; srcline_nr = -1; }
				~stackframe(void) { std::cout << "stackframe destroyed\n"; }
				void set_pc_addr(uint32_t pc_value) { this->pc_value = pc_value; flags.pc_value_valid = 1; }
				void set_comp_unit_name(const std::string & comp_unit_name) { this->comp_unit_name = comp_unit_name; flags.comp_unit_valid = 1; }
				void set_subprogram_name(const std::string & subprogram_name) { this->subprogram_name = subprogram_name; flags.subprogram_valid = 1; }
				void set_srcfile_name(const std::string & srcfile_name) { this->srcfile_name = srcfile_name; flags.srcfile_valid = 1; }
				void set_srcline_nr(int srcline_nr) { this->srcline_nr = srcline_nr; flags.srcline_nr_valid = 1; }

				bool is_pc_value_valid(void) { return flags.pc_value_valid ? true : false; }
				bool is_comp_unit_valid(void) { return flags.comp_unit_valid ? true : false; }
				bool is_subprogram_valid(void) { return flags.subprogram_valid ? true : false; }
				bool is_srcfile_valid(void) { return flags.srcfile_valid ? true : false; }
				bool is_srcline_nr_valid(void) { return flags.srcline_nr_valid ? true : false; }

				uint32_t pc_val(void) { return pc_value; }
				const std::string & get_comp_unit_name(void) { return comp_unit_name; }
				const std::string & get_subprogram_name(void) { return subprogram_name; }
				const std::string & get_srcfile_name(void) { return srcfile_name; }
				int get_srcline_nr(void) { return srcline_nr; }
		};
	private:
		std::vector<class stackframe>	frame_list;
	public:
	stackframe_list(void) : dbg_record(dbg_record::DBG_RECORD_STACKFRAME_LIST) { std::cout << "stackframes constructed\n"; }
	~stackframe_list(void) { std::cout << "stackframes destroyed\n"; }
	std::vector<class stackframe> & stackframes(void) { return frame_list; }

};

#endif /* __STACKFRAME_LIST_HXX__ */

