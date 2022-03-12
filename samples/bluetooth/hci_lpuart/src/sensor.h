/**
 * @file sensor.h
 * 
 */
#ifndef _SENSOR_H_
#define _SENSOR_H_

#include <zephyr/types.h>

void init_sensor(void);

int measure(int16_t *p_buffer);

int get_tilt_count(void);

#endif /* _SENSOR_H_ */
