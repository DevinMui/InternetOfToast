#ifndef __SENSORS_H_
#define __SENSORS_H_

void sensors_init(void);
void read_sensors(void);

#define SENSOR_FIELD_LEN_LIMIT  32
typedef struct
{
    char  Temperature[SENSOR_FIELD_LEN_LIMIT];
    char  Humidity[SENSOR_FIELD_LEN_LIMIT];
    char  AccelX[SENSOR_FIELD_LEN_LIMIT];
    char  AccelY[SENSOR_FIELD_LEN_LIMIT];
    char  AccelZ[SENSOR_FIELD_LEN_LIMIT];
    char  MagnetometerX[SENSOR_FIELD_LEN_LIMIT];
    char  MagnetometerY[SENSOR_FIELD_LEN_LIMIT];
    char  MagnetometerZ[SENSOR_FIELD_LEN_LIMIT];
    char  AmbientLightVis[SENSOR_FIELD_LEN_LIMIT];
    char  AmbientLightIr[SENSOR_FIELD_LEN_LIMIT];
    char  UVindex[SENSOR_FIELD_LEN_LIMIT];
    char  Proximity[SENSOR_FIELD_LEN_LIMIT];
    char  Temperature_Si7020[SENSOR_FIELD_LEN_LIMIT];
    char  Humidity_Si7020[SENSOR_FIELD_LEN_LIMIT];
} K64F_Sensors_t ;

extern K64F_Sensors_t  SENSOR_DATA;

#endif