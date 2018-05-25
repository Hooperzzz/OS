
/*++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 console.c
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
 Forrest Yu, 2005
 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++*/

/*
	回车键: 把光标移到第一列
	换行键: 把光标前进到下一行
    TAB键：最多体现为4个空格，可实现纵向对齐，可整体删除
 */


#include "type.h"
#include "const.h"
#include "protect.h"
#include "string.h"
#include "proc.h"
#include "tty.h"
#include "console.h"
#include "global.h"
#include "keyboard.h"
#include "proto.h"

PRIVATE void set_cursor(unsigned int position);
PRIVATE void set_video_start_addr(u32 addr);
PRIVATE void flush(CONSOLE* p_con);

PRIVATE u8* esc_p_vmem;
PRIVATE int esc_and_enter;
PRIVATE int esc_cursor;
/*======================================================================*
 init_screen
 *======================================================================*/
PUBLIC void init_screen(TTY* p_tty)
{
    int nr_tty = p_tty - tty_table;
    p_tty->p_console = console_table + nr_tty;
    
    int v_mem_size = V_MEM_SIZE >> 1;	/* 显存总大小 (in WORD) */
    
    int con_v_mem_size                   = v_mem_size / NR_CONSOLES;
    p_tty->p_console->original_addr      = nr_tty * con_v_mem_size;
    p_tty->p_console->v_mem_limit        = con_v_mem_size;
    p_tty->p_console->current_start_addr = p_tty->p_console->original_addr;
    
    /* 默认光标位置在最开始处 */
    p_tty->p_console->cursor = p_tty->p_console->original_addr;
    
    if (nr_tty == 0) {
        /* 第一个控制台沿用原来的光标位置 */
        p_tty->p_console->cursor = disp_pos / 2;
        disp_pos = 0;
    }
    else {
        out_char(p_tty->p_console, nr_tty + '0');
        out_char(p_tty->p_console, '#');
    }
    
    esc_p_vmem = 0;
    esc_and_enter = 0;
    esc_cursor = 0;
    
    set_cursor(p_tty->p_console->cursor);
    
    u8* p_vmem = (u8*)(V_MEM_BASE + p_tty->p_console->cursor * 2);
    for (int temp_p_cursor = p_tty->p_console->original_addr; temp_p_cursor<=p_tty->p_console->original_addr + p_tty->p_console->v_mem_limit - 1; temp_p_cursor++) {
        u8* temp_p_vmem = (u8*)(V_MEM_BASE + temp_p_cursor * 2);
        *p_vmem++ = '\0';
        *p_vmem++ = DEFAULT_CHAR_COLOR;
    }
}


/*======================================================================*
 is_current_console
 *======================================================================*/
PUBLIC int is_current_console(CONSOLE* p_con)
{
    return (p_con == &console_table[nr_current_console]);
}


/*======================================================================*
 out_char
 *======================================================================*/
