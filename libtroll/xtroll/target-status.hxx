#ifndef __TARGET_STATUS_HXX__
#define __TARGET_STATUS_HXX__

#include <string>
#include <stdint.h>

class target_status : public dbg_record
{
	public:
		enum TARGET_STATE_ENUM
		{
			TARGET_STATE_INVALID	= 0,
			TARGET_STATE_HALTED,
			TARGET_STATE_RUNNING,
			TARGET_STATE_DEAD,
		};
	private:
		struct
		{
			int	halt_addr_valid	: 1;
			int	comp_unit_valid	: 1;
			int	subprogram_valid	: 1;
			int	srcfile_valid	: 1;
			int	srcline_nr_valid	: 1;
		}
		flags;
		enum TARGET_STATE_ENUM state;

		uint32_t	halt_addr;
		std::string	comp_unit_name;
		std::string	subprogram_name;
		std::string	srcfile_name;
		int		srcline_nr;


	public:
		target_status(enum TARGET_STATE_ENUM state = TARGET_STATE_INVALID) : dbg_record(DBG_RECORD_TARGET_STATUS) {
			flags.halt_addr_valid = flags.comp_unit_valid = flags.subprogram_valid = 
				flags.srcfile_valid = flags.srcline_nr_valid = 0; srcline_nr = -1;
				this->state = state; }
			void set_state(enum TARGET_STATE_ENUM state) { this->state = state; }
			void set_halt_addr(uint32_t halt_addr) { this->halt_addr = halt_addr; flags.halt_addr_valid = 1; }
			void set_comp_unit_name(const std::string & comp_unit_name) { this->comp_unit_name = comp_unit_name; flags.comp_unit_valid = 1; }
			void set_subprogram_name(const std::string & subprogram_name) { this->subprogram_name = subprogram_name; flags.subprogram_valid = 1; }
			void set_srcfile_name(const std::string & srcfile_name) { this->srcfile_name = srcfile_name; flags.srcfile_valid = 1; }
			void set_srcline_nr(int srcline_nr) { this->srcline_nr = srcline_nr; flags.srcline_nr_valid = 1; }

			bool is_halt_addr_valid(void) { return flags.halt_addr_valid ? true : false; }
			bool is_comp_unit_valid(void) { return flags.comp_unit_valid ? true : false; }
			bool is_subprogram_valid(void) { return flags.subprogram_valid ? true : false; }
			bool is_srcfile_valid(void) { return flags.srcfile_valid ? true : false; }
			bool is_srcline_nr_valid(void) { return flags.srcline_nr_valid ? true : false; }

			uint32_t get_halt_addr(void) { return halt_addr; }
			const std::string & get_comp_unit_name(void) { return comp_unit_name; }
			const std::string & get_subprogram_name(void) { return subprogram_name; }
			const std::string & get_srcfile_name(void) { return srcfile_name; }
			int get_srcline_nr(void) { return srcline_nr; }
			enum TARGET_STATE_ENUM get_state(void) { return state; }
};

#endif /* __TARGET_STATUS_HXX__ */

