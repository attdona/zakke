
#include "dev/button-sensor.h"

//const struct sensors_sensor button_1_sensor;
//const struct sensors_sensor button_2_sensor;

/*---------------------------------------------------------------------------*/
void
button_1_press(void)
{
  sensors_changed(&button_1_sensor);
}
/*---------------------------------------------------------------------------*/
static int
value_b1(int type)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
configure_b1(int type, int value)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status_b1(int type)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
value_b2(int type)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
configure_b2(int type, int value)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
status_b2(int type)
{
  return 0;
}
/*---------------------------------------------------------------------------*/
SENSORS_SENSOR(button_1_sensor, BUTTON_SENSOR, value_b1, configure_b1, status_b1);
SENSORS_SENSOR(button_2_sensor, BUTTON_SENSOR, value_b2, configure_b2, status_b2);

