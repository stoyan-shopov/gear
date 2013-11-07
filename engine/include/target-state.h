/*!
 *	\file	target-state.h
 *	\brief	target core state enumeration
 *	\author	shopov
 *
 *	Revision summary:
 *
 *	$Log: $
 */
#ifndef __TARGET_STATE_H__
#define __TARGET_STATE_H__

/*
 *
 * exported types follow
 *
 */ 

/*! target core connection status (target state)
 *
 * only two major states have been identified for the connection
 * to a target core - connection to the target core is either
 * established (alive) or not (dead) (because either there
 * is no connection to a target core controller responsible for
 * providing access to a target core, or because there is
 * no connection to the target core itself); the alive connection
 * state is further specialized by discriminating between a
 * target core running or halted; so, in total, this enumeration
 * contains three enumerator values corresponding to the two
 * major target connection states identified - dead or alive, with
 * two enumerators corresponding to (and further specializing) the
 * alive state, and one enumerator corresponding to the dead state -
 * amounting to a total of three enumerator values
 *
 * the transitions in either direction between these two major states 
 * constitute significant, system-wide events in the gear engine
 * because specific important actions may have to be performed
 * on these transitions (for example, if connection to the target
 * core is alive and suddenly drops down, any allocated target
 * memory cache buffers should be invalidated, and maybe deallocated,
 * various status data should be updated, etc.); physically, the transitions
 * between these states are detected in the target-comm.c module
 * and the target state change notification routines
 * are invoked from module target_comm.c; also see comments
 * about the core_register_target_state_change_callback
 * data field in structure core_control in file
 * core-access.h
 *
 * under most common circumstances, the need for taking certain
 * actions on the transitions between target states
 * is addressed within module exec.c by invoking registered callback
 * functions which are interested in the occurrences of the
 * transitions
 *
 * the callbacks spoken of here are *not* the target state change
 * callbacks normally mentioned elsewhere in the gear engine documentation;
 * do *not* confuse callbacks here invoked from the exec.c
 * module with the target state change callbacks invoked
 * from module target-comm.c; the very important distinction
 * about these two sets of callbacks is that the target state
 * change callbacks invoked from module target-comm.c are
 * intended to seize control of a target core at a basic, fundamental
 * level, giving opportunity to control a target core closely, thus,
 * perhaps, overriding any other gear engine modules striving
 * to control and monitor a target core; an example of this is
 * the target-img-load.c module, which needs exclusive target
 * control to achieve its purpose; normally, the target state
 * change callback invoked by module target-comm.c is the executive
 * module - exec.c; from there, in this most common mode of
 * operation, are the callbacks described herein invoked -
 * their purpose is the proper maintainance of various gear engine
 * data structures, such as cleanup and initialization
 * logic - these callbacks are, therefore, much more passive in
 * operation and purpose compared to the target state change
 * callbacks, their one true purpose being *maintenance*;
 * read below;
 * it is a fatal error if i(sgs)
 * am not being clear enough here, and someone does not truly
 * understand whatever described within these paragraphs; as
 * my(sgs) english is not very good, i(sgs) will gladly
 * cooperate with anyone truly devoted to the improvement
 * of the gear engine documentation
 *
 * this is only one solution to the problem, and maybe
 * not the best one, but as the gear engine does not have some
 * form of event dispatching mechanism built in (as, for example,
 * commonly is the case for gear frontends), this seems like
 * the simplest and most straightforward solution; beware, however,
 * that this mechanism is not foolproof and pitfalls are possible,
 * as illustrated in the following scenario:
 *	as the gear engine state (and - as a part of it - the
 *	call stack of the engine) can be arbitrary at the time
 *	of detecting, for example, the "target alive - target dead"
 *	transition, and as the invocation of the transition callback
 *	functions is performed immediately upon the detection
 *	of such a transition - one may end up cleaning up something
 *	that has been only halfway initialized by code residing
 *	upwards in the call stack, which, if not handled properly,
 *	may have quite unpleasant results; currently, there is
 *	not a preferred solution for such problems, but as
 *	simple guidelines, the following may be of use: data
 *	"locks" and/or flags may be added on data that is to be
 *	manipulated by the transitions callback, or simply not
 *	supply a callback and handle errors in the code upwards
 *	the call stack (in the scenario above); of course, other
 *	solutions are possible and, depending on the circumstances,
 *	may work better
 * 
 * also see the comments at the start of file exec.c
 *
 *	\note	if, except explicitly stated otherwise, anywhere
 *		the phrase 'target state' is used, it means
 *		the same as the phrase 'target core connection state'
 *		used herein
 */
enum TARGET_CORE_STATE_ENUM
{
	/*! invalid state, used for catching errors */
	TARGET_CORE_STATE_INVALID = 0,
	/*! connection to the target not established (dead) */
	TARGET_CORE_STATE_DEAD,
	/*! connection to the target established (alive), and the target is halted */
	TARGET_CORE_STATE_HALTED,
	/*! connection to the target established (alive), and the target is running */
	TARGET_CORE_STATE_RUNNING,
};

#endif /* __TARGET_STATE_H__ */

