#ifndef __CONTIKI_CONF_H__
#define __CONTIKI_CONF_H__

#include "8051def.h"
#include "sys/cc.h"
#include <string.h>


/* Include Project Specific conf */
#ifdef PROJECT_CONF_H
#include "project-conf.h"
#endif /* PROJECT_CONF_H */

#define MODEL_STRING "TI cc1110-mini-dk\n"

#define UIP_CONF_IPV6 0

#ifndef FREQ_CONF_915MHZ
  #define FREQ_868MHZ 1
#else
  #define FREQ_915MHZ 1
#endif

/*
 * Define this as 1 to poll the etimer process from within main instead of from
 * the clock ISR. This reduces the ISR's stack usage and may prevent crashes.
 */
#ifndef CLOCK_CONF_STACK_FRIENDLY
#define CLOCK_CONF_STACK_FRIENDLY 1
#endif

#ifndef STACK_CONF_DEBUGGING
#define STACK_CONF_DEBUGGING  0
#endif

/* Energest Module */
#ifndef ENERGEST_CONF_ON
#define ENERGEST_CONF_ON      0
#endif

/* Verbose Startup? Turning this off saves plenty of bytes of CODE in HOME */
#ifndef STARTUP_CONF_VERBOSE
#define STARTUP_CONF_VERBOSE  0
#endif

/* More CODE space savings by turning off process names */
#define PROCESS_CONF_NO_PROCESS_NAMES 1

#define UART_ON_USART     0

#define UART1_CONF_ENABLE 0

#ifndef UART0_CONF_ENABLE
#define UART0_CONF_ENABLE  1
#endif
#ifndef UART0_CONF_WITH_INPUT
#define UART0_CONF_WITH_INPUT 1
#endif

#ifndef UART0_CONF_HIGH_SPEED
#define UART0_CONF_HIGH_SPEED 0
#endif

/* Are we a SLIP bridge? */
#if SLIP_ARCH_CONF_ENABLE
/* Make sure the UART is enabled, with interrupts */
#undef UART0_CONF_ENABLE
#undef UART0_CONF_WITH_INPUT
#define UART0_CONF_ENABLE  1
#define UART0_CONF_WITH_INPUT 1
#define UIP_FALLBACK_INTERFACE slip_interface
#endif

/* Output all captured frames over the UART in hexdump format */
#ifndef CC1110_RF_CONF_HEXDUMP
#define CC1110_RF_CONF_HEXDUMP 0
#endif

#if CC1110_RF_CONF_HEXDUMP
/* We need UART1 output */
#undef UART_ZERO_CONF_ENABLE
#define UART_ZERO_CONF_ENABLE   1
#endif

/* Code Shortcuts */
/*
 * When set, this directive also configures the following bypasses:
 *   - process_post_synch() in tcpip_input() (we call packet_input())
 *   - process_post_synch() in tcpip_uipcall (we call the relevant pthread)
 *   - mac_call_sent_callback() is replaced with sent() in various places
 *
 * These are good things to do, they reduce stack usage and prevent crashes
 */
#define NETSTACK_CONF_SHORTCUTS   1

/* Interrupt Number 6: Shared between P2 Inputs, I2C and USB
 * A single ISR handles all of the above. Leave this as is if you are not
 * interested in any of the above. Define as 1 (e.g. in project-conf.h) if
 * at least one of those interrupt sources will need handled */
#ifndef PORT_2_ISR_ENABLED
#define PORT_2_ISR_ENABLED 0
#endif

/*
 * Sensors
 * It is harmless to #define XYZ 1
 * even if the sensor is not present on our device
 */
#ifndef BUTTON_SENSOR_CONF_ON
#define BUTTON_SENSOR_CONF_ON   1  /* Buttons */
#endif

/*---------------------------------------------------------------------------*/
/* LEDs */
/*---------------------------------------------------------------------------*/
/* Some files include leds.h before us */
#undef LEDS_GREEN
#undef LEDS_RED

/*
 * CC1110 mini DK LEDs
 *  1: P1_0 (Green)
 *  2: P1_1 (Red)
 */

#define LEDS_GREEN    1
#define LEDS_RED      2

/* H/W Pin mapping */
#define LED1_PIN   P1_0
#define LED2_PIN   P1_1


/* P0DIR and P0SEL masks */
#define LED1_MASK  0x01
#define LED2_MASK  0x02

/* The platform has 2 Leds, LEDS_ALL is the 11 bitmask value */
#define LEDS_CONF_ALL 3

/* ADC - Turning this off will disable everything below */
#ifndef ADC_SENSOR_CONF_ON
#define ADC_SENSOR_CONF_ON      1
#endif
#define TEMP_SENSOR_CONF_ON     1  /* Temperature */
#define VDD_SENSOR_CONF_ON      1  /* Supply Voltage */
#define BATTERY_SENSOR_CONF_ON  0  /* Battery */

/* Low Power Modes - We only support PM0/Idle and PM1 */
#ifndef LPM_CONF_MODE
#define LPM_CONF_MODE         0 /* 0: no LPM, 1: MCU IDLE, 2: Drop to PM1 */
#endif

/* DMA Configuration */
#ifndef DMA_CONF_ON
 #define DMA_CONF_ON 1
 #define HAVE_RF_DMA 1

/*
 *   Use DMA to read the radio packet from 1 byte RFD FIFO into memory
 */
 #define DMA_RADIO_CHANNEL 0
#endif

/* Network Stack */
#define NETSTACK_CONF_NETWORK rime_driver

#ifndef NETSTACK_CONF_MAC
#define NETSTACK_CONF_MAC     csma_driver
#endif

#ifndef NETSTACK_CONF_RDC
#define NETSTACK_CONF_RDC     nullrdc_noframer_driver
#define NULLRDC_802154_AUTOACK 1
#define NULLRDC_802154_AUTOACK_HW 1
#endif

#ifndef NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE
#define NETSTACK_CONF_RDC_CHANNEL_CHECK_RATE 8
#endif

#define NETSTACK_CONF_FRAMER  framer_802154
#define NETSTACK_CONF_RADIO   cc1101_rf_driver

/* RF Config */
#define IEEE802154_CONF_PANID 0x5449 /* TI */

/* Network setup for non-IPv6 (rime). */
#define UIP_CONF_IP_FORWARD                  1
#define UIP_CONF_BUFFER_SIZE               108
#define RIME_CONF_NO_POLITE_ANNOUCEMENTS     0
#define QUEUEBUF_CONF_NUM                    8

#endif /* __CONTIKI_CONF_H__ */
