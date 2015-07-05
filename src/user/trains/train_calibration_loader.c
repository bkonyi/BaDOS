#include <trains/train_calibration_loader.h>
#include <terminal/terminal.h>

static void load_train_65_calibration_info(train_position_info_t* train_position_info);
static uint16_t train_65_stopping_distance(uint16_t velocity);
static void load_train_66_calibration_info(train_position_info_t* train_position_info);
static uint16_t train_66_stopping_distance(uint16_t velocity);
static void load_default_calibration(train_position_info_t* train_position_info);

void load_calibration(int16_t train, train_position_info_t* train_position_info) {
    switch(train) {
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

void load_train_65_calibration_info(train_position_info_t* train_position_info) {
    uint16_t velocities[MAX_STORED_SPEEDS] = { 100, 239, 292, 352, 419, 484, 556 };
    train_position_info->stopping_distance = train_65_stopping_distance;

    send_term_heavy_msg(false, "Calibrating train 65!");

    int i, j, k;
    for(i = 0; i < 80; ++i) {
        for(j = 0; j < MAX_AV_SENSORS_FROM; ++j) {
            for(k = 0; k < MAX_STORED_SPEEDS; ++k) {
                train_position_info->average_velocities[i][j][k].average_velocity = velocities[k];
                train_position_info->average_velocities[i][j][k].average_velocity_count = 10;
                train_position_info->average_velocities[i][j][k].from = NULL; //TODO should this be changed?
            }
        }
    }
}

uint16_t train_65_stopping_distance(uint16_t velocity) {
    int64_t big_velocity = ((int64_t)velocity);
    int64_t distance;

    //This calculates f(x) = (0.3712)x^2 + 0.6223x + 54.877
    //That's the best polynomial fit for our stopping distances for this train
    distance = ((big_velocity * big_velocity) * 3712) + (6223 * big_velocity) + 548770;

    return ((uint16_t)(distance / 10000));
}

void load_train_66_calibration_info(train_position_info_t* train_position_info) {
    uint16_t velocities[MAX_STORED_SPEEDS] = { 412, 454, 485, 534, 587, 606, 632 };
    train_position_info->stopping_distance = train_66_stopping_distance;
    
    int i, j, k;
    for(i = 0; i < 80; ++i) {
        for(j = 0; j < MAX_AV_SENSORS_FROM; ++j) {
            for(k = 0; k < MAX_STORED_SPEEDS; ++k) {
                train_position_info->average_velocities[i][j][k].average_velocity = velocities[k];
                train_position_info->average_velocities[i][j][k].average_velocity_count = 10;
                train_position_info->average_velocities[i][j][k].from = NULL; //TODO should this be changed?
            }
        }
    }
}

uint16_t train_66_stopping_distance(uint16_t velocity) {
    int64_t big_velocity = ((int64_t)velocity);
    int64_t distance;

    //This calculates f(x) = (0.3148)x^2 - 15.236x + 684.02
    //That's the best polynomial fit for our stopping distances for this train
    distance = ((big_velocity * big_velocity) * 3148) - (152360 * big_velocity) + 6840200;

    return ((uint16_t)(distance / 10000));
}

void load_default_calibration(train_position_info_t* train_position_info) {
    uint16_t velocities[MAX_STORED_SPEEDS] = { 0, 0, 0, 0, 0, 0, 0 };

    //Since we already have these stopping distances...
    train_position_info->stopping_distance = train_65_stopping_distance; 

    int i, j, k;
    for(i = 0; i < 80; ++i) {
        for(j = 0; j < MAX_AV_SENSORS_FROM; ++j) {
            for(k = 0; k < MAX_STORED_SPEEDS; ++k) {
                train_position_info->average_velocities[i][j][k].average_velocity = velocities[k];
                train_position_info->average_velocities[i][j][k].average_velocity_count = 0;
                train_position_info->average_velocities[i][j][k].from = NULL; //TODO should this be changed?
            }
        }
    }
}
