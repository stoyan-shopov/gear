#ifndef __STACKFRAME_ACTIVATED_HXX__
#define __STACKFRAME_ACTIVATED_HXX__

class stackframe_activated : public dbg_record
{
	private:
		int stackframe_active;
	public:
	stackframe_activated(int stackframe_active) : dbg_record(dbg_record::DBG_RECORD_STACKFRAME_ACTIVATED) { this->stackframe_active = stackframe_active; }	
	int active_stackframe(void) { return stackframe_active; }

};

#endif /* __STACKFRAME_ACTIVATED_HXX__ */

