
typedef unsigned int ARM_CORE_WORD;

#include "target-state.h"


/* forward declaration */
struct core_access_struct	* cc;

struct target_ctl_context
{
	struct core_access_struct	* cc;
	/*! target specific data
	 *
	 * this is available for use by the low level target
	 * core controller driver, usually initialized by a private
	 * data type in target_get_core_access() and/or core_open() */
	struct target_ctl_private_data		* core_data;
};

/*! this describes debug events in the target being debugged */
struct dbg_event
{
	/*! the debug event code */
	enum DBG_EVENT_ENUM
	{
		DBG_EVENT_INVALID = 0,
		DBG_EVENT_THREAD_HALTED,
		DBG_EVENT_THREAD_CREATED,
	}
	dbg_evt_code;
	/*! which member of the union applies depends on the value of the dbg_evt_code field above */
	union
	{
		/*! applicable for DBG_EVENT_THREAD_HALTED and DBG_EVENT_THREAD_CREATED */
		int	thread_id;
	};
};

/*! core control data structure */
struct core_access_struct
{
	/*! establishes a connection to a target core */
	enum GEAR_ENGINE_ERR_ENUM (*core_open)(struct target_ctl_context * ctx, const char * executable_fname, int argc, const char ** argv, const char ** errmsg_hint);
	/*! shuts down access to a target core */
	enum GEAR_ENGINE_ERR_ENUM (*core_close)(struct target_ctl_context * ctx);
	/*! reads memory in a target system */
	enum GEAR_ENGINE_ERR_ENUM (*core_mem_read)(struct target_ctl_context * ctx, void *dest, ARM_CORE_WORD source, unsigned *nbytes);
	/*! writes memory in a target system */
	enum GEAR_ENGINE_ERR_ENUM (*core_mem_write)(struct target_ctl_context * ctx, ARM_CORE_WORD dest, const void *source, unsigned *nbytes);
	/*! reads register(s) from a target core */
	enum GEAR_ENGINE_ERR_ENUM (*core_reg_read)(struct target_ctl_context * ctx, unsigned mode, unsigned thread_id, unsigned long mask, ARM_CORE_WORD buffer[]);
	/*! writes register(s) in a target core */
	enum GEAR_ENGINE_ERR_ENUM (*core_reg_write)(struct target_ctl_context * ctx, unsigned mode, unsigned thread_id, unsigned long mask, ARM_CORE_WORD buffer[]);
	/*! reads register(s) from a coprocessor */
	enum GEAR_ENGINE_ERR_ENUM (*core_cop_read)(struct target_ctl_context * ctx, unsigned CPnum, unsigned long mask, ARM_CORE_WORD buffer[]);
	/*! writes register(s) to a coprocessor */
	enum GEAR_ENGINE_ERR_ENUM (*core_cop_write)(struct target_ctl_context * ctx, unsigned CPnum, unsigned long mask, ARM_CORE_WORD buffer[]);
	/*! sets a breakpoint */
	enum GEAR_ENGINE_ERR_ENUM (*core_set_break)(struct target_ctl_context * ctx, ARM_CORE_WORD address);
	/*! clears a breakpoint
	 * \todo clean this up */
	enum GEAR_ENGINE_ERR_ENUM (*core_clear_break)(struct target_ctl_context * ctx, ARM_CORE_WORD address);
	/*! runs a core until a breakpoint has been hit or until other halting condition comes in effect
	 *
	 * \todo	document what other conditions abort the 'run' mode */
	enum GEAR_ENGINE_ERR_ENUM (*core_run)(struct target_ctl_context * ctx, unsigned thread_id/*ARM_CORE_WORD * halt_addr*/);
	/*! halts a target core */
	enum GEAR_ENGINE_ERR_ENUM (*core_halt)(struct target_ctl_context * ctx, unsigned thread_id);
	/*! instruction-wise single step a target core */
	enum GEAR_ENGINE_ERR_ENUM (*core_insn_step)(struct target_ctl_context * ctx, unsigned thread_id/*ARM_CORE_WORD * halt_addr*/);
	/*! generic input/output control interface */
	enum GEAR_ENGINE_ERR_ENUM (*core_ioctl)(struct target_ctl_context * ctx, int request_len, ARM_CORE_WORD * request, int * response_len, ARM_CORE_WORD ** response);
	/*! obtains target core status information */
	//enum GEAR_ENGINE_ERR_ENUM (*core_get_dbg_event)(struct target_ctl_context * ctx, struct dbg_event * evt);
	enum GEAR_ENGINE_ERR_ENUM (*core_get_status)(struct target_ctl_context * ctx, enum TARGET_CORE_STATE_ENUM * status);
};

#define BIT0	(1 << 0)
#define BIT1	(1 << 1)
#define BIT2	(1 << 2)
#define BIT3	(1 << 3)
#define BIT4	(1 << 4)
#define BIT5	(1 << 5)
#define BIT6	(1 << 6)
#define BIT7	(1 << 7)
#define BIT8	(1 << 8)
#define BIT9	(1 << 9)

#define BIT10	(1 << 10)
#define BIT11	(1 << 11)
#define BIT12	(1 << 12)
#define BIT13	(1 << 13)
#define BIT14	(1 << 14)
#define BIT15	(1 << 15)
#define BIT16	(1 << 16)
#define BIT17	(1 << 17)
#define BIT18	(1 << 18)
#define BIT19	(1 << 19)

#define BIT20	(1 << 20)
#define BIT21	(1 << 21)
#define BIT22	(1 << 22)
#define BIT23	(1 << 23)
#define BIT24	(1 << 24)
#define BIT25	(1 << 25)
#define BIT26	(1 << 26)
#define BIT27	(1 << 27)
#define BIT28	(1 << 28)
#define BIT29	(1 << 29)

#define BIT30	(1 << 30)
#define BIT31	(1 << 31)

