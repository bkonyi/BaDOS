#include <trains/train_calibration_loader.h>
#include <terminal/terminal.h>
#include <terminal/terminal_debug_log.h>

static void load_train_58_calibration_info(train_position_info_t* train_position_info);
static uint16_t train_58_stopping_distance(uint16_t speed, bool is_under_over);
static uint32_t train_58_short_move_time(uint16_t speed, int16_t distance);
static void load_train_62_calibration_info(train_position_info_t* train_position_info);
static uint16_t train_62_stopping_distance(uint16_t speed, bool is_under_over);
static uint32_t train_62_short_move_time(uint16_t speed, int16_t distance);
static void load_train_64_calibration_info(train_position_info_t* train_position_info);
static uint16_t train_64_stopping_distance(uint16_t speed, bool is_under_over);
static uint32_t train_64_short_move_time(uint16_t speed, int16_t distance);
static void load_train_65_calibration_info(train_position_info_t* train_position_info);
static uint16_t train_65_stopping_distance(uint16_t speed, bool is_under_over);
static void load_train_66_calibration_info(train_position_info_t* train_position_info);
static uint16_t train_66_stopping_distance(uint16_t speed, bool is_under_over);
static uint32_t train_66_short_move_time(uint16_t speed, int16_t distance);
static void load_default_calibration(train_position_info_t* train_position_info);

void load_calibration(int16_t train, train_position_info_t* train_position_info) {
    send_term_debug_log_msg("Loading calibration info for: %d", train);

    switch(train) {
        case 58:
            load_train_58_calibration_info(train_position_info);
            break;
        case 62:
            load_train_62_calibration_info(train_position_info);
            break;
        case 64:
            load_train_64_calibration_info(train_position_info);
            break;
        case 65:
            load_train_65_calibration_info(train_position_info);
            break;
        case 66:
            load_train_66_calibration_info(train_position_info);
            break;
        default:
            load_default_calibration(train_position_info);
            break;
    }
}

void _set_defaults(train_position_info_t* train_position_info, uint16_t* velocities) {
    train_position_info->acceleration_while_accel_thousandths_mm_ticks = 140;
    train_position_info->acceleration_at_max_thousandths_mm_ticks = 210 ;
    train_position_info->acceleration_current_thousandths_mm_ticks = train_position_info->acceleration_while_accel_thousandths_mm_ticks;
    int i;
    for(i = 0; i < MAX_STORED_SPEEDS; i ++) {
        train_position_info->default_av_velocity[i]=velocities[i];
    }
}

void load_train_58_calibration_info(train_position_info_t* train_position_info) {
    uint16_t velocities[MAX_STORED_SPEEDS] = { 100, 249, 306, 371, 444, 518, 600 };
    train_position_info->stopping_distance = train_58_stopping_distance;
    train_position_info->short_move_time = train_58_short_move_time;
    _set_defaults(train_position_info,velocities);

    train_position_info->acceleration_while_accel_thousandths_mm_ticks = 135;
    train_position_info->acceleration_at_max_thousandths_mm_ticks = 140 ;
    train_position_info->acceleration_current_thousandths_mm_ticks = train_position_info->acceleration_while_accel_thousandths_mm_ticks;
    int i, j, k;
    for(i = 0; i < 80; ++i) {
        for(j = 0; j < MAX_AV_SENSORS_FROM; ++j) {
            for(k = 0; k < MAX_STORED_SPEEDS; ++k) {
                train_position_info->average_velocities[i][j][k].average_velocity = velocities[k];
                train_position_info->average_velocities[i][j][k].average_velocity_count = 10; 
                train_position_info->average_velocities[i][j][k].from = NULL;
            }
        }
    }
}

uint16_t train_58_stopping_distance(uint16_t speed, bool is_under_over) {
    int64_t big_speed = ((int64_t)speed);
    int64_t distance;
    (void)is_under_over;

    //This calculates f(x) = (21.964)x^2 - 315.65x + 1394.5
    //That's the best polynomial fit for our stopping distances for this train
    distance = ((big_speed * big_speed) * 21964) - (315650 * big_speed) + 1394500;

    if(distance < 0) {
        return 0;
    }

    return ((uint16_t)(distance / 1000));
}

uint32_t train_58_short_move_time(uint16_t speed, int16_t distance) {
    if(distance <= 0) {
        return 0;
    }

    uint64_t long_distance = distance;
    uint32_t result = 0;

    speed = 12; //Maybe we'll do something with this later.

    switch(speed) {
        case 12:
            if(long_distance >= 51) { 
                //This calculates f(x) = (1/344) * (sqrt(6880000 * x - 345469551) + 15977)
                //which is the equation for determining time to move a certain distance
                result = (sqrt(6880000ULL * long_distance - 345469551ULL) + 15977ULL) / 344;
            } else {
                //We don't want to calculate the square root of a negative number...
                //Just assume the distance is 0
                result = 250;
            }
            break;
        default:
            ASSERT(0);
            break;
    }

    return result;
}

