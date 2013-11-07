#ifndef __REGISTER_LIST_HXX__
#define __REGISTER_LIST_HXX__

#include <vector>
#include <string>
#include <stdint.h>
#include <iostream>

class register_list : public dbg_record
{
	public:
		class reg
		{
			private:
				std::string regname;
				uint32_t	val;
			public:
				reg(std::string regname, uint32_t val) { std::cout << "reg constructed\n"; this->regname = regname; this->val = val; }
				~reg(void) { std::cout << "reg destroyed\n"; }
				const std::string & name(void) { return regname; }
				void set_name(const std::string & regname) { this->regname = regname; }
				uint32_t value(void) { return val; }
		};
	private:
		std::vector<class reg>	reglist;
	public:
	register_list(void) : dbg_record(dbg_record::DBG_RECORD_REGISTER_LIST) { std::cout << "regs constructed\n"; }
	~register_list(void) { std::cout << "regs destroyed\n"; }
	std::vector<class reg> & regs(void) { return reglist; }

};

#endif /* __REGISTER_LIST_HXX__ */
