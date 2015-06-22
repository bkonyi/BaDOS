#include <terminal/terminal.h>
#include <common.h>
#include <io/io.h>
#include <servers.h>
#include <syscalls.h>
#include <io/io_common.h>
#include <terminal/terminal_control.h>
#include <ring_buffer.h>
#include <task_priorities.h>
#include <track_maps.h>

#define RIGHT_BAR_COL             75
#define TERM_SENSORS_ROW          4
#define TERM_SENSORS_DATA_ROW     TERM_SENSORS_ROW + 3
#define TERM_SENSORS_DATA_WIDTH   4

#define TERM_TIME_ROW TERM_SENSORS_ROW + 12
#define TERM_TIME_COL 54

#define TERM_SWITCHES_DATA_ROW       TERM_SENSORS_ROW + 4
#define TERM_SWITCHES_DATA_COLUMN    48
#define TERM_SWITCHES_COL_WIDTH      3
#define TERM_SWITCHES_BIG_COL_WIDTH  6
#define TERM_SWITCHES_ROW_HEIGHT     3
#define TERM_INPUT_ROW               TERM_SENSORS_ROW + 16
#define TERM_INPUT_COORDS            10, TERM_INPUT_ROW
#define TERM_STATUS_ROW              (TERM_INPUT_ROW-2)
#define TERM_STATUS_COORDS           11, TERM_STATUS_ROW

#define MAP_ROW                      TERM_INPUT_ROW+2
#define MAP_COL                      3
#define MAP_COORDS                   MAP_COL,MAP_ROW

#define NUM_RECENT_SENSORS           10
#define RECENT_SENSORS_DATA_ROW      TERM_SENSORS_DATA_ROW 
#define RECENT_SENSORS_DATA_COLUMN   38
CREATE_NON_POINTER_BUFFER_TYPE(recent_sensors_buffer_t, int, NUM_RECENT_SENSORS);


static void handle_update_terminal_clock(int32_t ticks);
static void handle_update_sensors(sensor_map_chars_t* sensor_chars, char* previous_sensors, char* sensors, int* recent_sensors, int* recent_sensors_index);
static void terminal_tick_notifier(void);
static void handle_train_command(int32_t num,int32_t speed);
static void handle_reverse_command(int32_t num);
static void handle_switch_command(int32_t num,char state);
static void handle_quit_command(void);
static void handle_start_command(void);
static void handle_stop_command(void);
static void clear_user_input(void);
static void status_message(char* fmt, ...);
static void draw_initial_track_map(void);
static void track_a_sensor_char_init(sensor_map_chars_t* sensor_chars);
static void update_map_sensor(sensor_map_chars_t* sensor_chars,int32_t group, int32_t index,bool state);
#define MAP_DRAW_COORDS(i,j) ((i*16)+j)