void load_train_62_calibration_info(train_position_info_t* train_position_info) {
    uint16_t velocities[MAX_STORED_SPEEDS] = { 100, 365, 448, 509, 547, 552, 558 };
    train_position_info->stopping_distance = train_62_stopping_distance;
    train_position_info->short_move_time = train_62_short_move_time;
    
    _set_defaults(train_position_info,velocities);
    train_position_info->acceleration_while_accel_thousandths_mm_ticks = 80;
    train_position_info->acceleration_at_max_thousandths_mm_ticks = 210 ;
    train_position_info->acceleration_current_thousandths_mm_ticks = train_position_info->acceleration_while_accel_thousandths_mm_ticks;
    int i, j, k;
    for(i = 0; i < 80; ++i) {
        for(j = 0; j < MAX_AV_SENSORS_FROM; ++j) {
            for(k = 0; k < MAX_STORED_SPEEDS; ++k) {
                train_position_info->average_velocities[i][j][k].average_velocity = velocities[k];
                //Lowered this count since this train isn't very consistent
                train_position_info->average_velocities[i][j][k].average_velocity_count = 3; 
                train_position_info->average_velocities[i][j][k].from = NULL;
            }
        }
    }
}

uint16_t train_62_stopping_distance(uint16_t speed, bool is_under_over) {
    int64_t big_speed = ((int64_t)speed);
    int64_t distance;
    (void)is_under_over;

    //This calculates f(x) = (-3.9252)x^2 + 144.59x - 426.65
    //That's the best polynomial fit for our stopping distances for this train
    distance = ((big_speed * big_speed) * -39252) + (1445900 * big_speed) - 4266500;

    if(distance < 0) {
        return 0;
    }

    return ((uint16_t)(distance / 10000));
}

uint32_t train_62_short_move_time(uint16_t speed, int16_t distance) {
    if(distance <= 0) {
        return 0;
    }

    uint64_t long_distance = distance;
    uint32_t result = 0;

    speed = 12; //Maybe we'll do something with this later.

    switch(speed) {
        case 12:
            if(long_distance >= 83) { 
                //This calculates f(x) = (1/322) * (sqrt(6440000 * x - 528910491) + 32647)
                //which is the equation for determining time to move a certain distance
                result = (sqrt(6440000ULL * long_distance - 528910591ULL) + 32647ULL) / 322;
            } else {
                //We don't want to calculate the square root of a negative number...
                //Just assume the distance is 0
                result = 250;
            }
            break;
        default:
            ASSERT(0);
            break;
    }

    return result + 1;
}

void load_train_64_calibration_info(train_position_info_t* train_position_info) {
    uint16_t velocities[MAX_STORED_SPEEDS] = { 100, 473, 499, 551, 613, 620, 640 };
    train_position_info->stopping_distance = train_64_stopping_distance;
    train_position_info->short_move_time = train_64_short_move_time;
    _set_defaults(train_position_info,velocities);
    int i, j, k;
    for(i = 0; i < 80; ++i) {
        for(j = 0; j < MAX_AV_SENSORS_FROM; ++j) {
            for(k = 0; k < MAX_STORED_SPEEDS; ++k) {
                train_position_info->average_velocities[i][j][k].average_velocity = velocities[k];
                //Lowered this count since this train isn't very consistent
                train_position_info->average_velocities[i][j][k].average_velocity_count = 3; 
                train_position_info->average_velocities[i][j][k].from = NULL;
            }
        }
    }
}

uint16_t train_64_stopping_distance(uint16_t speed, bool is_under_over) {
    int64_t big_speed = ((int64_t)speed);
    int64_t distance;
    (void)is_under_over;

    //This calculates f(x) = (-8.4524)x^2 + 269.55x - 1108.4
    //That's the best polynomial fit for our stopping distances for this train
    distance = ((big_speed * big_speed) * -84524) + (2695500 * big_speed) - 11084000;

    if(distance < 0) {
        return 0;
    }

    return ((uint16_t)(distance / 10000));
}

uint32_t train_64_short_move_time(uint16_t speed, int16_t distance) {
    if(distance <= 0) {
        return 0;
    }

    uint64_t long_distance = distance;
    uint32_t result = 0;

    speed = 12; //Maybe we'll do something with this later.

    switch(speed) {
        case 12:
            if(long_distance >= 90) { 
                //This calculates f(x) = (5/318) * (5 * sqrt(10176 * x - 914903) + 7609)
                //which is the equation for determining time to move a certain distance
                result = (5 * (5 * sqrt(10176ULL * long_distance - 914903ULL) + 7609ULL)) / 318;
            } else {
                //We don't want to calculate the square root of a negative number...
                //Just assume the distance is 0
                result = 250;
            }
            break;
        default:
            ASSERT(0);
            break;
    }

    return result;
}

