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
#include <track/track_node.h>
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

#define TERM_TRAIN_STATE_START_ROW   TERM_SWITCHES_DATA_ROW - 1
#define TERM_TRAIN_STATE_START_COL   76
#define TERM_TRAIN_STATE_TRAIN_OFF   3
#define TERM_TRAIN_STATE_SPEED_OFF   TERM_TRAIN_STATE_TRAIN_OFF + 8
#define TERM_TRAIN_STATE_LANDM_OFF   TERM_TRAIN_STATE_SPEED_OFF + 10
#define TERM_TRAIN_STATE_DIST_OFF    TERM_TRAIN_STATE_LANDM_OFF + 7
#define TERM_TRAIN_STATE_NEXT_OFF    TERM_TRAIN_STATE_DIST_OFF  + 13
#define TERM_TRAIN_STATE_VELO_OFF    TERM_TRAIN_STATE_NEXT_OFF + 6
#define TERM_TRAIN_STATE_ERR_OFF     TERM_TRAIN_STATE_VELO_OFF + 10

#define MAP_ROW                      TERM_INPUT_ROW+2
#define MAP_COL                      3
#define MAP_COORDS                   MAP_COL,MAP_ROW

#define NUM_RECENT_SENSORS           10
#define RECENT_SENSORS_DATA_ROW      TERM_SENSORS_DATA_ROW 
#define RECENT_SENSORS_DATA_COLUMN   38

CREATE_NON_POINTER_BUFFER_TYPE(recent_sensors_buffer_t, int, NUM_RECENT_SENSORS);


static void handle_update_terminal_clock(int32_t ticks);
static void handle_update_sensors(bool map_initialized, sensor_map_chars_t* sensor_chars, char* previous_sensors, char* sensors, int* recent_sensors, int* recent_sensors_index);
static void terminal_tick_notifier(void);
static void handle_train_command(int32_t num,int32_t speed);
static void handle_reverse_command(int32_t num);
static void handle_switch_command(int32_t num,char state);
static void handle_quit_command(void);
static void handle_start_command(void);
static void handle_stop_command(void);
static void handle_set_track(sensor_map_chars_t* sensor_display_info, char track);
static void handle_register_train(int8_t train, int8_t slot);
static void handle_init_train_slot(int8_t train, int8_t slot);
static void handle_update_train_slot_speed(int8_t slot, int8_t speed);
static void handle_update_train_slot_current_location(int8_t slot, int16_t sensor_position);
static void handle_update_train_slot_next_location(int8_t slot, int16_t sensor_position);
static void handle_clear_train_slot(int8_t slot);
static void handle_update_train_slot_velocity(int8_t slot, uint32_t v) ;
static void _status_message(bool,char* fmt, ...);
static void clear_track_map(void);
static void draw_initial_track_map(char track_map[TRACK_SIZE_Y][TRACK_SIZE_X]);
static void update_map_sensor(sensor_map_chars_t* sensor_chars,int32_t group, int32_t index,bool state);
static void handle_find_command(void);
static void handle_terminal_send_2_ints(uint32_t command, uint32_t num1, uint32_t num2);
static void handle_update_train_slot_distance(int8_t slot, int32_t dist);
static void handle_update_train_slot_error(int8_t slot, int32_t err);
static void handle_command_success_message(char *cmd) ;
static void _clear_user_input(void) ;

static void handle_initialize_track_switches(void);

#define MAX_RECEIVE_LENGTH (sizeof(terminal_data_t)+100)