void terminal_server(void) {
    int sending_tid;
    int result;
    int buffer_char_count =0;   //keep track of how many chars are in the user in buff
    terminal_data_t data;

    char previous_sensors[10];
    int recent_sensors[NUM_RECENT_SENSORS];
    int recent_sensors_index = 0;

    int i;
    for(i = 0; i < 10; ++i) {
        previous_sensors[i] = 0;
        recent_sensors[i] = -1;
    }

    RegisterAs(TERMINAL_SERVER);

    term_clear();
    //term_move_cursor(1,1);
    //printf(COM2, "Time:");

    term_move_cursor(1, 1);
    printf(COM2, "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\r\n");
    printf(COM2, "┃                                   \e[32;1;4mBADos\e[0m                                 ┃\r\n");
    printf(COM2, "┃                  \e[2mCreated by: Dan Chevalier and Ben Konyi\e[0m                ┃\r\n");
    printf(COM2, "┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\r\n");
    printf(COM2, "┃        \e[1;96mSENSOR STATES\e[0m           ┃  \e[1;91mRECENT\e[0m  ┃        \e[1;35mSWITCH STATES\e[0m        ┃\r\n");
    printf(COM2, "┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\r\n");
    printf(COM2, "┃A1  A2  A3  A4  A5  A6  A7  A8  ┃          ┃  1  2  3  4  5  6  7  8  9  ┃\r\n");
    printf(COM2, "┃A9  A10 A11 A12 A13 A14 A15 A16 ┃          ┃  ?  ?  ?  ?  ?  ?  ?  ?  ?  ┃\r\n");
    printf(COM2, "┃B1  B2  B3  B4  B5  B6  B7  B8  ┃          ┃                             ┃\r\n");
    printf(COM2, "┃B9  B10 B11 B12 B13 B14 B15 B16 ┃          ┃  10 11 12 13 14 15 16 17 18 ┃\r\n");
    printf(COM2, "┃C1  C2  C3  C4  C5  C6  C7  C8  ┃          ┃  ?  ?  ?  ?  ?  ?  ?  ?  ?  ┃\r\n");
    printf(COM2, "┃C9  C10 C11 C12 C13 C14 C15 C16 ┃          ┃                             ┃\r\n");
    printf(COM2, "┃D1  D2  D3  D4  D5  D6  D7  D8  ┃          ┃    153   154   155   156    ┃\r\n");
    printf(COM2, "┃D9  D10 D11 D12 D13 D14 D15 D16 ┃          ┃     ?     ?     ?     ?     ┃\r\n");
    printf(COM2, "┃E1  E2  E3  E4  E5  E6  E7  E8  ┃          ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\r\n");
    printf(COM2, "┃E9  E10 E11 E12 E13 E14 E15 E16 ┃          ┃ \e[1;34mTIME:\e[0m                       ┃\r\n");
    printf(COM2, "┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\r\n");
    printf(COM2, "┃ \e[33;1mResult:\e[0m                                                                 ┃\r\n");
    printf(COM2, "┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\r\n");
    printf(COM2, "┃ \e[32;1mInput:\e[0m                                                                  ┃\r\n");
    printf(COM2, "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\r\n");
    term_fmt_clr();

    Create(TERMINAL_TICK_NOTIFIER, terminal_tick_notifier);

    draw_initial_track_map();
    sensor_map_chars_t sensor_chars[16*16];
    track_a_sensor_char_init(sensor_chars);
    //IMPORTANT: This needs to be the last coord we set so user input is in the right place
    term_move_cursor(TERM_INPUT_COORDS);

    FOREVER {
        result = Receive(&sending_tid, (char*)&data, sizeof(terminal_data_t));
        ASSERT(result >= 0);

        //Just send a null reply for now
        Reply(sending_tid, (char*)NULL, 0);

        switch(data.command) {
            case TERMINAL_UPDATE_CLOCK:
                handle_update_terminal_clock(data.num1);
                break;
            case TERMINAL_ECHO_INPUT:
                if(data.byte1 != CARRIAGE_RETURN  ) {
                    if(data.byte1 == BACKSPACE){
                        if(buffer_char_count>0){
                            buffer_char_count--; 
                            printf(COM2,"%c %c",BACKSPACE,BACKSPACE);
                        }//otherwise we don't need to do anything
                        continue;
                    }else{
                        buffer_char_count++;
                        putc(COM2, data.byte1);
                    }
                }else{
                    buffer_char_count=0;
                }
                break;
            case TERMINAL_TRAIN_COMMAND:
                handle_train_command(data.num1,data.num2);
                break;
            case TERMINAL_SWITCH_COMMAND:
                handle_switch_command(data.num1,data.byte1);
                break;
            case TERMINAL_REVERSE_COMMAND:
                handle_reverse_command(data.num1);
                break;
            case TERMINAL_QUIT:
                handle_quit_command();
                break;
            case TERMINAL_START_CTRL:
                handle_start_command();
                break;
            case TERMINAL_STOP_CTRL:
                handle_stop_command();
                break;
            case TERMINAL_UPDATE_SENSORS:
                handle_update_sensors(sensor_chars,previous_sensors, data.sensors, recent_sensors, &recent_sensors_index);
                break;
            case TERMINAL_COMMAND_ERROR:
                status_message("Input Error");
                clear_user_input();
                break;
            default:
                ASSERT(0);
                break;
        }
    }
}

void terminal_tick_notifier(void) {
    int32_t ticks;
    FOREVER {
        ticks = Delay(10);
        update_terminal_clock(ticks); //Delay 100ms
    }
}

void update_terminal_clock(int32_t ticks) {
    terminal_data_t request;
    request.command = TERMINAL_UPDATE_CLOCK;
    request.num1   = ticks;
    Send(TERMINAL_SERVER_ID, (char*)&request, sizeof(terminal_data_t), (char*)NULL, 0);
}

void update_terminal_sensors_display(int8_t* sensors) {
    terminal_data_t request;
    request.command = TERMINAL_UPDATE_SENSORS;
    memcpy(request.sensors, sensors, sizeof(int8_t) * 10);

    Send(TERMINAL_SERVER_ID, (char*)&request, sizeof(terminal_data_t), (char*)NULL, 0);
}

void handle_update_terminal_clock(int32_t ticks) {
    term_save_cursor();
    term_hide_cursor();
    term_move_cursor(TERM_TIME_COL,TERM_TIME_ROW);
    ticks = Time();
    printf(COM2, "%d:%d.%d ", (ticks / 100) / 60, (ticks / 100) % 60, (ticks / 10) % 10);
    term_restore_cursor();
    term_show_cursor();
}

