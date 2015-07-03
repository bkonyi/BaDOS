#ifndef _TERMINAL_CONTROL_H_
#define _TERMINAL_CONTROL_H_

/**
 * Helper functions for issuing common terminal escape codes.
 */

void term_hide_cursor(void);
void term_show_cursor(void);

void term_red_text(void);
void term_green_text(void);
void term_blue_text(void);
void term_fmt_clr(void);
//Term  cmds
void term_clear(void);
void term_clear_whole_line(void);
void term_clear_rest_line(void);
void term_move_cursor(int col, int row);
void term_save_cursor(void);
void term_restore_cursor(void);
void term_move_to_column(int col);
#endif //_TERMINAL_CONTROL_H_
