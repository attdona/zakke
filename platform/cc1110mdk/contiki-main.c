/*
 * Copyright (c) 2013, Piccino Lab (piccino.lab@gmail.com)
 * All rights reserved.
 *
 */

/** \addtogroup CC1110
 * @{
 */

/**
 * \defgroup startup CC1110 startup
 *
 *
 *         bootstrap entry point.
 *
 *         Attilio Dona' - <piccino.lab@gmail.com>
 *
 *
 * @{
 */
#include <stdio.h>
#include "contiki.h"
#include "soc.h"
#include "stack.h"
#include "sys/clock.h"
#include "sys/autostart.h"
#include "dev/serial-line.h"
#include "dev/slip.h"
#include "dev/leds.h"
#include "dev/io-arch.h"
#include "dev/dma.h"
#include "dev/watchdog.h"
#include "dev/clock-isr.h"
#include "dev/port2.h"
#include "dev/lpm.h"
#include "dev/button-sensor.h"
#include "dev/leds-arch.h"
#include "net/rime.h"
#include "net/netstack.h"
#include "net/mac/frame802154.h"
#include "debug.h"
#include "cc1110.h"
#include "sfr-bits.h"
#include "contiki-lib.h"
#include "contiki-net.h"
/*---------------------------------------------------------------------------*/
#if VIZTOOL_CONF_ON
PROCESS_NAME(viztool_process);
#endif
/*---------------------------------------------------------------------------*/
#if STARTUP_CONF_VERBOSE
#define PUTSTRING(...) putstring(__VA_ARGS__)
#define PUTHEX(...) puthex(__VA_ARGS__)
#define PUTBIN(...) putbin(__VA_ARGS__)
#define PUTCHAR(...) putchar(__VA_ARGS__)
#else
#define PUTSTRING(...)
#define PUTHEX(...)
#define PUTBIN(...)
#define PUTCHAR(...)
#endif
/*---------------------------------------------------------------------------*/
#if CLOCK_CONF_STACK_FRIENDLY
extern volatile uint8_t sleep_flag;
#endif
/*---------------------------------------------------------------------------*/

__xdata __at (0x7FFE) unsigned char target_address[] = {(RIMEADDR>>8)&0xFF,RIMEADDR&0xFF};

SENSORS(&button1, &button2);

extern rimeaddr_t rimeaddr_node_addr;
static CC_AT_DATA uint16_t len;


/*---------------------------------------------------------------------------*/
#if ENERGEST_CONF_ON
static unsigned long irq_energest = 0;
#define ENERGEST_IRQ_SAVE(a) do { \
    a = energest_type_time(ENERGEST_TYPE_IRQ); } while(0)
#define ENERGEST_IRQ_RESTORE(a) do { \
    energest_type_set(ENERGEST_TYPE_IRQ, a); } while(0)
#else
#define ENERGEST_IRQ_SAVE(a) do {} while(0)
#define ENERGEST_IRQ_RESTORE(a) do {} while(0)
#endif
/*---------------------------------------------------------------------------*/
#if 0
static void
fade(int l) CC_NON_BANKED
{
  volatile int i, a;
  int k, j;
  for(k = 0; k < 400; ++k) {
    j = k > 200 ? 400 - k : k;

    leds_on(l);
    for(i = 0; i < j; ++i) {
      a = i;
    }
    leds_off(l);
    for(i = 0; i < 200 - j; ++i) {
      a = i;
    }
  }
}
#endif



/*---------------------------------------------------------------------------*/
static void
set_rime_addr(void)
{
  char i;

  PUTSTRING("Rime is 0x");
  PUTHEX(sizeof(rimeaddr_t));
  PUTSTRING(" bytes long\n");

  PUTSTRING("Reading MAC from flash\n");

  for(i = 0; i < RIMEADDR_SIZE; i++) {
    rimeaddr_node_addr.u8[i] = target_address[i];
  }

  /* Now the address is stored MSB first */
#if STARTUP_CONF_VERBOSE
  PUTSTRING("Rime configured with address ");
  for(i = 0; i < RIMEADDR_SIZE - 1; i++) {
    PUTHEX(rimeaddr_node_addr.u8[i]);
    PUTCHAR(':');
  }
  PUTHEX(rimeaddr_node_addr.u8[i]);
  PUTCHAR('\n');
#endif

  return;
}

/** \brief main entry point initialize clocks, interrupts and radio module
 *
 *   stack_poison is needed because ...
 *
 */