void terminal_server(void) {
    int sending_tid;
    int result;
    int buffer_char_count =0;   //keep track of how many chars are in the user in buff
    char input_buffer[MAX_RECEIVE_LENGTH];
    terminal_data_t* data = (terminal_data_t*)input_buffer;

    char previous_sensors[10];
    int recent_sensors[NUM_RECENT_SENSORS];
    int recent_sensors_index = 0;
    bool map_initialized = false;

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
    printf(COM2, "┏━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┓\r\n");
    printf(COM2, "┃                                   \e[32;1;4mBADos\e[0m                                 ┃                    TRAIN                                          ┃\r\n");
    printf(COM2, "┃                  \e[2mCreated by: Dan Chevalier and Ben Konyi\e[0m                ┃                 INFORMATION                                       ┃\r\n");
    printf(COM2, "┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┳━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━┳━━━━━━━┳━━━━━━━━━━┳━━━━━━━━━━┳━━━━━━┳━━━━━━━━━┳━━━━━━━━━━━┫\r\n");         
    printf(COM2, "┃        \e[1;96mSENSOR STATES\e[0m           ┃  \e[1;91mRECENT\e[0m  ┃        \e[1;35mSWITCH STATES\e[0m        ┃ NUMBER ┃ SPEED ┃ LANDMARK ┃ DISTANCE ┃ NEXT ┃ V(cm/s) ┃ ERROR(cm) ┃\r\n");
    printf(COM2, "┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━━━╋━━━━━━━━━━━━━━━━━━━━━━━━━━━━━╋━━━━━━━━┻━━━━━━━┻━━━━━━━━━━┻━━━━━━━━━━┻━━━━━━┻━━━━━━━━━┻━━━━━━━━━━━┫\r\n");
    printf(COM2, "┃A1  A2  A3  A4  A5  A6  A7  A8  ┃          ┃  1  2  3  4  5  6  7  8  9  ┃                                                                   ┃\r\n");
    printf(COM2, "┃A9  A10 A11 A12 A13 A14 A15 A16 ┃          ┃  ?  ?  ?  ?  ?  ?  ?  ?  ?  ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\r\n");
    printf(COM2, "┃B1  B2  B3  B4  B5  B6  B7  B8  ┃          ┃                             ┃                                                                   ┃\r\n");
    printf(COM2, "┃B9  B10 B11 B12 B13 B14 B15 B16 ┃          ┃  10 11 12 13 14 15 16 17 18 ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\r\n");
    printf(COM2, "┃C1  C2  C3  C4  C5  C6  C7  C8  ┃          ┃  ?  ?  ?  ?  ?  ?  ?  ?  ?  ┃                                                                   ┃\r\n");
    printf(COM2, "┃C9  C10 C11 C12 C13 C14 C15 C16 ┃          ┃                             ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\r\n");
    printf(COM2, "┃D1  D2  D3  D4  D5  D6  D7  D8  ┃          ┃    153   154   155   156    ┃                                                                   ┃\r\n");
    printf(COM2, "┃D9  D10 D11 D12 D13 D14 D15 D16 ┃          ┃     ?     ?     ?     ?     ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\r\n");
    printf(COM2, "┃E1  E2  E3  E4  E5  E6  E7  E8  ┃          ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫                                                                   ┃\r\n");
    printf(COM2, "┃E9  E10 E11 E12 E13 E14 E15 E16 ┃          ┃ TIME:                       ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\r\n");
    printf(COM2, "┣━━━━━━━━━━┳━━━━━━━━━━━━━━━━━━━┳━┻━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫                                                                   ┃\r\n");
    printf(COM2, "┃ Track: ? ┃ Idle Time: XX.X%%  ┃                                          ┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\r\n");   
    printf(COM2, "┣━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━┻━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\r\n");
    printf(COM2, "┃ \e[33;1mResult:\e[0m                                                                 ┃\r\n");
    printf(COM2, "┣━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┫\r\n");
    printf(COM2, "┃ \e[32;1mInput:\e[0m                                                                  ┃\r\n");
    printf(COM2, "┗━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━┛\r\n");

    CreateName(TERMINAL_TICK_NOTIFIER_PRIORITY, terminal_tick_notifier, TERMINAL_TICK_NOTIFIER);

    //draw_initial_track_map();
    sensor_map_chars_t sensor_chars[16*16];
    //track_b_sensor_char_init(sensor_chars);
    //IMPORTANT: This needs to be the last coord we set so user input is in the right place
    term_move_cursor(TERM_INPUT_COORDS);

    FOREVER {
        result = Receive(&sending_tid, input_buffer, MAX_RECEIVE_LENGTH);
        ASSERT(result >= 0);

        //Just send a null reply for now
        Reply(sending_tid, (char*)NULL, 0);

        switch(data->command) {
            case TERMINAL_UPDATE_CLOCK:
                handle_update_terminal_clock(data->num1);
                break;
            case TERMINAL_ECHO_INPUT:
                if(data->byte1 != CARRIAGE_RETURN  ) {
                    if(data->byte1 == BACKSPACE){
                        if(buffer_char_count>0){
                            buffer_char_count--; 
                            printf(COM2,"%c %c",BACKSPACE,BACKSPACE);
                        }//otherwise we don't need to do anything
                        continue;
                    }else{
                        buffer_char_count++;
                        putc(COM2, data->byte1);
                    }
                }else{
                    buffer_char_count=0;
                }
                break;
            case TERMINAL_TRAIN_COMMAND:
                handle_train_command(data->num1,data->num2);
                break;
            case TERMINAL_SWITCH_COMMAND:
                handle_switch_command(data->num1,data->byte1);
                break;
            case TERMINAL_REVERSE_COMMAND:
                handle_reverse_command(data->num1);
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
                handle_update_sensors(map_initialized, sensor_chars, previous_sensors, data->sensors, recent_sensors, &recent_sensors_index);
                break;
            case TERMINAL_SET_TRACK:
                map_initialized = true;
                handle_set_track(sensor_chars, data->byte1);
                break;
            case TERMINAL_REGISTER_TRAIN:
                handle_register_train(data->num1, data->num2);
                break;
            case TERMINAL_INIT_TRAIN_SLOT:
                handle_init_train_slot(data->num1, data->num2);
                break;
            case TERMINAL_UPDATE_TRAIN_SLOT_SPEED:
                handle_update_train_slot_speed(data->num2, data->byte1);
                break;
            case TERMINAL_UPDATE_TRAIN_SLOT_CURRENT_LOCATION:
                handle_update_train_slot_current_location(data->num2, data->num1);
                break;
            case TERMINAL_UPDATE_TRAIN_SLOT_NEXT_LOCATION:
                handle_update_train_slot_next_location(data->num2, data->num1);
                break;
            case TERMINAL_CLEAR_TRAIN_SLOT:
                handle_clear_train_slot(data->num2);
                break;
            case TERMINAL_FIND_TRAIN:
                handle_find_command();
                break;
            case TERMINAL_INIT_ALL_SWITCHES:
                handle_initialize_track_switches();
                break;
            case TERMINAL_UPDATE_TRAIN_VELO:
                handle_update_train_slot_velocity(data->num1, data->num2);
                break;
            case TERMINAL_UPDATE_TRAIN_DIST:
                handle_update_train_slot_distance(data->num1, data->num2);
                break;
            case TERMINAL_UPDATE_TRAIN_ERROR:
                handle_update_train_slot_error(data->num1, data->num2);
                break;
            case TERMINAL_COMMAND_HEAVY_MESSAGE:
                //Null terminate it just in case
                input_buffer[MAX_RECEIVE_LENGTH-1] = '\0';
                //place our error message
                _status_message(((bool)data->num1),((char*)input_buffer)+(sizeof(terminal_data_t)));
                break;
            case TERMINAL_COMMAND_SUCCESS:
                //Null terminate it just in case
                input_buffer[MAX_RECEIVE_LENGTH-1] = '\0';
                //place our error message
                handle_command_success_message(((char*)input_buffer)+(sizeof(terminal_data_t)));
                break;
            default:
                ASSERT(0);
                break;
        }
    }
}

