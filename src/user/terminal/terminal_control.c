#include <terminal/terminal.h>
#include <io.h>

void term_hide_cursor(void){
  printf(COM2,"\033[?25l");
}
void term_show_cursor(void){
  printf(COM2,"\033[?25h");
}


void term_red_text(void){
   printf(COM2,"\033[31m");
}
void term_green_text(void){
    printf(COM2,"\033[32m");
}
void term_blue_text(void){
    printf(COM2,"\033[34m");
}
void term_fmt_clr(void){
    printf(COM2,"\033[0m");
}
//Term  cmds
void term_clear(void){
    printf(COM2,"\033[2J");
}
void term_clear_whole_line(void){
    printf(COM2,"\033[2K");
}
void term_clear_rest_line(void){
    printf(COM2,"\033[K");
}
void term_move_cursor(int col, int row){
   printf(COM2,"\033[%d;%dH",row,col);
}
void term_save_cursor(void){
   printf(COM2,"\033[s");
}
void term_restore_cursor(void){
   printf(COM2,"\033[u");
}
