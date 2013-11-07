#ifndef __SRCINFO_DATA_H__
#define __SRCINFO_DATA_H__

extern "C"
{
#include <srcinfo.h>
#include <string.h>
#include <malloc.h>
};

#include <vector>

class srcinfo_data : public dbg_record
{
	private:
		void srcinfo_destroy(struct srcinfo_type_struct * node)
		{
			struct srclist_type_struct * p, * p1;
			int i;

			if (!node)
				return;
			for (i = 0; i < node->nr_comp_units; i++)
			{
				free((void *) node->comp_units[i].name);
				free((void *) node->comp_units[i].compdir);
			}
			if (node->comp_units)
				free(node->comp_units);

			p = node->srclist;
			while (p)
			{
				if (p->srcname)
					free(p->srcname);
				if (p->bkpt_bmap)
					free(p->bkpt_bmap);
				if (p->srcaddr_pairs)
					free(p->srcaddr_pairs);
				if (p->subprogram_arr)
				{
					for (i = 0; i < p->subprogram_arr_len; i ++)
						free(p->subprogram_arr[i].name);
					free(p->subprogram_arr);
				}
				p1 = p;
				p = p->next;
				free(p1);
			}

			free(node);
		}

		struct srcinfo_type_struct * srcinfo_clone(struct srcinfo_type_struct * src)
		{
			struct srclist_type_struct * p;
			struct srclist_type_struct * psrc;
			struct srcinfo_type_struct * newnode;
			int i;


			if (!src)
				return 0;
			if (!(newnode = (struct srcinfo_type_struct *) calloc(1, sizeof * newnode)))
				return 0;

			if ((i = src->nr_comp_units))
			{
				newnode->comp_units = (struct cu_info_struct *) calloc(i, sizeof * newnode->comp_units);
				newnode->nr_comp_units = i;
				for (i = 0; i < src->nr_comp_units; i++)
				{
					newnode->comp_units[i].name = strdup(src->comp_units[i].name);
					newnode->comp_units[i].compdir = strdup(src->comp_units[i].compdir);
				}
			}

			/* clone srclist */
			newnode->srclist = p = (struct srclist_type_struct *) calloc(1, sizeof * p);
			psrc = src->srclist;
			do
			{
				p->srcname = strdup(psrc->srcname);
				p->bmap_size = psrc->bmap_size;
				p->low_pc = psrc->low_pc;
				p->hi_pc = psrc->hi_pc;
				p->bkpt_bmap = (unsigned int *) malloc(p->bmap_size * sizeof * p->bkpt_bmap);
				memcpy(p->bkpt_bmap, psrc->bkpt_bmap, p->bmap_size * sizeof * p->bkpt_bmap);
				p->srcaddr_len = psrc->srcaddr_len;
				if (psrc->srcaddr_pairs)
				{
					p->srcaddr_pairs = (struct srcline_addr_pair_struct *) malloc(p->srcaddr_len * sizeof * p->srcaddr_pairs);
					memcpy(p->srcaddr_pairs, psrc->srcaddr_pairs,
							p->srcaddr_len * sizeof * p->srcaddr_pairs);
				}
				else
					p->srcaddr_pairs = 0;
				/* copy subprogram list */
				if (psrc->subprogram_arr_len)
				{
					p->subprogram_arr = (struct subprogram_type_struct *) calloc(psrc->subprogram_arr_len, sizeof * p->subprogram_arr);
					p->subprogram_arr_len = psrc->subprogram_arr_len;
					for (i = 0; i < p->subprogram_arr_len; i ++)
						p->subprogram_arr[i].name = strdup(psrc->subprogram_arr[i].name),
							p->subprogram_arr[i].srcline_nr = psrc->subprogram_arr[i].srcline_nr;
				}

				psrc = psrc->next;
				if (psrc)
					p = p->next = (struct srclist_type_struct *) calloc(1, sizeof * p);

			}
			while (psrc);

			return newnode;
		}

		struct srcinfo_type_struct * srcdata;

	public:
		struct srcline_addr_data
		{
			struct srclist_type_struct * srcfile;
			std::vector<struct srcline_addr_pair_struct *> srcdata;
		};
		std::vector<struct srcline_addr_data> get_target_addresses_for_srcline(const std::string & srcfile_name, int srcline_nr)
		{
			std::vector<struct srcline_addr_data> v;
			int i;
			struct srclist_type_struct * srcfile;
			for (srcfile = srcdata->srclist; srcfile; srcfile = srcfile->next)
				if (srcfile_name == srcfile->srcname)
					break;
			if (srcfile)
			{
				std::vector<struct srcline_addr_pair_struct *> v1;

				for (i = 0; i < srcfile->srcaddr_len; i ++)
					/*! \todo	use binary search here, as the 'srcaddr_pairs'
					 *		array is necessarily sorted by address */
					if (srcfile->srcaddr_pairs[i].srcline_nr == srcline_nr)
						v1.push_back(srcfile->srcaddr_pairs + i);
				if (!v1.empty())
				{
					struct srcline_addr_data s;
					s.srcfile = srcfile;
					s.srcdata = v1;
					v.push_back(s);
				}
			}
			return v;
		}

		std::vector<struct srcline_addr_data> get_srclines_for_target_addr(uint32_t target_addr)
		{
			std::vector<struct srcline_addr_data> v;
			int i;
			struct srclist_type_struct * srcfile;
			for (srcfile = srcdata->srclist; srcfile; srcfile = srcfile->next)
			{
				std::vector<struct srcline_addr_pair_struct *> v1;

				if (!(srcfile->low_pc <= target_addr && target_addr < srcfile->hi_pc))
					continue;
				for (i = 0; i < srcfile->srcaddr_len; i ++)
					/*! \todo	use binary search here, as the 'srcaddr_pairs'
					 *		array is necessarily sorted by address */
					if (srcfile->srcaddr_pairs[i].addr == target_addr)
						v1.push_back(srcfile->srcaddr_pairs + i);
				if (!v1.empty())
				{
					struct srcline_addr_data s;
					s.srcfile = srcfile;
					s.srcdata = v1;
					v.push_back(s);
				}
			}
			return v;
		}


		srcinfo_data(struct srcinfo_type_struct * srcdata) : dbg_record(DBG_RECORD_SRCINFO_DATA) { this->srcdata = srcdata; }
		srcinfo_data(const srcinfo_data & src) : dbg_record(DBG_RECORD_SRCINFO_DATA) { srcdata = srcinfo_clone(src.srcdata); }
		srcinfo_data & operator=(srcinfo_data const & srcinfo) { if (this != & srcinfo) { srcinfo_destroy(srcdata); srcdata = srcinfo_clone(srcinfo.srcdata); } return * this; }
		struct srclist_type_struct * srclist(void) { return srcdata->srclist; }
		struct srcinfo_type_struct * srcinfo(void) { return srcdata; }
		~srcinfo_data(void) { srcinfo_destroy(srcdata); }
};

#endif /* __SRCINFO_DATA_H__ */