void handle_terminal_send_2_ints(uint32_t command, uint32_t num1, uint32_t num2) {
    terminal_data_t terminal_data;
    terminal_data.command = command;
    terminal_data.num1 = num1;
    terminal_data.num2 = num2;
    Send(TERMINAL_SERVER_ID,(char*)&terminal_data,sizeof(terminal_data_t),(char*)NULL,0);
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

void initialize_terminal_train_slot(int8_t train, int8_t slot) {
    terminal_data_t request;
    request.command = TERMINAL_INIT_TRAIN_SLOT;
    request.num1 = train;
    request.num2 = slot;

    Send(TERMINAL_SERVER_ID, (char*)&request, sizeof(terminal_data_t), (char*)NULL, 0);
}

void update_terminal_train_slot_speed(int8_t train, int8_t slot, int8_t speed) {
    terminal_data_t request;
    request.command = TERMINAL_UPDATE_TRAIN_SLOT_SPEED;
    request.num1 = train;
    request.num2 = slot;
    request.byte1 = speed;

    Send(TERMINAL_SERVER_ID, (char*)&request, sizeof(terminal_data_t), (char*)NULL, 0);
}

void update_terminal_train_slot_current_location(int8_t train, int8_t slot, int16_t sensor_location) {
    terminal_data_t request;
    request.command = TERMINAL_UPDATE_TRAIN_SLOT_CURRENT_LOCATION;
    request.num1 = sensor_location;
    request.num2 = slot;

    Send(TERMINAL_SERVER_ID, (char*)&request, sizeof(terminal_data_t), (char*)NULL, 0);
}

void update_terminal_train_slot_next_location(int8_t train, int8_t slot, int16_t sensor_location) {
    terminal_data_t request;
    request.command = TERMINAL_UPDATE_TRAIN_SLOT_NEXT_LOCATION;
    request.num1 = sensor_location;
    request.num2 = slot;

    Send(TERMINAL_SERVER_ID, (char*)&request, sizeof(terminal_data_t), (char*)NULL, 0);
}

void clear_terminal_train_slot(int8_t slot) {
    terminal_data_t request;
    request.command = TERMINAL_CLEAR_TRAIN_SLOT;
    request.num2 = slot;

    Send(TERMINAL_SERVER_ID, (char*)&request, sizeof(terminal_data_t), (char*)NULL, 0);
}
void send_term_update_velocity_msg (uint32_t slot, uint32_t v) {
    handle_terminal_send_2_ints(TERMINAL_UPDATE_TRAIN_VELO, slot, v);
}
void send_term_update_dist_msg (uint32_t slot, int32_t dist)  {
    handle_terminal_send_2_ints(TERMINAL_UPDATE_TRAIN_DIST, slot, dist);
}

void send_term_update_err_msg(uint32_t slot, int32_t dist) {
    handle_terminal_send_2_ints(TERMINAL_UPDATE_TRAIN_ERROR, slot, dist);
}

void handle_update_train_slot_distance(int8_t slot, int32_t dist) {
    term_save_cursor();
    term_move_cursor(TERM_TRAIN_STATE_START_COL + TERM_TRAIN_STATE_DIST_OFF, TERM_TRAIN_STATE_START_ROW + (2 * (slot - 1)));
    //We expect the distance in mm
    printf(COM2, "          ");
    term_move_cursor(TERM_TRAIN_STATE_START_COL + TERM_TRAIN_STATE_DIST_OFF, TERM_TRAIN_STATE_START_ROW + (2 * (slot - 1)));

    int32_t decimal = dist % 10;
    int32_t distance = dist / 10;

    if(decimal < 0) {
        decimal *= -1;
    }

    //We expect the distance in mm
    printf(COM2, "%d.%d", distance, decimal);
    term_restore_cursor();
}

void handle_update_train_slot_velocity(int8_t slot, uint32_t v) {
    term_save_cursor();
    term_move_cursor(TERM_TRAIN_STATE_START_COL + TERM_TRAIN_STATE_VELO_OFF, TERM_TRAIN_STATE_START_ROW + (2 * (slot - 1)));
    //we received our time in mm/s
    printf(COM2, "%d.%d ", v/10,v%10);
    term_restore_cursor();
}

void handle_update_train_slot_error(int8_t slot, int32_t err) {
    term_save_cursor();
    term_move_cursor(TERM_TRAIN_STATE_START_COL + TERM_TRAIN_STATE_ERR_OFF, TERM_TRAIN_STATE_START_ROW + (2 * (slot - 1)));
    //We expect the distance in mm
    printf(COM2, "       ");
    term_move_cursor(TERM_TRAIN_STATE_START_COL + TERM_TRAIN_STATE_ERR_OFF, TERM_TRAIN_STATE_START_ROW + (2 * (slot - 1)));
    //We expect the distance in mm
    int32_t decimal = err % 10;

    if(decimal < 0) {
        decimal *= -1;
    }

    printf(COM2, "%d.%d", err / 10, decimal);
    term_restore_cursor();
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
        printf(COM2, "%d.%d%%", idle_percentage / 100, idle_percentage % 100);
    }

    term_restore_cursor();
    term_show_cursor();
}