PUBLIC void out_char(CONSOLE* p_con, char ch)
{
    //当前显示地址
    u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
    
    
    switch(ch) {
        case '\n':
            if (is_esc_mode) {
                int str_len = p_con->cursor - esc_cursor-1;
                int equal_flag = 0;
                for (int temp_p_cursor = p_con->current_start_addr; temp_p_cursor<=esc_cursor; temp_p_cursor++) {
                    u8* temp_p_vmem = (u8*)(V_MEM_BASE + temp_p_cursor * 2);
                    for (int i=0; i<str_len*2; i=i+2) {
                        
                        if (*(temp_p_vmem+i) != *(esc_p_vmem+i)) {
                            equal_flag = 0;
                            break;
                        } else {
                            equal_flag = 1;
                        }
                    }
                    if (equal_flag==1) {
                        for (int i=0; i<str_len; i++) {
                            temp_p_vmem = temp_p_vmem+1;
                            *temp_p_vmem = ESC_CHAR_COLOR;
                            temp_p_vmem = temp_p_vmem + 1;
                        }
                    }
                }
                esc_and_enter = 1;
            }else{
                *p_vmem++ = 0x0A;
                *p_vmem++ = SPECIAL_CHAR_COLOR;
                if (p_con->cursor < p_con->original_addr +
                    p_con->v_mem_limit - SCREEN_WIDTH) {
                    p_con->cursor = p_con->original_addr + SCREEN_WIDTH *
                    ((p_con->cursor - p_con->original_addr) /
                     SCREEN_WIDTH + 1);
                }
            }
            break;
        case '\b':
            if (!esc_and_enter) {
                if (p_con->cursor > p_con->original_addr) {
                    u8* temp_p_vmem = (u8*)(V_MEM_BASE + (p_con->cursor-1) * 2);
                    //                                    *p_vmem++ = *temp_p_vmem;
                    //                                    *p_vmem++ = TEST_COLOR;
                    if (*temp_p_vmem == '\0') {
                        do{
                            p_con->cursor--;
                            temp_p_vmem = temp_p_vmem-2;
                        }while (*temp_p_vmem =='\0');
                        p_con->cursor--;
                        u8* temp_vmem = (u8*)(V_MEM_BASE + (p_con->cursor) * 2);
                        *temp_vmem = ' ';
                        *(temp_vmem+1) = DEFAULT_CHAR_COLOR;
                    } else if(*temp_p_vmem == 0x09)
                    {
                        int count = 4;
                        do{
                            p_con->cursor--;
                            count--;
                            *(p_vmem-2) = '\0';
                            *(p_vmem-1) = DEFAULT_CHAR_COLOR;
                            temp_p_vmem = temp_p_vmem-2;
                        }while (*temp_p_vmem == 0x09 && count>0);
                        u8* temp_vmem = (u8*)(V_MEM_BASE + (p_con->cursor) * 2);
                        *temp_vmem = ' ';
                        *(temp_vmem+1) = DEFAULT_CHAR_COLOR;
                    } else
                    {
                        p_con->cursor--;
                        *(p_vmem-2) = '\0';
                        *(p_vmem-1) = DEFAULT_CHAR_COLOR;
                    }
                }
            }
            break;
        case '\t':
            if (!esc_and_enter) {
                if (p_con->cursor + 4 < p_con->original_addr +
                    p_con->v_mem_limit) {
                    //这一行第一个字符在显存中的位置
                    int first_cursor = p_con->original_addr+((p_con->cursor - p_con->original_addr)/SCREEN_WIDTH )* SCREEN_WIDTH;
                    int count = 4 - (p_con->cursor - first_cursor) % 4 ;
                    for(int i=0; i<count; i++){
                        *p_vmem++ = 0x09;
                        *p_vmem++ = SPECIAL_CHAR_COLOR;
                        p_con->cursor ++;
                    }
                }
            }
            break;
        case '\e':
            if (!is_esc_mode) {
                esc_cursor = p_con->cursor - 1;
                esc_p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
                is_esc_mode = 1;
            } else
            {
                while (p_vmem > esc_p_vmem) {
                    *(p_vmem-2) = ' ';
                    *(p_vmem-1) = DEFAULT_CHAR_COLOR;
                    p_con->cursor--;
                    p_vmem = p_vmem - 2;
                }
                
                for (int temp_p_cursor = p_con->current_start_addr; temp_p_cursor<=p_con->cursor; temp_p_cursor++) {
                    u8* temp_p_vmem = (u8*)(V_MEM_BASE + temp_p_cursor * 2);
                    if (*temp_p_vmem == 0x09 || *temp_p_vmem == 0x0A) {
                        temp_p_vmem++;
                        *temp_p_vmem = SPECIAL_CHAR_COLOR;
                    }else{
                        temp_p_vmem++;
                        *temp_p_vmem = DEFAULT_CHAR_COLOR;
                    }
                }
                esc_p_vmem = 0;
                is_esc_mode = 0;
                esc_and_enter = 0;
            }
            break;
        default:
            if (!esc_and_enter) {
                if (p_con->cursor <
                    p_con->original_addr + p_con->v_mem_limit - 1) {
                    *p_vmem++ = ch;
                    if (is_esc_mode) {
                        *p_vmem++ = ESC_CHAR_COLOR;
                    }
                    else
                    {
                        *p_vmem++ = DEFAULT_CHAR_COLOR;
                    }
                    p_con->cursor++;
                }
            }
            break;
    }
    
    while (p_con->cursor >= p_con->current_start_addr + SCREEN_SIZE) {
        scroll_screen(p_con, SCR_DN);
    }
    
    flush(p_con);
}
/*======================================================================*
 clean_screen
 *======================================================================*/