void handle_update_sensors(sensor_map_chars_t* sensor_chars, char* previous_sensors, char* sensors, int* recent_sensors, int* recent_sensors_index) {
    term_save_cursor();
    term_hide_cursor();
    int i;
    bool sensor_changed = false;
    int group,index;
    for(i = 0; i < 10; ++i) {
        //If the sensor state hasn't changed, don't update anything
        if(previous_sensors[i] != sensors[i]) {
            sensor_changed = true;

            int j;
            int previous_sensor_data = previous_sensors[i];
            int new_sensor_data = sensors[i];

            //Compare sensor bits from the most significant to the least significant
            for(j = 0; j < 8; ++j) {
                //If a sensor state has been change since last update, we'll update it
                if((previous_sensor_data & 0x1) != (new_sensor_data & 0x1)) {
                    //Find the location of the sensor in the display
                    //Note: it's 7 - j since we're going from most significant to least
                    term_move_cursor(2 + (7 - j) * TERM_SENSORS_DATA_WIDTH, TERM_SENSORS_DATA_ROW + i);

                    group = (i / 2);
                    index = ((i * 8) % 16) + (7 - j) + 1;

                    //If the sensor has been triggered...
                    if(new_sensor_data & 0x1) {
                        //Set the text to green
                        term_green_text();

                        //Convert the sensor to a number which represents the switch and push it
                        //into a buffer of most recently triggered sensors
                        recent_sensors[*recent_sensors_index] = i * 8 + (7 - j);
                        *recent_sensors_index = (*recent_sensors_index + 1) % 10;
                        update_map_sensor(sensor_chars,group, index-1,true);
                    }else {
                        update_map_sensor(sensor_chars,group, index-1,false);
                    }
                    
                    //Update the sensor label
                    printf(COM2, "%c%d", 'A' + group, index);
                    
                    if(new_sensor_data & 0x1) {
                        term_fmt_clr();
                    }
                }

                previous_sensor_data >>= 1;
                new_sensor_data >>= 1;
            }

            //Keep track of the previous sensor states
            previous_sensors[i] = sensors[i];
        }
    }

    //If the sensor hasn't changed, we have nothing to update
    if(sensor_changed) {
        int value;
        int index = (*recent_sensors_index - 1) % 10; //We increment whenever we push into the buffer, so just subtract 1
                                                      //To find the first element of the buffer
        int i;

        if(index < 0) {
            index = 9;
        }

        for(i = 0; i < 10; ++i) {
            value = recent_sensors[index];

            //Check to see if there's any more elements to print
            if(value < 0) {
                break;
            }

            term_move_cursor(RECENT_SENSORS_DATA_COLUMN, RECENT_SENSORS_DATA_ROW + i);
            char letter = (value / 16) + 'A';
            int number = (value % 16) + 1;

            //We go through the buffer backwards as the most recent element is at the back
            if(--index < 0) {
                index = 9;
            }

            //Print the sensor label
            printf(COM2, "%c%d ", letter, number); //Extra space is to overwrite any two digit numbers previously there
        }
    }
    term_restore_cursor();
    term_show_cursor();
}

/**
 * @brief function that takes the same parameters that printf would take but move the cursor 
 * to the location of the screen where we want our status message to go
 */
void status_message(char* fmt, ...){
    term_hide_cursor();
    term_move_cursor(TERM_STATUS_COORDS);
    term_clear_rest_line();
    va_list va;
    va_start(va,fmt);
    vprintf(COM2,fmt,va);
    va_end(va);
    term_move_cursor(RIGHT_BAR_COL,TERM_STATUS_ROW);
    printf(COM2,"┃");

    clear_user_input();
}
void clear_user_input(void) {

    //Clear the line
    term_move_cursor(TERM_INPUT_COORDS);
    term_clear_rest_line();
    //Rewrite the bar
    term_move_cursor(RIGHT_BAR_COL,TERM_INPUT_ROW);
    printf(COM2,"┃");
    //Return the cursor
    term_move_cursor(TERM_INPUT_COORDS);
    term_show_cursor();
}
void handle_train_command(int32_t num,int32_t speed){
    status_message("CMD: 'TR' #: '%d' Speed: '%d'",num,speed);
}
void handle_reverse_command(int32_t num){
    status_message("CMD: 'RV' #: '%d'",num);
}
void handle_switch_command(int32_t num,char state){
    status_message("CMD 'SW' #: '%d' State: '%c'",num,state);

    term_hide_cursor();
    term_save_cursor();
    num -= 1;

    //We've got 4 switches with weird assignments, so we need to have a special case for them
    if(num <= 17) {
        term_move_cursor(TERM_SWITCHES_DATA_COLUMN + TERM_SWITCHES_COL_WIDTH * (num % 9), TERM_SWITCHES_DATA_ROW + TERM_SWITCHES_ROW_HEIGHT * (num / 9));
    } else {
        num -= 152;
        term_move_cursor((TERM_SWITCHES_DATA_COLUMN + 3) + TERM_SWITCHES_BIG_COL_WIDTH * num, TERM_SWITCHES_DATA_ROW + TERM_SWITCHES_ROW_HEIGHT * 2);
    }

    printf(COM2, "%c", char_to_upper(state));

    term_restore_cursor();
    term_show_cursor();
}
void handle_quit_command(void){
    status_message("CMD QUIT");
    Terminate();
}