void handle_update_sensors(bool map_initialized, sensor_map_chars_t* sensor_chars, char* previous_sensors, char* sensors, int* recent_sensors, int* recent_sensors_index) {
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
                    ASSERT(2 + (7 - j) * TERM_SENSORS_DATA_WIDTH >= 0);
                    ASSERT(TERM_SENSORS_DATA_ROW + i >= 0);
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
                    if(map_initialized) {
                        update_map_sensor(sensor_chars, group, index-1, (new_sensor_data & 0x1));
                    }
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
void _status_message(bool clr_usr_in,char* fmt, ...){
    term_hide_cursor();
    term_move_cursor(TERM_STATUS_COORDS);
    term_clear_rest_line();
    va_list va;
    va_start(va,fmt);
    vprintf(COM2,fmt,va);
    va_end(va);
    term_move_cursor(RIGHT_BAR_COL,TERM_STATUS_ROW);
    printf(COM2,"┃");
    if(clr_usr_in == true) {
        _clear_user_input();
    }
}
void handle_command_success_message(char *cmd) {
    term_hide_cursor();

    term_move_cursor(TERM_STATUS_COORDS);
    term_clear_rest_line();
    printf(COM2,"Command Succeeed: '%s'",cmd);
    _clear_user_input();
}
void _clear_user_input(void) {

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
    _status_message(true,"CMD: 'TR' #: '%d' Speed: '%d'",num,speed);
}
void send_term_train_msg(int32_t num,int32_t speed) {
    terminal_data_t terminal_data;
    terminal_data.command = TERMINAL_TRAIN_COMMAND;
    terminal_data.num1 = num;
    terminal_data.num2 = speed;
    Send(TERMINAL_SERVER_ID,(char*)&terminal_data,sizeof(terminal_data_t),(char*)NULL,0);
}
void handle_reverse_command(int32_t num){
   _status_message(true, "CMD: 'RV' #: '%d'",num);
}
void send_term_heavy_msg(bool clr_user_input,char*message,...) {
    char err_msg[128];

    va_list va;
    va_start(va,message);
    vsprintf(err_msg,message,va);
    va_end(va);
    
    int input_len = strlen(err_msg)+1; // include nullus terminus (roman for null terminator....)
    int msg_len = sizeof(terminal_data_t) + input_len*sizeof(char);
    char msg_to_send[msg_len];

    terminal_data_t* terminal_data = (terminal_data_t*)msg_to_send;
    terminal_data->command = TERMINAL_COMMAND_HEAVY_MESSAGE;
    terminal_data->num1 = clr_user_input;
    strcpy(msg_to_send + sizeof(terminal_data_t),err_msg);
    Send(TERMINAL_SERVER_ID,msg_to_send,msg_len, NULL,0);
}
void send_term_cmd_success_msg(char*cmd) {
    
    int input_len = strlen(cmd)+1; // include nullus terminus (rRoman for null terminator....)
    int msg_len = sizeof(terminal_data_t) + input_len*sizeof(char);
    char msg_to_send[msg_len];

    terminal_data_t* terminal_data = (terminal_data_t*)msg_to_send;
    terminal_data->command = TERMINAL_COMMAND_SUCCESS;
    strcpy(msg_to_send + sizeof(terminal_data_t),cmd);
    Send(TERMINAL_SERVER_ID,msg_to_send,msg_len, NULL,0);
}
void send_term_reverse_msg(uint32_t train_num) {
    terminal_data_t terminal_data;
    terminal_data.command = TERMINAL_REVERSE_COMMAND;
    terminal_data.num1 = train_num;
    Send(TERMINAL_SERVER_ID,(char*)&terminal_data,sizeof(terminal_data_t),(char*)NULL,0);
}
void handle_switch_command(int32_t num,char state){
    _status_message(true, "CMD 'SW' #: '%d' State: '%c'",num,state);

    term_hide_cursor();
    term_save_cursor();
    num -= 1;

    //We've got 4 switches with weird assignments, so we need to have a special case for them
    if(num <= 17) {
        term_move_cursor(TERM_SWITCHES_DATA_COLUMN + TERM_SWITCHES_COL_WIDTH * (num % 9), TERM_SWITCHES_DATA_ROW + TERM_SWITCHES_ROW_HEIGHT * (num / 9));
    }else if (146-1 <= num && num <=148-1) {
        //TODO: include these?
    } else if(0x99-1 <= num && num <=0x9c-1){
        num -= 152;
        term_move_cursor((TERM_SWITCHES_DATA_COLUMN + 3) + TERM_SWITCHES_BIG_COL_WIDTH * num, TERM_SWITCHES_DATA_ROW + TERM_SWITCHES_ROW_HEIGHT * 2);
    }

    printf(COM2, "%c", char_to_upper(state));

    term_restore_cursor();
    term_show_cursor();
}
void send_term_switch_msg(int32_t train_num,char state) {
    terminal_data_t terminal_data;
    terminal_data.command = TERMINAL_SWITCH_COMMAND;
    terminal_data.num1 = train_num;
    terminal_data.byte1 = state;
    Send(TERMINAL_SERVER_ID,(char*)&terminal_data,sizeof(terminal_data_t),(char*)NULL,0);
}
void send_term_initialize_track_switches(void) {
    terminal_data_t terminal_data;
    terminal_data.command = TERMINAL_INIT_ALL_SWITCHES;

    Send(TERMINAL_SERVER_ID,(char*)&terminal_data,sizeof(terminal_data_t),(char*)NULL,0);
}
void handle_initialize_track_switches(void) {
    int i;
    for(i = 0; i < 200; i ++){
        if(is_valid_switch_number(i)){
            handle_switch_command(i,'C');
        }
    }
}
void handle_quit_command(void){
    _status_message(true,"CMD QUIT");
    Terminate();
}
void send_term_quit_msg (void) {
    terminal_data_t terminal_data;
    terminal_data.command = TERMINAL_QUIT;
    Send(TERMINAL_SERVER_ID,(char*)&terminal_data,sizeof(terminal_data_t),(char*)NULL,0);
}

void handle_start_command(void) {
    _status_message(false,"ENABLING TRAIN CONTROLLER");
}
void send_term_start_msg(void) {
    terminal_data_t terminal_data;
    terminal_data.command = TERMINAL_START_CTRL;
    Send(TERMINAL_SERVER_ID,(char*)&terminal_data,sizeof(terminal_data_t),(char*)NULL,0);
}

void handle_stop_command(void) {
    _status_message(false,"STOPPING TRAINS AND TURNING OFF CONTROLLER");
}
void send_term_stop_msg(void) {
    terminal_data_t terminal_data;
    terminal_data.command = TERMINAL_STOP_CTRL;
    Send(TERMINAL_SERVER_ID,(char*)&terminal_data,sizeof(terminal_data_t),(char*)NULL,0);
}
void handle_find_command(void) {
    _status_message(false,"FINDING REGISTERED TRAINS...");
}
void send_term_find_msg(void) {
    terminal_data_t terminal_data;
    terminal_data.command = TERMINAL_FIND_TRAIN;
    Send(TERMINAL_SERVER_ID,(char*)&terminal_data,sizeof(terminal_data_t),(char*)NULL,0);
}

void handle_set_track(sensor_map_chars_t* sensor_display_info, char track) {
    //TODO actually set track info
    clear_track_map();

    if(track == 'A') {
        char track_display[TRACK_SIZE_Y][TRACK_SIZE_X] = TRACKA_STR_ARRAY;

        track_a_sensor_char_init(sensor_display_info);
        _status_message(true,"Setting active track to track A");
        draw_initial_track_map(track_display);
    } else if(track == 'B') {
        char track_display[TRACK_SIZE_Y][TRACK_SIZE_X] = TRACKB_STR_ARRAY;

        track_b_sensor_char_init(sensor_display_info);
        _status_message(true,"Setting active track to track B");
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

void send_term_set_track_msg(char track) {
    terminal_data_t terminal_data;
    terminal_data.command = TERMINAL_SET_TRACK;
    terminal_data.byte1 = track;
    Send(TERMINAL_SERVER_ID,(char*)&terminal_data,sizeof(terminal_data_t),(char*)NULL,0);
}

void handle_register_train(int8_t train, int8_t slot) {
    _status_message(true,"TRAIN: %d REGISTERED TO SLOT: %d", train, slot);
}
void send_term_register_train_msg(int8_t train, int8_t slot) {
    terminal_data_t terminal_data;
    terminal_data.command = TERMINAL_REGISTER_TRAIN;
    terminal_data.num1 = train;
    terminal_data.num2 = slot;
    Send(TERMINAL_SERVER_ID,(char*)&terminal_data,sizeof(terminal_data_t),(char*)NULL,0);
}

void handle_init_train_slot(int8_t train, int8_t slot) {
    int slot_offset = TERM_TRAIN_STATE_START_ROW + (2 * (slot - 1));

    term_save_cursor();
    term_move_cursor(TERM_TRAIN_STATE_START_COL + TERM_TRAIN_STATE_TRAIN_OFF, slot_offset);
    printf(COM2, "%d ", train);
    term_move_cursor(TERM_TRAIN_STATE_START_COL + TERM_TRAIN_STATE_SPEED_OFF, slot_offset);
    printf(COM2, "??");
    term_move_cursor(TERM_TRAIN_STATE_START_COL + TERM_TRAIN_STATE_LANDM_OFF, slot_offset);
    printf(COM2, "??");
    term_move_cursor(TERM_TRAIN_STATE_START_COL + TERM_TRAIN_STATE_DIST_OFF, slot_offset);
    printf(COM2, "?");
    term_move_cursor(TERM_TRAIN_STATE_START_COL + TERM_TRAIN_STATE_NEXT_OFF, slot_offset);
    printf(COM2, "??");
    term_restore_cursor();
}


void handle_update_train_slot_speed(int8_t slot, int8_t speed) {
    term_save_cursor();
    term_move_cursor(TERM_TRAIN_STATE_START_COL + TERM_TRAIN_STATE_SPEED_OFF, TERM_TRAIN_STATE_START_ROW + (2 * (slot - 1)));
    printf(COM2, "%d ", speed);
    term_restore_cursor();
}


void handle_update_train_slot_current_location(int8_t slot, int16_t sensor_position) {
    term_save_cursor();
    term_move_cursor(TERM_TRAIN_STATE_START_COL + TERM_TRAIN_STATE_LANDM_OFF, TERM_TRAIN_STATE_START_ROW + (2 * (slot - 1)));
    if(sensor_position != -1) {
        char sensor_letter = sensor_id_to_letter(sensor_position);
        int sensor_number = sensor_id_to_number(sensor_position);

        printf(COM2, "%c%d ", sensor_letter, sensor_number);
    } else {
        printf(COM2, "--");
    }
    term_restore_cursor();
}

void handle_update_train_slot_next_location(int8_t slot, int16_t sensor_position) {
    term_save_cursor();
    term_move_cursor(TERM_TRAIN_STATE_START_COL + TERM_TRAIN_STATE_NEXT_OFF, TERM_TRAIN_STATE_START_ROW + (2 * (slot - 1)));
    if(sensor_position != -1) {
        char sensor_letter = sensor_id_to_letter(sensor_position);
        int sensor_number = sensor_id_to_number(sensor_position);

        printf(COM2, "%c%d ", sensor_letter, sensor_number);
    } else {
        printf(COM2, "--");
    }
    term_restore_cursor();
}

void handle_clear_train_slot(int8_t slot) {
    int slot_offset = TERM_TRAIN_STATE_START_ROW + (2 * (slot - 1));

    term_save_cursor();
    term_move_cursor(TERM_TRAIN_STATE_START_COL + TERM_TRAIN_STATE_TRAIN_OFF, slot_offset);
    printf(COM2, "  ");
    term_move_cursor(TERM_TRAIN_STATE_START_COL + TERM_TRAIN_STATE_SPEED_OFF, slot_offset);
    printf(COM2, "    ");
    term_move_cursor(TERM_TRAIN_STATE_START_COL + TERM_TRAIN_STATE_LANDM_OFF, slot_offset);
    printf(COM2, "    ");
    term_move_cursor(TERM_TRAIN_STATE_START_COL + TERM_TRAIN_STATE_DIST_OFF, slot_offset);
    printf(COM2, "    ");
    term_move_cursor(TERM_TRAIN_STATE_START_COL + TERM_TRAIN_STATE_NEXT_OFF, slot_offset);
    printf(COM2, "    ");
    term_restore_cursor();
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

void update_map_sensor(sensor_map_chars_t* sensor_chars,int32_t group, int32_t index, bool state) {
    sensor_map_chars_t* data = &sensor_chars[MAP_DRAW_COORDS(group,index)];
    //bwprintf(COM2, "Group: %d Index: %d COORDS: %d\r\n", group, index, MAP_DRAW_COORDS(group,index));
    ASSERT(MAP_COL + data->x >= 0);
    ASSERT(MAP_ROW + data->y >= 0);
    term_move_cursor(MAP_COL + data->x,MAP_ROW + data->y);
    if(state == true) {
        putc(COM2,data->activated);
    } else if (state == false) {
        putc(COM2,data->original);
    }
}

