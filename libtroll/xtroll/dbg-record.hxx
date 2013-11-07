#ifndef __DBG_RECORD_HXX__
#define __DBG_RECORD_HXX__

class register_list;
class stackframe_list;
class stackframe_activated;
class target_status;
class srcinfo_data;
class breakpoint_cache;

class dbg_record
{
	public:
	enum ENUM_DBG_RECORD
	{
		DBG_RECORD_INVALID	= 0,
		DBG_RECORD_REGISTER_LIST,
		DBG_RECORD_STACKFRAME_LIST,
		DBG_RECORD_STACKFRAME_ACTIVATED,
		DBG_RECORD_TARGET_STATUS,
		DBG_RECORD_SRCINFO_DATA,
		DBG_RECORD_BREAKPOINT_CACHE,
	};
	dbg_record(enum ENUM_DBG_RECORD type) { this->type = type; }
	class register_list * asRegisterList(void) const { return (type == DBG_RECORD_REGISTER_LIST) ? (register_list *) this : 0; }
	class stackframe_list * asStackframeList(void) const { return (type == DBG_RECORD_STACKFRAME_LIST) ? (stackframe_list *) this : 0; }
	class stackframe_activated * asStackframeActivated(void) const { return (type == DBG_RECORD_STACKFRAME_ACTIVATED) ? (stackframe_activated *) this : 0; }
	class target_status * asTargetStatus(void) const { return (type == DBG_RECORD_TARGET_STATUS) ? (target_status *) this : 0; }
	class srcinfo_data * asSrcinfoData(void) const { return (type == DBG_RECORD_SRCINFO_DATA) ? (srcinfo_data *) this : 0; }
	class breakpoint_cache * asBreakpointCache(void) const { return (type == DBG_RECORD_BREAKPOINT_CACHE) ? (breakpoint_cache *) this : 0; }
	private:
	enum ENUM_DBG_RECORD type;
};

#include "register-list.hxx"
#include "stackframe-list.hxx"
#include "stackframe-activated.hxx"
#include "target-status.hxx"
#include "srcinfo-data.hxx"
#include "breakpoint-cache.hxx"

#endif /* __DBG_RECORD_HXX__ */
