#include <trains/train_calibration_loader.h>

static void load_train_65_calibration_info(train_position_info_t* train_position_info);
static uint16_t train_65_stopping_distance(uint16_t velocity);
static void load_default_calibration(train_position_info_t* train_position_info);

void load_calibration(int16_t train, train_position_info_t* train_position_info) {
    switch(train) {
        case 65:
            load_train_65_calibration_info(train_position_info);
            break;
        default:
            load_default_calibration(train_position_info);
            break;
    }
}

void load_train_65_calibration_info(train_position_info_t* train_position_info) {
    uint16_t velocities[MAX_STORED_SPEEDS] = { 100, 239, 292, 352, 419, 484, 556 };
    train_position_info->stopping_distance = train_65_stopping_distance;

    int i, j, k;
    for(i = 0; i < 80; ++i) {
        for(j = 0; j < MAX_AV_SENSORS_FROM; ++j) {
            for(k = 0; k < MAX_STORED_SPEEDS; ++k) {
                train_position_info->average_velocities[i][j][k].average_velocity = velocities[k];
                train_position_info->average_velocities[i][j][k].average_velocity_count = 10;
                //TODO train_position_info->average_velocities[i][j][k].from = NULL;
            }
        }
    }
}

uint16_t train_65_stopping_distance(uint16_t velocity) {
    int64_t big_velocity = ((int64_t)velocity) * 100000;
    int64_t distance;

    //This calculates f(x) = (-2 * 10^(-5))x^2 + 0.0633x + 7.4358
    //That's the best polynomial fit for our stopping distances for this train
    distance = ((big_velocity * big_velocity) * -2) + (6330 * big_velocity) + 743580;

    return ((uint16_t)(distance / 100000));
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
                //TODO train_position_info->average_velocities[i][j][k].from = NULL;
            }
        }
    }
}