void load_train_65_calibration_info(train_position_info_t* train_position_info) {
    uint16_t velocities[MAX_STORED_SPEEDS] = { 100, 239, 292, 352, 419, 484, 556 };

    train_position_info->stopping_distance = train_65_stopping_distance;
    train_position_info->short_move_time = train_62_short_move_time; //TODO change this

    _set_defaults(train_position_info,velocities);
    int i, j, k;
    for(i = 0; i < 80; ++i) {
        for(j = 0; j < MAX_AV_SENSORS_FROM; ++j) {
            for(k = 0; k < MAX_STORED_SPEEDS; ++k) {
                train_position_info->average_velocities[i][j][k].average_velocity = velocities[k];
                train_position_info->average_velocities[i][j][k].average_velocity_count = 10;
                train_position_info->average_velocities[i][j][k].from = NULL;
            }
        }
    }
}

uint16_t train_65_stopping_distance(uint16_t speed, bool is_under_over) {
    int64_t big_speed = ((int64_t)speed);
    int64_t distance;

    if(is_under_over) {
        distance = ((big_speed * big_speed) * 16640) - (186080 * big_speed) + 670710;
    } else {
        //This calculates f(x) = (0.3712)x^2 + 0.6223x + 54.877
        //That's the best polynomial fit for our stopping distances for this train
        distance = ((big_speed * big_speed) * 17518) - (218360 * big_speed) + 829660;
    }

    if(distance < 0) {
        return 0;
    }

    return ((uint16_t)(distance / 1000));
}

void load_train_66_calibration_info(train_position_info_t* train_position_info) {
    send_term_debug_log_msg("Loading train 66 calibration info");
    uint16_t velocities[MAX_STORED_SPEEDS] = { 412, 454, 485, 534, 587, 606, 632 };
    train_position_info->stopping_distance = train_66_stopping_distance;
    train_position_info->short_move_time = train_66_short_move_time;

    _set_defaults(train_position_info,velocities);
    int i, j, k;
    for(i = 0; i < 80; ++i) {
        for(j = 0; j < MAX_AV_SENSORS_FROM; ++j) {
            for(k = 0; k < MAX_STORED_SPEEDS; ++k) {
                train_position_info->average_velocities[i][j][k].average_velocity = velocities[k];
                train_position_info->average_velocities[i][j][k].average_velocity_count = 10;
                train_position_info->average_velocities[i][j][k].from = NULL;
            }
        }
    }
}

uint16_t train_66_stopping_distance(uint16_t speed, bool is_under_over) {
    int64_t big_speed = ((int64_t)speed);
    int64_t distance;
    (void)is_under_over; //TODO use this eventually

    //This calculates f(x) = (0.3148)x^2 - 15.236x + 684.02
    //That's the best polynomial fit for our stopping distances for this train
    distance = ((big_speed * big_speed) * -27181) + (1302900 * big_speed) - 3213400;

    if(distance < 0) {
        return 0;
    }

    return (((uint16_t)(distance / 10000)));
}

uint32_t train_66_short_move_time(uint16_t speed, int16_t distance) {
    if(distance <= 0) {
        return 0;
    }

    uint64_t long_distance = distance;
    uint32_t result = 0;

    speed = 12; //Maybe we'll do something with this later.

    switch(speed) {
        case 12:
            if(distance >= 77) {
                //This calculates f(x) = (5/274) * (sqrt(219200 * x - 16747751) + 6013)
                //which is the equation for determining time to move a certain distance
                result = 5 * (sqrt(219200ULL * long_distance - 16747751ULL) + 6013ULL) / 274;
            } else {
                //Don't want to do a square root of a negative number..
                result = 250;
            }
            break;
        default:
            ASSERT(0);
            break;
    }

    return result;
}

void load_default_calibration(train_position_info_t* train_position_info) {
    uint16_t velocities[MAX_STORED_SPEEDS] = { 0, 0, 0, 0, 0, 0, 0 };
    _set_defaults(train_position_info,velocities);
    //Since we already have these stopping distances...
    train_position_info->stopping_distance = train_65_stopping_distance; 
    train_position_info->short_move_time = train_62_short_move_time; //I love consistency, can't you tell?

    int i, j, k;
    for(i = 0; i < 80; ++i) {
        for(j = 0; j < MAX_AV_SENSORS_FROM; ++j) {
            for(k = 0; k < MAX_STORED_SPEEDS; ++k) {
                train_position_info->average_velocities[i][j][k].average_velocity = velocities[k];
                train_position_info->average_velocities[i][j][k].average_velocity_count = 0;
                train_position_info->average_velocities[i][j][k].from = NULL;
            }
        }
    }
}