PUBLIC void clean_screen(CONSOLE* p_con)
{
    p_con->cursor = p_con->original_addr;
    u8* p_vmem = (u8*)(V_MEM_BASE + p_con->cursor * 2);
    while (p_con->cursor < p_con->original_addr + p_con->v_mem_limit - 1) {
        *p_vmem++ = '\0';
        *p_vmem++ = DEFAULT_CHAR_COLOR;
        p_con->cursor ++;
    }
    p_con->cursor = p_con->original_addr;
    p_con->current_start_addr = p_con->original_addr;
    flush(p_con);
}
/*======================================================================*
 flush
 *======================================================================*/
PRIVATE void flush(CONSOLE* p_con)
{
    set_cursor(p_con->cursor);
    set_video_start_addr(p_con->current_start_addr);
}

/*======================================================================*
 set_cursor
 *======================================================================*/
PRIVATE void set_cursor(unsigned int position)
{
    disable_int();
    out_byte(CRTC_ADDR_REG, CURSOR_H);
    out_byte(CRTC_DATA_REG, (position >> 8) & 0xFF);
    out_byte(CRTC_ADDR_REG, CURSOR_L);
    out_byte(CRTC_DATA_REG, position & 0xFF);
    enable_int();
}

/*======================================================================*
 set_video_start_addr
 *======================================================================*/
PRIVATE void set_video_start_addr(u32 addr)
{
    disable_int();
    out_byte(CRTC_ADDR_REG, START_ADDR_H);
    out_byte(CRTC_DATA_REG, (addr >> 8) & 0xFF);
    out_byte(CRTC_ADDR_REG, START_ADDR_L);
    out_byte(CRTC_DATA_REG, addr & 0xFF);
    enable_int();
}



/*======================================================================*
 select_console
 *======================================================================*/
PUBLIC void select_console(int nr_console)	/* 0 ~ (NR_CONSOLES - 1) */
{
    if ((nr_console < 0) || (nr_console >= NR_CONSOLES)) {
        return;
    }
    
    nr_current_console = nr_console;
    
    set_cursor(console_table[nr_console].cursor);
    set_video_start_addr(console_table[nr_console].current_start_addr);
}

/*======================================================================*
 scroll_screen
 *----------------------------------------------------------------------*
 滚屏.
 *----------------------------------------------------------------------*
 direction:
	SCR_UP	: 向上滚屏
	SCR_DN	: 向下滚屏
	其它	: 不做处理
 *======================================================================*/
PUBLIC void scroll_screen(CONSOLE* p_con, int direction)
{
    if (direction == SCR_UP) {
        if (p_con->current_start_addr > p_con->original_addr) {
            p_con->current_start_addr -= SCREEN_WIDTH;
        }
    }
    else if (direction == SCR_DN) {
        if (p_con->current_start_addr + SCREEN_SIZE <
            p_con->original_addr + p_con->v_mem_limit) {
            p_con->current_start_addr += SCREEN_WIDTH;
        }
    }
    else{
    }
    
    set_video_start_addr(p_con->current_start_addr);
    set_cursor(p_con->cursor);
}


