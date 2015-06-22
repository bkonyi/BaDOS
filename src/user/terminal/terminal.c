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
#define TERM_INPUT_ROW               TERM_SENSORS_ROW + 18
#define TERM_INPUT_COORDS            10, TERM_INPUT_ROW
#define TERM_STATUS_ROW              (TERM_INPUT_ROW-2)
#define TERM_STATUS_COORDS           11, TERM_STATUS_ROW
#define TERM_TRACK_SELECTION_COORDS  10, TERM_STATUS_ROW - 2
#define TERM_IDLE_PERCENTAGE_COORDS  25, TERM_STATUS_ROW - 2

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
static void handle_set_track(sensor_map_chars_t* sensor_display_info, char track);
static void clear_user_input(void);
static void status_message(char* fmt, ...);
static void clear_track_map(void);
static void draw_initial_track_map(char track_map[TRACK_SIZE_Y][TRACK_SIZE_X]);
static void update_map_sensor(sensor_map_chars_t* sensor_chars,int32_t group, int32_t index,bool state);


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
    printf(COM2, "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\r\n");
    printf(COM2, "┃                                   \e[32;1;4mBADos\e[0m                                 ┃                TRAIN                 ┃\r\n");
    printf(COM2, "┃                  \e[2mCreated by: Dan Chevalier and Ben Konyi\e[0m                ┃             INFORMATION              ┃\r\n");
    printf(COM2, "┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━┳━━━━━━━┳━━━━━━━━━━┳━━━━━━━━━━┫\r\n");         
    printf(COM2, "┃        \e[1;96mSENSOR STATES\e[0m           ┃  \e[1;91mRECENT\e[0m  ┃        \e[1;35mSWITCH STATES\e[0m        ┃ NUMBER ┃ SPEED ┃ LANDMARK ┃ DISTANCE ┃\r\n");
    printf(COM2, "┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━┻━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━┫\r\n");
    printf(COM2, "┃A1  A2  A3  A4  A5  A6  A7  A8  ┃          ┃  1  2  3  4  5  6  7  8  9  ┃                                      ┃\r\n");
    printf(COM2, "┃A9  A10 A11 A12 A13 A14 A15 A16 ┃          ┃  ?  ?  ?  ?  ?  ?  ?  ?  ?  ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\r\n");
    printf(COM2, "┃B1  B2  B3  B4  B5  B6  B7  B8  ┃          ┃                             ┃                                      ┃\r\n");
    printf(COM2, "┃B9  B10 B11 B12 B13 B14 B15 B16 ┃          ┃  10 11 12 13 14 15 16 17 18 ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\r\n");
    printf(COM2, "┃C1  C2  C3  C4  C5  C6  C7  C8  ┃          ┃  ?  ?  ?  ?  ?  ?  ?  ?  ?  ┃                                      ┃\r\n");
    printf(COM2, "┃C9  C10 C11 C12 C13 C14 C15 C16 ┃          ┃                             ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\r\n");
    printf(COM2, "┃D1  D2  D3  D4  D5  D6  D7  D8  ┃          ┃    153   154   155   156    ┃                                      ┃\r\n");
    printf(COM2, "┃D9  D10 D11 D12 D13 D14 D15 D16 ┃          ┃     ?     ?     ?     ?     ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\r\n");
    printf(COM2, "┃E1  E2  E3  E4  E5  E6  E7  E8  ┃          ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫                                      ┃\r\n");
    printf(COM2, "┃E9  E10 E11 E12 E13 E14 E15 E16 ┃          ┃ TIME:                       ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\r\n");
    printf(COM2, "┣━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━┳━┻━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫                                      ┃\r\n");
    printf(COM2, "┃ Track: ? ┃ Idle Time: XX.X%%  ┃                                          ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\r\n");   
    printf(COM2, "┣━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\r\n");
    printf(COM2, "┃ \e[33;1mResult:\e[0m                                                                 ┃\r\n");
    printf(COM2, "┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\r\n");
    printf(COM2, "┃ \e[32;1mInput:\e[0m                                                                  ┃\r\n");
    printf(COM2, "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\r\n");
    term_fmt_clr();

    Create(TERMINAL_TICK_NOTIFIER, terminal_tick_notifier);

    //draw_initial_track_map();
    sensor_map_chars_t sensor_chars[16*16];
    //track_b_sensor_char_init(sensor_chars);
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
                handle_update_sensors(sensor_chars, previous_sensors, data.sensors, recent_sensors, &recent_sensors_index);
                break;
            case TERMINAL_SET_TRACK:
                handle_set_track(sensor_chars, data.byte1);
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

    if((ticks / 10) % 10 == 0) {
        int idle_percentage = GetIdleTime();
        term_move_cursor(TERM_IDLE_PERCENTAGE_COORDS);
        printf(COM2, "%d.%d", idle_percentage / 100, idle_percentage % 100);
    }

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
                    }
                    
                    //Update the sensor label
                    printf(COM2, "%c%d", 'A' + group, index);
                    update_map_sensor(sensor_chars, group, index-1, (new_sensor_data & 0x1));
                    
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

void handle_set_track(sensor_map_chars_t* sensor_display_info, char track) {
    //TODO actually set track info

    clear_track_map();

    if(track == 'A') {
        char track_display[TRACK_SIZE_Y][TRACK_SIZE_X] = TRACKA_STR_ARRAY;

        track_a_sensor_char_init(sensor_display_info);
        status_message("Setting active track to track A");
        draw_initial_track_map(track_display);
    } else if(track == 'B') {
        char track_display[TRACK_SIZE_Y][TRACK_SIZE_X] = TRACKB_STR_ARRAY;

        track_b_sensor_char_init(sensor_display_info);
        status_message("Setting active track to track B");
        draw_initial_track_map(track_display);
    } else {
        //This shouldn't happen
        ASSERT(0);
    }

    term_save_cursor();
    term_hide_cursor();

    term_move_cursor(TERM_TRACK_SELECTION_COORDS);

    printf(COM2, "%c", track);
    term_restore_cursor();
    term_show_cursor();
}

void clear_track_map(void) {
    term_save_cursor();
    term_hide_cursor();
    term_move_cursor(MAP_COORDS);

    int i, j;
    for(i = 0; i < TRACK_SIZE_Y; ++i) {
        for(j = 0; j < TRACK_SIZE_X; ++j) {
            putc(COM2, ' ');
        }
        printf(COM2, "\r\n");
    }

    term_show_cursor();
    term_restore_cursor();
}

void draw_initial_track_map(char track_map[TRACK_SIZE_Y][TRACK_SIZE_X]) {
    int i = 0;

    term_save_cursor();
    term_hide_cursor();
    term_move_cursor(MAP_COORDS);
    for(i = 0; i < TRACK_SIZE_Y; i++) {
        term_move_cursor(MAP_COL,MAP_ROW+i);
        printf(COM2,"%s",track_map[i]);
    }


    term_restore_cursor();
    term_show_cursor();
}

void update_map_sensor(sensor_map_chars_t* sensor_chars,int32_t group, int32_t index,bool state) {
     sensor_map_chars_t* data = &sensor_chars[MAP_DRAW_COORDS(group,index)];
     term_move_cursor(MAP_COL + data->x,MAP_ROW + data->y);
     if(state == true) {
        putc(COM2,data->activated);
     } else if (state == false) {
        putc(COM2,data->original);
     }
}

