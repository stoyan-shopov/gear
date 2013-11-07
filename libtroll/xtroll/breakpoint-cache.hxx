#ifndef __BREAKPOINT_CACHE_HXX__
#define __BREAKPOINT_CACHE_HXX__

#include <vector>
#include <map>
#include <stdint.h>
#include <iostream>

class breakpoint_cache : public dbg_record
{
	private:
		std::vector<uint32_t>	addr_bkpts;
		std::vector<uint32_t>	all_addr_bkpts;

		std::map<std::string, std::vector<uint32_t> > src_bkpts;
	public:
	breakpoint_cache(void) : dbg_record(dbg_record::DBG_RECORD_BREAKPOINT_CACHE) { std::cout << "breakpoints constructed\n"; }
	~breakpoint_cache(void) { std::cout << "breakpoints destroyed\n"; }
	//std::vector<uint32_t> & breakpoints(void) { return addr_bkpts; }
	std::vector<uint32_t> & breakpoints(void) { return addr_bkpts; }
	std::vector<uint32_t> & breakpoints(class srcinfo_data * s)
	{
		all_addr_bkpts = addr_bkpts;
		std::map<std::string, std::vector<uint32_t> >::iterator i;
		int j, k;

		for (i = src_bkpts.begin(); i != src_bkpts.end(); i ++)
		{
		//std::vector<struct srcline_addr_data> get_target_addresses_for_srcline(const std::string & srcfile_name, int srcline_nr)
			for (j = 0; j < (*i).second.size(); j ++)
			{
				std::vector<struct srcinfo_data::srcline_addr_data> x = s->get_target_addresses_for_srcline((*i).first, i->second.operator[](j));
				/*
		struct srcline_addr_data
		{
			struct srclist_type_struct * srcfile;
			std::vector<struct srcline_addr_pair_struct *> srcdata;
		};
		*/
				for (k = 0; k < x.size(); k ++)
				{
					int i;
					for (i = 0; i < x[k].srcdata.size(); i ++)
					{
						all_addr_bkpts.push_back(x[k].srcdata[i]->addr);
					}
				}
			}
		}

		/*! \todo	!!! IMPORTANT - SORT THIS, MAKE ELEMENTS UNIQUE !!! */
		return all_addr_bkpts;
	}

	std::vector<uint32_t> & breakpoints(const std::string & srcfile_name) { return src_bkpts[srcfile_name]; }


	void insert_breakpoint_at_addr(uint32_t addr) { int i; for (i = 0; i < addr_bkpts.size(); i ++) if (addr_bkpts[i] == addr) break; if (i == addr_bkpts.size()) addr_bkpts.push_back(addr); }
	void remove_breakpoint_at_addr(uint32_t addr) { std::vector<uint32_t>::iterator i; for (i = addr_bkpts.begin(); i != addr_bkpts.end(); i ++) if (i.operator *() == addr) { addr_bkpts.erase(i); break; } }

	void insert_breakpoint_at_src(const std::string & srcfile_name, unsigned int line_nr)
	//{ int i; std::vector<uint32_t> v = src_bkpts[srcfile_name]; for (i = 0; i < v.size(); i ++) if(v[i] == line_nr) break; if (i == v.size()) v.push_back(line_nr); }
	{ int i; std::vector<uint32_t> * v = & src_bkpts[srcfile_name]; for (i = 0; i < v->size(); i ++) if((*v)[i] == line_nr) break; if (i == v->size()) v->push_back(line_nr); }
	void remove_breakpoint_at_src(const std::string & srcfile_name, unsigned int line_nr)
	//{ std::vector<uint32_t>::iterator i; std::vector<uint32_t> v = src_bkpts[srcfile_name]; for (i = v.begin(); i != v.end(); i ++) if (i.operator*() == line_nr) break; if (i != v.end()) v.erase(i); }
	{ std::vector<uint32_t>::iterator i; std::vector<uint32_t> * v = & src_bkpts[srcfile_name]; for (i = v->begin(); i != v->end(); i ++) if (i.operator*() == line_nr) break; if (i != v->end()) v->erase(i); }

};

#endif /* __BREAKPOINT_CACHE_HXX__ */