int
main(void)
{
  /* Hardware initialization */
  clock_init();
  soc_init();
  rtimer_init();

  stack_poison();

  /* Init LEDs here */
  leds_init();
  leds_off(LEDS_ALL);
  //fade(LEDS_GREEN);

  /* initialize process manager. */
  process_init();

  // prepare the DMA controller
  dma_init();

  io_arch_init();
  PUTSTRING("booting\n");

#if SLIP_ARCH_CONF_ENABLE
  slip_arch_init(0);
#else
  io_arch_set_input(serial_line_input_byte);
  serial_line_init();
#endif
  //fade(LEDS_RED);
  watchdog_init();

  /* Initialise the H/W RNG engine. */
  random_init(0xEA);

  /* start services */
  process_start(&etimer_process, NULL);

  ctimer_init();

  /* initialize the netstack */
  netstack_init();

  set_rime_addr();

#if BUTTON_SENSOR_ON
  process_start(&sensors_process, NULL);
  BUTTON_SENSOR_ACTIVATE();
#endif

  energest_init();
  ENERGEST_ON(ENERGEST_TYPE_CPU);
  autostart_start(autostart_processes);

  watchdog_start();

  //fade(LEDS_YELLOW);

  while(1) {
    uint8_t r;
    do {
      /* Reset watchdog and handle polls and events */
      watchdog_periodic();

#if CLOCK_CONF_STACK_FRIENDLY
      if(sleep_flag) {
        if(etimer_pending() &&
            (etimer_next_expiration_time() - clock_time() - 1) > MAX_TICKS) {
          etimer_request_poll();
        }
        sleep_flag = 0;
      }
#endif
      r = process_run();
    } while(r > 0);

    len = NETSTACK_RADIO.pending_packet();
    if(len) {
        // moved before DMA configuration
        packetbuf_clear();

        // needed even if DMA intervention did the hard job
        len = NETSTACK_RADIO.read(packetbuf_dataptr(), PACKETBUF_SIZE);
        if(len > 0) {
          packetbuf_set_datalen(len);
          NETSTACK_RDC.input();
        }
    }

#if 0
//#if LPM_MODE
#if (LPM_MODE==LPM_MODE_PM2)
    SLEEP &= ~SLEEP_OSC_PD;            /* Make sure both HS OSCs are on */
    while(!(SLEEP & SLEEP_HFRC_STB));  /* Wait for RCOSC to be stable */
    CLKCON = (CLKCON & ~0x07) | CLKCONCMD_OSC | 0x01; /* Switch to the RCOSC and set max CPU speed (CLKCON.CLKSPD = 1)*/
    while(!(CLKCON & CLKCONCMD_OSC));      /* Wait till it's happened */
    SLEEP |= SLEEP_OSC_PD;             /* Turn the other one off */
#endif /* LPM_MODE==LPM_MODE_PM2 */

    /*
     * Set MCU IDLE or Drop to PM1. Any interrupt will take us out of LPM
     * Sleep Timer will wake us up in no more than 7.8ms (max idle interval)
     */
    SLEEP = (SLEEP & 0xFC) | (LPM_MODE - 1);

#if (LPM_MODE==LPM_MODE_PM2)
    /*
     * Wait 3 NOPs. Either an interrupt occurred and SLEEP.MODE was cleared or
     * no interrupt occurred and we can safely power down
     */
    __asm
      nop
      nop
      nop
    __endasm;

#endif /* LPM_MODE==LPM_MODE_PM2 */

      ENERGEST_OFF(ENERGEST_TYPE_CPU);
      ENERGEST_ON(ENERGEST_TYPE_LPM);

      /* We are only interested in IRQ energest while idle or in LPM */
      ENERGEST_IRQ_RESTORE(irq_energest);

      /* Go IDLE or Enter PM1 */
      PCON |= PCON_IDLE;

      /* First instruction upon exiting PM1 must be a NOP */
      __asm
        nop
      __endasm;

      /* Remember energest IRQ for next pass */
      ENERGEST_IRQ_SAVE(irq_energest);

      ENERGEST_ON(ENERGEST_TYPE_CPU);
      ENERGEST_OFF(ENERGEST_TYPE_LPM);

#if (LPM_MODE==LPM_MODE_PM1 || LPM_MODE==LPM_MODE_PM2)

      SLEEP &= ~SLEEP_OSC_PD;            /* Make sure both HS OSCs are on */
      while(!(SLEEP & SLEEP_XOSC_STB));  /* Wait for XOSC to be stable */
      CLKCON &= ~CLKCONCMD_OSC;              /* Switch to the XOSC */
      /*
       * On occasion the XOSC is reported stable when in reality it's not.
       * We need to wait for a safeguard of 64us or more before selecting it
       */
      clock_delay_usec(65);
      while(CLKCON & CLKCONCMD_OSC);         /* Wait till it's happened */
      SLEEP |= SLEEP_OSC_PD;                 /* Power down HS RCOSC */
#endif
#endif /* LPM_MODE */
  }


  //return 0;
}
/*---------------------------------------------------------------------------*/
/** @} */
/** @} */
