
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
			if (p -> is_wait || p -> sleep_ticks){
				continue;
			}

			if (p->ticks > greatest_ticks) {
				greatest_ticks = p->ticks;
				p_proc_ready = p;
			}
		}

		if (!greatest_ticks) {
			for (p = proc_table; p < proc_table+NR_TASKS; p++) {
				if (p -> is_wait || p -> sleep_ticks){
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
	char color = colors[p_proc_ready - proc_table - 1];
	char* temp = str;
    while (*temp != 0){
        out_char_color(current_con, *temp, color);
        temp++;
    }

    return 0;
}

/*======================================================================*
                           sys_sem_p
 *======================================================================*/
PUBLIC int sys_sem_p(SEMAPHORE* s){
	s->value--;
	if(s->value < 0){
		p_proc_ready->is_wait = 1;
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
		schedule();
	}
	return 0;
}

/*======================================================================*
                           sys_sem_v
 *======================================================================*/
PUBLIC int sys_sem_v(SEMAPHORE* s){
	s->value++;
	if(s->value <= 0){
		//wake up process
		p_proc_ready = s->queue;
		
		s->queue = s-> queue -> next;
		p_proc_ready->next = 0;
		p_proc_ready->is_wait = 0;
		
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
		disp_str("Barber is sleeping\n");

		sem_p(first_semaphore);
		sem_p(first_semaphore + 2);
		waiting--;
		sem_v(first_semaphore + 1);
		sem_v(first_semaphore + 2);

		process_sleep(20000);
		disp_str_s("Barber finished cutting for ");
		itoa(buffer, now);
		disp_str_s(buffer);
		disp_str_s(" customer\n");
	}
}

/*======================================================================*
                           customers
 *======================================================================*/
PUBLIC void customers(char* name){
	sem_p(first_semaphore + 2);
	ID++;
	int count = ID;
	disp_str("Customer ");
	itoa(buffer, count);
	disp_str(buffer);
	disp_str(" come. ");
	itoa(buffer, waiting);
	disp_str(buffer);
	disp_str(" people waiting\n");
	if(waiting < chairs){
		disp_str("Customer ");
		itoa(buffer, count);
		disp_str(buffer);
		disp_str(" waiting \n");

		waiting++;
		sem_v(first_semaphore);
		sem_v(first_semaphore + 2);
		sem_p(first_semaphore + 1);

		disp_str("Customer ");
		itoa(buffer, count);
		disp_str(buffer);
		disp_str(" get hair cut\n");
		now = count;
	}
	else{
		sem_v(first_semaphore + 2);

		disp_str("Customer ");
		itoa(buffer, count);
		disp_str(buffer);
		disp_str(" leave\n");
	}
}
