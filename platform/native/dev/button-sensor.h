#ifndef __BUTTON_SENSOR_H__
#define __BUTTON_SENSOR_H__

#include "lib/sensors.h"

#define button_sensor button_1_sensor
extern const struct sensors_sensor button_1_sensor;
extern const struct sensors_sensor button_2_sensor;

#define BUTTON_SENSOR "Button"

void button_press(void);

#endif /* __BUTTON_SENSOR_H__ */
