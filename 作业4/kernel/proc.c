
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                               proc.c
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
                                                    Forrest Yu, 2005
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

#include "type.h"
#include "const.h"
#include "protect.h"
#include "tty.h"
#include "console.h"
#include "string.h"
#include "proc.h"
#include "global.h"
#include "proto.h"


PRIVATE char buffer[10];
PRIVATE int now;
PRIVATE int ID;
PRIVATE int waiting;
PRIVATE int chairs;
PRIVATE SEMAPHORE 	semaphores[3] = {{0, 0},{0, 0},{1, 0},
              									//customers, barber, mutex
												};
PRIVATE SEMAPHORE*  first_semaphore;
/*======================================================================*
                              schedule
 *======================================================================*/
PUBLIC void schedule()
{
	PROCESS* p;
	int	 greatest_ticks = 0;

	//minus sleep time
	for (p = proc_table; p < proc_table+NR_TASKS; p++) {
		if(p -> sleep_ticks > 0){
			p -> sleep_ticks--;
		}
	}

	while (!greatest_ticks) {
		for (p = proc_table; p < proc_table+NR_TASKS; p++) {
			if (p -> wait || p -> sleep_ticks){
				continue;
			}

			if (p->ticks > greatest_ticks) {
				greatest_ticks = p->ticks;
				p_proc_ready = p;
			}
		}

		if (!greatest_ticks) {
			for (p = proc_table; p < proc_table+NR_TASKS; p++) {
				if (p -> wait || p -> sleep_ticks){
					continue;
				}
				p->ticks = p->priority;
			}
		}
	}
}

/*======================================================================*
                           sys_get_ticks
 *======================================================================*/
PUBLIC int sys_get_ticks()
{
	return ticks;
}

/*======================================================================*
                           sys_process_sleep
 *======================================================================*/
PUBLIC int sys_process_sleep(int milli_sec){
	p_proc_ready -> sleep_ticks = milli_sec / 10;
	schedule();

	return 0;
}

/*======================================================================*
                           sys_disp_str
 *======================================================================*/
PUBLIC int sys_disp_str(char* str){
	//choose color for each process
	char color = colors[p_proc_ready - proc_table - 1];

	//output str
	char* temp = str;
    while (*temp != 0){
        out_char(current_con, *temp, color);
        temp++;
    }

    return 0;
}

/*======================================================================*
                           sys_P
 *======================================================================*/
PUBLIC int sys_P(SEMAPHORE* s){
	s->value--;
	if(s->value < 0){
		//sleep process
		p_proc_ready->wait = 1;
		if(s -> queue == 0){
			s->queue = p_proc_ready;
		}
		else{
			PROCESS* rear = s->queue;
			while(rear->next != 0){
				rear = rear -> next;
			}
			rear->next = p_proc_ready;
		}
		//
		schedule();
	}
	return 0;
}

/*======================================================================*
                           sys_V
 *======================================================================*/
PUBLIC int sys_V(SEMAPHORE* s){
	s->value++;
	if(s->value <= 0){
		//wake up process
		p_proc_ready = s->queue;
		
		s->queue = s-> queue -> next;
		p_proc_ready->next = 0;
		p_proc_ready->wait = 0;
		
	}
	return 0;
}

/*======================================================================*
                           init
 *======================================================================*/
PUBLIC void init(){
	ID = 0;
	waiting = 0;
	chairs = 3;
	first_semaphore = semaphores;
}

/*======================================================================*
                           barber
 *======================================================================*/
PUBLIC void barber(){
	while(1){
		system_disp_str("Barber is sleeping\n");

		system_P(first_semaphore);
		system_P(first_semaphore + 2);
		waiting--;
		system_V(first_semaphore + 1);
		system_V(first_semaphore + 2);

		system_process_sleep(20000);
		system_disp_str("Barber finished cutting for ");
		itoa(buffer, now);
		system_disp_str(buffer);
		system_disp_str(" customer\n");
	}
}

/*======================================================================*
                           customers
 *======================================================================*/
PUBLIC void customers(char* name){
	system_P(first_semaphore + 2);
	ID++;
	int count = ID;
	system_disp_str("Customer ");
	itoa(buffer, count);
	system_disp_str(buffer);
	system_disp_str(" come. ");
	itoa(buffer, waiting);
	system_disp_str(buffer);
	system_disp_str(" people waiting\n");
	if(waiting < chairs){
		system_disp_str("Customer ");
		itoa(buffer, count);
		system_disp_str(buffer);
		system_disp_str(" waiting \n");

		waiting++;
		system_V(first_semaphore);
		system_V(first_semaphore + 2);
		system_P(first_semaphore + 1);

		system_disp_str("Customer ");
		itoa(buffer, count);
		system_disp_str(buffer);
		system_disp_str(" get hair cut\n");
		now = count;
	}
	else{
		system_V(first_semaphore + 2);

		system_disp_str("Customer ");
		itoa(buffer, count);
		system_disp_str(buffer);
		system_disp_str(" leave\n");
	}
}


