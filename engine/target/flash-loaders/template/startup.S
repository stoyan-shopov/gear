
/* this is the default stack size, in words */
#define STACK_SIZE	64
/* an arbitrary value to use for filling the stack */
#define STACK_WORD_INIT_VAL	0xccaa55cc
/* the scratch area size, in bytes, to be used by the
 * gear engine flash loading code */
#define SCRATCH_AREA_SIZE	0x200

.extern	main
.extern flash_init
.extern flash_shutdown
.extern flash_erase
.extern flash_write

/* export the gear engine flash data area */
.global gear_flash_data
.global _asm_startup
.code 32
.text

_asm_startup:

	/* setup the stack */
	ldr	sp,	=stack_end

	/* branch to the main c supplied routine */
	ldr	r0,	=main
	bx	r0

/* the gear engine flash loader halt location */

gear_engine_halt_location:
	b	gear_engine_halt_location


.data
/* this is an area to be used as the program stack */
.fill STACK_SIZE, 4, STACK_WORD_INIT_VAL
stack_end:

.align 4

gear_flash_data:
/* void	* scratch_area;	*/
.word	scratch_area
/* int	scratch_area_len; */
.word	SCRATCH_AREA_SIZE
/* the gear engine flash loader halt location */
.word	gear_engine_halt_location
/* int	(*flash_init)(void); */
.word	flash_init
/* int	(*flash_shutdown)(void); */
.word	flash_shutdown
/* int	(*flash_erase)(void * addr, int size); */
.word	flash_erase
/* int	(*flash_write)(void * addr, void * data, int size); */
.word	flash_write

scratch_area:
.fill SCRATCH_AREA_SIZE, 1, 0

.end

