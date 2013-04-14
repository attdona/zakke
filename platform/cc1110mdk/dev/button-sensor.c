/*
 * Copyright (c) 2013, Piccino Lab (piccino.lab@gmail.com)
 * All rights reserved.
 *
 */

/*
 * Copyright (c) 2005, Swedish Institute of Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 */
/**
 * \file
 *         Implementation of the buttons drivers
 *
 *
 * \author
 *         Attilio Dona' - <piccino.lab@gmail.com>
 */

#include "dev/port.h"
#include "dev/button-sensor.h"
#include "dev/watchdog.h"
/*---------------------------------------------------------------------------*/
static CC_AT_DATA struct timer debouncetimer;

struct sensors_sensor button1 = { BUTTON1_SENSOR, value_b1, configure_b1, status_b1 };
struct sensors_sensor button2 = { BUTTON2_SENSOR, value_b2, configure_b2, status_b2 };

/*---------------------------------------------------------------------------*/
/* Button 1 - MASTER */
/*---------------------------------------------------------------------------*/
static int
value_b1(int type)
{
  type;
  return BUTTON_READ(1) || !timer_expired(&debouncetimer);
}
/*---------------------------------------------------------------------------*/
static int
status_b1(int type)
{
  switch(type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return BUTTON_IRQ_ENABLED(1);
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
configure_b1(int type, int value)
{
  switch(type) {
  case SENSORS_HW_INIT:
    BUTTON_IRQ_ON_PRESS(1);
    BUTTON_FUNC_GPIO(1);
    BUTTON_DIR_INPUT(1);
    return 1;
  case SENSORS_ACTIVE:
    if(value) {
      if(!BUTTON_IRQ_ENABLED(1)) {
        timer_set(&debouncetimer, 0);
        BUTTON_IRQ_FLAG_OFF(1);
        BUTTON_IRQ_ENABLE(1);
      }
    } else {
      BUTTON_IRQ_DISABLE(1);
    }
    return 1;
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
/* Button 2 - SLAVE */
/*---------------------------------------------------------------------------*/
static int
value_b2(int type)
{
  type;
  return BUTTON_READ(2) || !timer_expired(&debouncetimer);
}
/*---------------------------------------------------------------------------*/
static int
status_b2(int type)
{
  switch(type) {
  case SENSORS_ACTIVE:
  case SENSORS_READY:
    return BUTTON_IRQ_ENABLED(2);
  }
  return 0;
}
/*---------------------------------------------------------------------------*/
static int
configure_b2(int type, int value)
{
  switch(type) {
  case SENSORS_HW_INIT:
    BUTTON_IRQ_ON_PRESS(2);
    BUTTON_FUNC_GPIO(2);
    BUTTON_DIR_INPUT(2);
    return 1;
  case SENSORS_ACTIVE:
    if(value) {
      if(!BUTTON_IRQ_ENABLED(2)) {
        timer_set(&debouncetimer, 0);
        BUTTON_IRQ_FLAG_OFF(2);
        BUTTON_IRQ_ENABLE(2);
      }
    } else {
      BUTTON_IRQ_DISABLE(2);
    }
    return 1;
  }
  return 0;
}

/*---------------------------------------------------------------------------*/
/* ISRs */
/*---------------------------------------------------------------------------*/
/* avoid referencing bits, we don't call code which use them */
#pragma save
#if CC_CONF_OPTIMIZE_STACK_SIZE
#pragma exclude bits
#endif
void
port_1_isr(void) __interrupt(P1INT_VECTOR)
{
  EA = 0;
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  /* This ISR is for the entire port. Check if the interrupt was caused by our
   * button's pin. */
  if(BUTTON_IRQ_CHECK(1)) {
    if(timer_expired(&debouncetimer)) {
      timer_set(&debouncetimer, CLOCK_SECOND / 8);
      sensors_changed(&button1);
    }
  }
  if(BUTTON_IRQ_CHECK(2)) {
    if(timer_expired(&debouncetimer)) {
      timer_set(&debouncetimer, CLOCK_SECOND / 8);
#if CC1110_CONF_B2_REBOOTS
      watchdog_reboot();
#else /* General Purpose */
      sensors_changed(&button2);
#endif
    }
  }

  BUTTON_IRQ_FLAG_OFF(1);
  BUTTON_IRQ_FLAG_OFF(2);

  ENERGEST_OFF(ENERGEST_TYPE_IRQ);
  EA = 1;
}

#pragma restore
/*---------------------------------------------------------------------------*/
//SENSORS_SENSOR(button_1_sensor, BUTTON_SENSOR, value_b1, configure_b1, status_b1);
//SENSORS_SENSOR(button_2_sensor, BUTTON_SENSOR, value_b2, configure_b2, status_b2);

