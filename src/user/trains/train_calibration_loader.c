#include <trains/train_calibration_loader.h>
#include <terminal/terminal.h>

static void load_train_62_calibration_info(train_position_info_t* train_position_info);
static uint16_t train_62_stopping_distance(uint16_t speed, bool is_under_over);
static void load_train_65_calibration_info(train_position_info_t* train_position_info);
static uint16_t train_65_stopping_distance(uint16_t speed, bool is_under_over);
static void load_train_66_calibration_info(train_position_info_t* train_position_info);
static uint16_t train_66_stopping_distance(uint16_t speed, bool is_under_over);
static void load_default_calibration(train_position_info_t* train_position_info);

void load_calibration(int16_t train, train_position_info_t* train_position_info) {
    switch(train) {
        case 62:
            load_train_62_calibration_info(train_position_info);
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
    int i;
    for(i = 0; i < MAX_STORED_SPEEDS; i ++) {
        train_position_info->default_av_velocity[i]=velocities[i];
    }
}

void load_train_62_calibration_info(train_position_info_t* train_position_info) {
    uint16_t velocities[MAX_STORED_SPEEDS] = { 100, 365, 448, 509, 547, 552, 558 };
    train_position_info->stopping_distance = train_62_stopping_distance;
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

void load_train_65_calibration_info(train_position_info_t* train_position_info) {
    uint16_t velocities[MAX_STORED_SPEEDS] = { 100, 239, 292, 352, 419, 484, 556 };
    train_position_info->stopping_distance = train_65_stopping_distance;
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
    uint16_t velocities[MAX_STORED_SPEEDS] = { 412, 454, 485, 534, 587, 606, 632 };
    train_position_info->stopping_distance = train_66_stopping_distance;
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

void load_default_calibration(train_position_info_t* train_position_info) {
    uint16_t velocities[MAX_STORED_SPEEDS] = { 0, 0, 0, 0, 0, 0, 0 };
    _set_defaults(train_position_info,velocities);
    //Since we already have these stopping distances...
    train_position_info->stopping_distance = train_65_stopping_distance; 

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