void handle_start_command(void) {
    status_message("ENABLING TRAIN CONTROLLER");
}

void handle_stop_command(void) {
    status_message("STOPPING TRAINS AND TURNING OFF CONTROLLER");
}

void draw_initial_track_map(void){
    char tracka[TRACK_SIZE_Y][TRACK_SIZE_X] = TRACKA_STR_ARRAY;
    int i = 0;

    term_save_cursor();
    term_move_cursor(MAP_COORDS);
    for(i = 0; i < TRACK_SIZE_Y; i++) {
        term_move_cursor(MAP_COL,MAP_ROW+i);
        printf(COM2,"%s",tracka[i]);
    }


    term_restore_cursor();
}

void update_map_sensor(sensor_map_chars_t* sensor_chars,int32_t group, int32_t index,bool state) {
    sensor_map_chars_t* data = &sensor_chars[MAP_DRAW_COORDS(group,index)];
     term_save_cursor();
     term_move_cursor(MAP_COL + data->x,MAP_ROW + data->y);
     if (state == true) {
        putc(COM2,data->activated);
     }else if (state == false) {
        putc(COM2,data->original);
     }
     term_restore_cursor();
}
void track_a_sensor_char_init(sensor_map_chars_t* sensor_chars){
    //inialize mem
    int i,j;
    for(i = 0; i < 16; i++) {
        for(j = 0; j < 16; j++) {
            sensor_chars[MAP_DRAW_COORDS(i,j)].x = -1;
            sensor_chars[MAP_DRAW_COORDS(i,j)].y = -1;
            sensor_chars[MAP_DRAW_COORDS(i,j)].original = '\0';
            sensor_chars[MAP_DRAW_COORDS(i,j)].activated = '\0';
        }   
    }
sensor_chars[MAP_DRAW_COORDS(0,0)].x = 9;
sensor_chars[MAP_DRAW_COORDS(0,0)].y = 0;
sensor_chars[MAP_DRAW_COORDS(0,0)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(0,0)].original = '=';
sensor_chars[MAP_DRAW_COORDS(0,9)].x = 6;
sensor_chars[MAP_DRAW_COORDS(0,9)].y = 12;
sensor_chars[MAP_DRAW_COORDS(0,9)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(0,9)].original = '=';
sensor_chars[MAP_DRAW_COORDS(0,10)].x = 1;
sensor_chars[MAP_DRAW_COORDS(0,10)].y = 11;
sensor_chars[MAP_DRAW_COORDS(0,10)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(0,10)].original = '=';
sensor_chars[MAP_DRAW_COORDS(0,11)].x = 1;
sensor_chars[MAP_DRAW_COORDS(0,11)].y = 11;
sensor_chars[MAP_DRAW_COORDS(0,11)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(0,11)].original = '=';
sensor_chars[MAP_DRAW_COORDS(0,12)].x = 8;
sensor_chars[MAP_DRAW_COORDS(0,12)].y = 1;
sensor_chars[MAP_DRAW_COORDS(0,12)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(0,12)].original = '=';
sensor_chars[MAP_DRAW_COORDS(0,13)].x = 8;
sensor_chars[MAP_DRAW_COORDS(0,13)].y = 1;
sensor_chars[MAP_DRAW_COORDS(0,13)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(0,13)].original = '=';
sensor_chars[MAP_DRAW_COORDS(0,14)].x = 5;
sensor_chars[MAP_DRAW_COORDS(0,14)].y = 2;
sensor_chars[MAP_DRAW_COORDS(0,14)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(0,14)].original = '=';
sensor_chars[MAP_DRAW_COORDS(0,15)].x = 5;
sensor_chars[MAP_DRAW_COORDS(0,15)].y = 2;
sensor_chars[MAP_DRAW_COORDS(0,15)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(0,15)].original = '=';
sensor_chars[MAP_DRAW_COORDS(0,1)].x = 9;
sensor_chars[MAP_DRAW_COORDS(0,1)].y = 0;
sensor_chars[MAP_DRAW_COORDS(0,1)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(0,1)].original = '=';
sensor_chars[MAP_DRAW_COORDS(0,2)].x = 11;
sensor_chars[MAP_DRAW_COORDS(0,2)].y = 5;
sensor_chars[MAP_DRAW_COORDS(0,2)].activated = '^';
sensor_chars[MAP_DRAW_COORDS(0,2)].original = '|';
sensor_chars[MAP_DRAW_COORDS(0,3)].x = 11;
sensor_chars[MAP_DRAW_COORDS(0,3)].y = 5;
sensor_chars[MAP_DRAW_COORDS(0,3)].activated = 'v';
sensor_chars[MAP_DRAW_COORDS(0,3)].original = '|';
sensor_chars[MAP_DRAW_COORDS(0,4)].x = 10;
sensor_chars[MAP_DRAW_COORDS(0,4)].y = 14;
sensor_chars[MAP_DRAW_COORDS(0,4)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(0,4)].original = '=';
sensor_chars[MAP_DRAW_COORDS(0,5)].x = 10;
sensor_chars[MAP_DRAW_COORDS(0,5)].y = 14;
sensor_chars[MAP_DRAW_COORDS(0,5)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(0,5)].original = '=';
sensor_chars[MAP_DRAW_COORDS(0,6)].x = 8;
sensor_chars[MAP_DRAW_COORDS(0,6)].y = 13;
sensor_chars[MAP_DRAW_COORDS(0,6)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(0,6)].original = '=';
sensor_chars[MAP_DRAW_COORDS(0,7)].x = 8;
sensor_chars[MAP_DRAW_COORDS(0,7)].y = 13;
sensor_chars[MAP_DRAW_COORDS(0,7)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(0,7)].original = '=';
sensor_chars[MAP_DRAW_COORDS(0,8)].x = 6;
sensor_chars[MAP_DRAW_COORDS(0,8)].y = 12;
sensor_chars[MAP_DRAW_COORDS(0,8)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(0,8)].original = '=';
sensor_chars[MAP_DRAW_COORDS(1,0)].x = 26;
sensor_chars[MAP_DRAW_COORDS(1,0)].y = 10;
sensor_chars[MAP_DRAW_COORDS(1,0)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(1,0)].original = '=';
sensor_chars[MAP_DRAW_COORDS(1,9)].x = 1;
sensor_chars[MAP_DRAW_COORDS(1,9)].y = 14;
sensor_chars[MAP_DRAW_COORDS(1,9)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(1,9)].original = '=';
sensor_chars[MAP_DRAW_COORDS(1,10)].x = 1;
sensor_chars[MAP_DRAW_COORDS(1,10)].y = 13;
sensor_chars[MAP_DRAW_COORDS(1,10)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(1,10)].original = '=';
sensor_chars[MAP_DRAW_COORDS(1,11)].x = 1;
sensor_chars[MAP_DRAW_COORDS(1,11)].y = 13;
sensor_chars[MAP_DRAW_COORDS(1,11)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(1,11)].original = '=';
sensor_chars[MAP_DRAW_COORDS(1,12)].x = 30;
sensor_chars[MAP_DRAW_COORDS(1,12)].y = 7;
sensor_chars[MAP_DRAW_COORDS(1,12)].activated = '^';
sensor_chars[MAP_DRAW_COORDS(1,12)].original = '\\';
sensor_chars[MAP_DRAW_COORDS(1,13)].x = 30;
sensor_chars[MAP_DRAW_COORDS(1,13)].y = 7;
sensor_chars[MAP_DRAW_COORDS(1,13)].activated = 'v';
sensor_chars[MAP_DRAW_COORDS(1,13)].original = '\\';
sensor_chars[MAP_DRAW_COORDS(1,14)].x = 11;
sensor_chars[MAP_DRAW_COORDS(1,14)].y = 7;
sensor_chars[MAP_DRAW_COORDS(1,14)].activated = '^';
sensor_chars[MAP_DRAW_COORDS(1,14)].original = '|';
sensor_chars[MAP_DRAW_COORDS(1,15)].x = 11;
sensor_chars[MAP_DRAW_COORDS(1,15)].y = 7;
sensor_chars[MAP_DRAW_COORDS(1,15)].activated = 'v';
sensor_chars[MAP_DRAW_COORDS(1,15)].original = '|';
sensor_chars[MAP_DRAW_COORDS(1,1)].x = 26;
sensor_chars[MAP_DRAW_COORDS(1,1)].y = 10;
sensor_chars[MAP_DRAW_COORDS(1,1)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(1,1)].original = '=';
sensor_chars[MAP_DRAW_COORDS(1,2)].x = 24;
sensor_chars[MAP_DRAW_COORDS(1,2)].y = 9;
sensor_chars[MAP_DRAW_COORDS(1,2)].activated = '^';
sensor_chars[MAP_DRAW_COORDS(1,2)].original = '/';
sensor_chars[MAP_DRAW_COORDS(1,3)].x = 24;
sensor_chars[MAP_DRAW_COORDS(1,3)].y = 9;
sensor_chars[MAP_DRAW_COORDS(1,3)].activated = 'v';
sensor_chars[MAP_DRAW_COORDS(1,3)].original = '/';
sensor_chars[MAP_DRAW_COORDS(1,6)].x = 1;
sensor_chars[MAP_DRAW_COORDS(1,6)].y = 12;
sensor_chars[MAP_DRAW_COORDS(1,6)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(1,6)].original = '=';
sensor_chars[MAP_DRAW_COORDS(1,7)].x = 1;
sensor_chars[MAP_DRAW_COORDS(1,7)].y = 12;
sensor_chars[MAP_DRAW_COORDS(1,7)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(1,7)].original = '=';
sensor_chars[MAP_DRAW_COORDS(1,8)].x = 1;
sensor_chars[MAP_DRAW_COORDS(1,8)].y = 14;
sensor_chars[MAP_DRAW_COORDS(1,8)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(1,8)].original = '=';
sensor_chars[MAP_DRAW_COORDS(2,0)].x = 28;
sensor_chars[MAP_DRAW_COORDS(2,0)].y = 7;
sensor_chars[MAP_DRAW_COORDS(2,0)].activated = 'v';
sensor_chars[MAP_DRAW_COORDS(2,0)].original = '/';
sensor_chars[MAP_DRAW_COORDS(2,9)].x = 19;
sensor_chars[MAP_DRAW_COORDS(2,9)].y = 10;
sensor_chars[MAP_DRAW_COORDS(2,9)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(2,9)].original = '=';
sensor_chars[MAP_DRAW_COORDS(2,10)].x = 29;
sensor_chars[MAP_DRAW_COORDS(2,10)].y = 2;
sensor_chars[MAP_DRAW_COORDS(2,10)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(2,10)].original = '=';
sensor_chars[MAP_DRAW_COORDS(2,11)].x = 29;
sensor_chars[MAP_DRAW_COORDS(2,11)].y = 2;
sensor_chars[MAP_DRAW_COORDS(2,11)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(2,11)].original = '=';
sensor_chars[MAP_DRAW_COORDS(2,12)].x = 18;
sensor_chars[MAP_DRAW_COORDS(2,12)].y = 0;
sensor_chars[MAP_DRAW_COORDS(2,12)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(2,12)].original = '=';
sensor_chars[MAP_DRAW_COORDS(2,13)].x = 18;
sensor_chars[MAP_DRAW_COORDS(2,13)].y = 0;
sensor_chars[MAP_DRAW_COORDS(2,13)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(2,13)].original = '=';
sensor_chars[MAP_DRAW_COORDS(2,14)].x = 24;
sensor_chars[MAP_DRAW_COORDS(2,14)].y = 12;
sensor_chars[MAP_DRAW_COORDS(2,14)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(2,14)].original = '=';
sensor_chars[MAP_DRAW_COORDS(2,15)].x = 24;
sensor_chars[MAP_DRAW_COORDS(2,15)].y = 12;
sensor_chars[MAP_DRAW_COORDS(2,15)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(2,15)].original = '=';
sensor_chars[MAP_DRAW_COORDS(2,1)].x = 28;
sensor_chars[MAP_DRAW_COORDS(2,1)].y = 7;
sensor_chars[MAP_DRAW_COORDS(2,1)].activated = '^';
sensor_chars[MAP_DRAW_COORDS(2,1)].original = '/';
sensor_chars[MAP_DRAW_COORDS(2,2)].x = 40;
sensor_chars[MAP_DRAW_COORDS(2,2)].y = 14;
sensor_chars[MAP_DRAW_COORDS(2,2)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(2,2)].original = '=';
sensor_chars[MAP_DRAW_COORDS(2,3)].x = 40;
sensor_chars[MAP_DRAW_COORDS(2,3)].y = 14;
sensor_chars[MAP_DRAW_COORDS(2,3)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(2,3)].original = '=';
sensor_chars[MAP_DRAW_COORDS(2,4)].x = 17;
sensor_chars[MAP_DRAW_COORDS(2,4)].y = 12;
sensor_chars[MAP_DRAW_COORDS(2,4)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(2,4)].original = '=';
sensor_chars[MAP_DRAW_COORDS(2,5)].x = 17;
sensor_chars[MAP_DRAW_COORDS(2,5)].y = 12;
sensor_chars[MAP_DRAW_COORDS(2,5)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(2,5)].original = '=';
sensor_chars[MAP_DRAW_COORDS(2,6)].x = 18;
sensor_chars[MAP_DRAW_COORDS(2,6)].y = 14;
sensor_chars[MAP_DRAW_COORDS(2,6)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(2,6)].original = '=';
sensor_chars[MAP_DRAW_COORDS(2,7)].x = 18;
sensor_chars[MAP_DRAW_COORDS(2,7)].y = 14;
sensor_chars[MAP_DRAW_COORDS(2,7)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(2,7)].original = '=';
sensor_chars[MAP_DRAW_COORDS(2,8)].x = 19;
sensor_chars[MAP_DRAW_COORDS(2,8)].y = 10;
sensor_chars[MAP_DRAW_COORDS(2,8)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(2,8)].original = '=';
sensor_chars[MAP_DRAW_COORDS(3,0)].x = 30;
sensor_chars[MAP_DRAW_COORDS(3,0)].y = 5;
sensor_chars[MAP_DRAW_COORDS(3,0)].activated = 'v';
sensor_chars[MAP_DRAW_COORDS(3,0)].original = '/';
sensor_chars[MAP_DRAW_COORDS(3,9)].x = 45;
sensor_chars[MAP_DRAW_COORDS(3,9)].y = 11;
sensor_chars[MAP_DRAW_COORDS(3,9)].activated = '^';
sensor_chars[MAP_DRAW_COORDS(3,9)].original = '/';
sensor_chars[MAP_DRAW_COORDS(3,10)].x = 35;
sensor_chars[MAP_DRAW_COORDS(3,10)].y = 12;
sensor_chars[MAP_DRAW_COORDS(3,10)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(3,10)].original = '=';
sensor_chars[MAP_DRAW_COORDS(3,11)].x = 35;
sensor_chars[MAP_DRAW_COORDS(3,11)].y = 12;
sensor_chars[MAP_DRAW_COORDS(3,11)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(3,11)].original = '=';
sensor_chars[MAP_DRAW_COORDS(3,12)].x = 32;
sensor_chars[MAP_DRAW_COORDS(3,12)].y = 10;
sensor_chars[MAP_DRAW_COORDS(3,12)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(3,12)].original = '=';
sensor_chars[MAP_DRAW_COORDS(3,13)].x = 32;
sensor_chars[MAP_DRAW_COORDS(3,13)].y = 10;
sensor_chars[MAP_DRAW_COORDS(3,13)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(3,13)].original = '=';
sensor_chars[MAP_DRAW_COORDS(3,14)].x = 34;
sensor_chars[MAP_DRAW_COORDS(3,14)].y = 9;
sensor_chars[MAP_DRAW_COORDS(3,14)].activated = '^';
sensor_chars[MAP_DRAW_COORDS(3,14)].original = '\\';
sensor_chars[MAP_DRAW_COORDS(3,15)].x = 34;
sensor_chars[MAP_DRAW_COORDS(3,15)].y = 9;
sensor_chars[MAP_DRAW_COORDS(3,15)].activated = 'v';
sensor_chars[MAP_DRAW_COORDS(3,15)].original = '\\';
sensor_chars[MAP_DRAW_COORDS(3,1)].x = 30;
sensor_chars[MAP_DRAW_COORDS(3,1)].y = 5;
sensor_chars[MAP_DRAW_COORDS(3,1)].activated = '^';
sensor_chars[MAP_DRAW_COORDS(3,1)].original = '/';
sensor_chars[MAP_DRAW_COORDS(3,2)].x = 32;
sensor_chars[MAP_DRAW_COORDS(3,2)].y = 2;
sensor_chars[MAP_DRAW_COORDS(3,2)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(3,2)].original = '=';
sensor_chars[MAP_DRAW_COORDS(3,3)].x = 32;
sensor_chars[MAP_DRAW_COORDS(3,3)].y = 2;
sensor_chars[MAP_DRAW_COORDS(3,3)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(3,3)].original = '=';
sensor_chars[MAP_DRAW_COORDS(3,4)].x = 46;
sensor_chars[MAP_DRAW_COORDS(3,4)].y = 4;
sensor_chars[MAP_DRAW_COORDS(3,4)].activated = '^';
sensor_chars[MAP_DRAW_COORDS(3,4)].original = '\\';
sensor_chars[MAP_DRAW_COORDS(1,4)].x = 26;
sensor_chars[MAP_DRAW_COORDS(1,4)].y = 2;
sensor_chars[MAP_DRAW_COORDS(1,4)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(1,4)].original = '=';
sensor_chars[MAP_DRAW_COORDS(3,5)].x = 46;
sensor_chars[MAP_DRAW_COORDS(3,5)].y = 4;
sensor_chars[MAP_DRAW_COORDS(3,5)].activated = 'v';
sensor_chars[MAP_DRAW_COORDS(3,5)].original = '\\';
sensor_chars[MAP_DRAW_COORDS(1,5)].x = 26;
sensor_chars[MAP_DRAW_COORDS(1,5)].y = 2;
sensor_chars[MAP_DRAW_COORDS(1,5)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(1,5)].original = '=';
sensor_chars[MAP_DRAW_COORDS(3,6)].x = 45;
sensor_chars[MAP_DRAW_COORDS(3,6)].y = 1;
sensor_chars[MAP_DRAW_COORDS(3,6)].activated = 'v';
sensor_chars[MAP_DRAW_COORDS(3,6)].original = '\\';
sensor_chars[MAP_DRAW_COORDS(3,7)].x = 45;
sensor_chars[MAP_DRAW_COORDS(3,7)].y = 1;
sensor_chars[MAP_DRAW_COORDS(3,7)].activated = '^';
sensor_chars[MAP_DRAW_COORDS(3,7)].original = '\\';
sensor_chars[MAP_DRAW_COORDS(3,8)].x = 45;
sensor_chars[MAP_DRAW_COORDS(3,8)].y = 11;
sensor_chars[MAP_DRAW_COORDS(3,8)].activated = 'v';
sensor_chars[MAP_DRAW_COORDS(3,8)].original = '/';
sensor_chars[MAP_DRAW_COORDS(4,0)].x = 28;
sensor_chars[MAP_DRAW_COORDS(4,0)].y = 5;
sensor_chars[MAP_DRAW_COORDS(4,0)].activated = 'v';
sensor_chars[MAP_DRAW_COORDS(4,0)].original = '\\';
sensor_chars[MAP_DRAW_COORDS(4,9)].x = 44;
sensor_chars[MAP_DRAW_COORDS(4,9)].y = 9;
sensor_chars[MAP_DRAW_COORDS(4,9)].activated = 'v';
sensor_chars[MAP_DRAW_COORDS(4,9)].original = '/';
sensor_chars[MAP_DRAW_COORDS(4,10)].x = 42;
sensor_chars[MAP_DRAW_COORDS(4,10)].y = 12;
sensor_chars[MAP_DRAW_COORDS(4,10)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(4,10)].original = '=';
sensor_chars[MAP_DRAW_COORDS(4,11)].x = 42;
sensor_chars[MAP_DRAW_COORDS(4,11)].y = 12;
sensor_chars[MAP_DRAW_COORDS(4,11)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(4,11)].original = '=';
sensor_chars[MAP_DRAW_COORDS(4,12)].x = 39;
sensor_chars[MAP_DRAW_COORDS(4,12)].y = 10;
sensor_chars[MAP_DRAW_COORDS(4,12)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(4,12)].original = '=';
sensor_chars[MAP_DRAW_COORDS(4,13)].x = 39;
sensor_chars[MAP_DRAW_COORDS(4,13)].y = 10;
sensor_chars[MAP_DRAW_COORDS(4,13)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(4,13)].original = '=';
sensor_chars[MAP_DRAW_COORDS(4,1)].x = 28;
sensor_chars[MAP_DRAW_COORDS(4,1)].y = 5;
sensor_chars[MAP_DRAW_COORDS(4,1)].activated = '^';
sensor_chars[MAP_DRAW_COORDS(4,1)].original = '\\';
sensor_chars[MAP_DRAW_COORDS(4,4)].x = 39;
sensor_chars[MAP_DRAW_COORDS(4,4)].y = 2;
sensor_chars[MAP_DRAW_COORDS(4,4)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(4,4)].original = '=';
sensor_chars[MAP_DRAW_COORDS(4,5)].x = 39;
sensor_chars[MAP_DRAW_COORDS(4,5)].y = 2;
sensor_chars[MAP_DRAW_COORDS(4,5)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(4,5)].original = '=';
sensor_chars[MAP_DRAW_COORDS(4,6)].x = 40;
sensor_chars[MAP_DRAW_COORDS(4,6)].y = 0;
sensor_chars[MAP_DRAW_COORDS(4,6)].activated = '>';
sensor_chars[MAP_DRAW_COORDS(4,6)].original = '=';
sensor_chars[MAP_DRAW_COORDS(4,7)].x = 40;
sensor_chars[MAP_DRAW_COORDS(4,7)].y = 0;
sensor_chars[MAP_DRAW_COORDS(4,7)].activated = '<';
sensor_chars[MAP_DRAW_COORDS(4,7)].original = '=';
sensor_chars[MAP_DRAW_COORDS(4,8)].x = 44;
sensor_chars[MAP_DRAW_COORDS(4,8)].y = 9;
sensor_chars[MAP_DRAW_COORDS(4,8)].activated = 'v';
sensor_chars[MAP_DRAW_COORDS(4,8)].original = '/';
}
