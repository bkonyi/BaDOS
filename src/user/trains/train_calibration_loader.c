#include <trains/train_calibration_loader.h>

void load_train_65_calibration_info(train_position_info_t* train_position_info);
void load_default_calibration(train_position_info_t* train_position_info);

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
    //TODO set properly
    uint16_t velocities[MAX_STORED_SPEEDS] = { 100, 100, 100, 100, 100, 100, 100 };

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

void load_default_calibration(train_position_info_t* train_position_info) {
    //TODO set properly
    uint16_t velocities[MAX_STORED_SPEEDS] = { 100, 100, 100, 100, 100, 100, 100 };

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
