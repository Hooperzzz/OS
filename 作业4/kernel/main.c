
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                            main.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "proto.h"


/*======================================================================*
                            kernel_main
 *======================================================================*/
PUBLIC int kernel_main()
{
	disp_str("-----\"kernel_main\" begins-----\n");

	TASK*		p_task		= task_table;
	PROCESS*	p_proc		= proc_table;
	char*		p_task_stack	= task_stack + STACK_SIZE_TOTAL;
	u16		selector_ldt	= SELECTOR_LDT_FIRST;
	int i;
	for (i = 0; i < NR_TASKS; i++) {
		strcpy(p_proc->p_name, p_task->name);	// name of the process
		p_proc->pid = i;			// pid

		p_proc->ldt_sel = selector_ldt;

		memcpy(&p_proc->ldts[0], &gdt[SELECTOR_KERNEL_CS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[0].attr1 = DA_C | PRIVILEGE_TASK << 5;
		memcpy(&p_proc->ldts[1], &gdt[SELECTOR_KERNEL_DS >> 3],
		       sizeof(DESCRIPTOR));
		p_proc->ldts[1].attr1 = DA_DRW | PRIVILEGE_TASK << 5;
		p_proc->regs.cs	= ((8 * 0) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ds	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.es	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.fs	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.ss	= ((8 * 1) & SA_RPL_MASK & SA_TI_MASK)
			| SA_TIL | RPL_TASK;
		p_proc->regs.gs	= (SELECTOR_KERNEL_GS & SA_RPL_MASK)
			| RPL_TASK;

		p_proc->regs.eip = (u32)p_task->initial_eip;
		p_proc->regs.esp = (u32)p_task_stack;
		p_proc->regs.eflags = 0x1202; /* IF=1, IOPL=1 */

		p_task_stack -= p_task->stacksize;
		p_proc++;
		p_task++;
		selector_ldt += 1 << 3;
	}

	proc_table[0].ticks = proc_table[0].priority = 1;
	proc_table[1].ticks = proc_table[1].priority = 1;
	proc_table[2].ticks = proc_table[2].priority = 1;
	proc_table[3].ticks = proc_table[3].priority = 1;
	proc_table[4].ticks = proc_table[4].priority = 1;

	proc_table[0].wait = proc_table[0].sleep_ticks = 0;
	proc_table[1].wait = proc_table[1].sleep_ticks = 0;
	proc_table[2].wait = proc_table[2].sleep_ticks = 0;
	proc_table[3].wait = proc_table[3].sleep_ticks = 0;
	proc_table[4].wait = proc_table[4].sleep_ticks = 0;

	proc_table[0].next = 0;
    proc_table[1].next = 0;
	proc_table[2].next = 0;
	proc_table[3].next = 0;
	proc_table[4].next = 0;

	k_reenter = 0;
	ticks = 0;

	p_proc_ready	= proc_table;

	init_clock();
    init_keyboard();

	restart();

	while(1){}
}

/*======================================================================*
                               Barber
 *======================================================================*/
void Barber()
{
	init();
	barber();
}

/*======================================================================*
                               Customer_A
 *======================================================================*/
void Customer_A()
{
	while(1){
	customers("A");
	system_process_sleep(10000);
	}
}

/*======================================================================*
                               Customer_B
 *======================================================================*/
void Customer_B()
{
	while(1){
	customers("B");
	system_process_sleep(10000);
	}
}

/*======================================================================*
                               Customer_C
 *======================================================================*/
void Customer_C()
{
	while(1){
	customers("C");
	system_process_sleep(10000);
	}
}
