/*
 * Copyright (c) 2013, Piccino Lab (piccino.lab@gmail.com)
 * All rights reserved.
 *
 */

/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 *         Implementation of the clock functions for the cc1110.
 *         Ported over from the cc243x original.
 *         For compliance with cc1110 minimum sleep time requirement
 *         (SWRS033G Page 126/244) the ST period is set to 15.6 msec
 *
 *
 * \author
 *         Zach Shelby (zach@sensinode.com) - original (cc243x)
 *         George Oikonomou - <oikonomou@users.sourceforge.net> - cc2530 port
 *         Attilio Dona' - <piccino.lab@gmail.com>
 */
#include "sfr-bits.h"
#include "sys/clock.h"
#include "sys/etimer.h"
#include "cc1110.h"
#include "sys/energest.h"

/*---------------------------------------------------------------------------*/
#if CLOCK_CONF_STACK_FRIENDLY
volatile uint8_t sleep_flag;
#endif
/*---------------------------------------------------------------------------*/
/* Do NOT remove the absolute address and do NOT remove the initialiser here */
__xdata __at(0x0000) static unsigned long timer_value = 0;

static volatile CC_AT_DATA clock_time_t count = 0; /* Uptime in ticks */
static volatile CC_AT_DATA clock_time_t seconds = 0; /* Uptime in secs */
/*---------------------------------------------------------------------------*/
/**
 * Each iteration is ~1.0xy usec, so this function delays for roughly len usec
 */
void
clock_delay_usec(uint16_t len)
{
  DISABLE_INTERRUPTS();
  while(len--) {
    ASM(nop);
  }
  ENABLE_INTERRUPTS();
}
/*---------------------------------------------------------------------------*/
/**
 * Wait for a multiple of ~8 ms (a tick)
 */
void
clock_wait(clock_time_t i)
{
  clock_time_t start;

  start = clock_time();
  while(clock_time() - start < (clock_time_t)i);
}
/*---------------------------------------------------------------------------*/
CCIF clock_time_t
clock_time(void)
{
  return count;
}
/*---------------------------------------------------------------------------*/
CCIF unsigned long
clock_seconds(void)
{
  return seconds;
}

void
clock_init(void)
{
  unsigned char temp;

  /* start with 32k RCOSC and 26MHz HS RCOSC*/
  CLKCON = CLKCONCMD_OSC32K | CLKCONCMD_OSC;

  /* Stay with 32 KHz RC OSC, Change System Clock to 26 MHz
     assure that CLKCON.CLKSPEED is reset to 000 (default is 001)
  */
  CLKCON &= ~(CLKCONCMD_OSC | CLKCONCMD_TICKSPD0 | CLKCONCMD_CLKSPD0);

  /* wait until high speed crystal oscillator is stable */
  while (! (SLEEP & SLEEP_XOSC_STB) );

  /* power down HS RCOSC */
  SLEEP |= SLEEP_OSC_PD;

  /* Tickspeed 406.25 kHz for timers[1-4]

  The power management controller generates
  a tick or enable signal for the peripheral
  timers, thus acting as a prescaler for the
  timers. This is a global clock division for Timer
  1, Timer 2, Timer 3, and Timer 4. The tick
  speed is programmed from 0.203 to 26 MHz
  for CC1110Fx assuming a 26 MHz crystal
  */
  CLKCON |= CLKCONCMD_TICKSPD2 | CLKCONCMD_TICKSPD1;

  /* WORIRQ is the sleep timer interrupt register
   * enable EVENT0 interrupt
   */
  WORIRQ  |= 0x10;

  WORCTRL |= 0x04; // Reset Sleep Timer

  // Alignment of updating EVENT0 to a positive edge on the 32 kHz clock source
  temp = WORTIME0;
  while(temp == WORTIME0); // Wait until a positive 32 kHz edge

  /*
    The time from the CC1110Fx/CC1111Fx enters
    PM2 until the next Event 0 is programmed to
    appear (tSLEEPmin) should be larger than 11.08
    ms when fref is 26 MHz
    SWRS033G Page 126 of 244
   */
  WOREVT0 = 0; // Set EVENT0, low byte
  WOREVT1 = 2; // Set EVENT0, high byte: EVENT0=512, WOR_RES=0 => tevent = 512/32768 sec (15,6 msec)

  STIE = 1; /* IEN0.STIE interrupt enable */
}

/*---------------------------------------------------------------------------*/
/* avoid referencing bits, we don't call code which use them */
#pragma save
#if CC_CONF_OPTIMIZE_STACK_SIZE
#pragma exclude bits
#endif
void
clock_isr(void) __interrupt(ST_VECTOR)
{
  DISABLE_INTERRUPTS();
  ENERGEST_ON(ENERGEST_TYPE_IRQ);

  ++count;

  /* Make sure the CLOCK_CONF_SECOND is a power of two, to ensure
     that the modulo operation below becomes a logical and and not
     an expensive divide. Algorithm from Wikipedia:
     http://en.wikipedia.org/wiki/Power_of_two */
#if (CLOCK_CONF_SECOND & (CLOCK_CONF_SECOND - 1)) != 0
#error CLOCK_CONF_SECOND must be a power of two (i.e., 1, 2, 4, 8, 16, 32, 64, ...).
#error Change CLOCK_CONF_SECOND in contiki-conf.h.
#endif
  if(count % CLOCK_CONF_SECOND == 0) {
    ++seconds;
  }

#if CLOCK_CONF_STACK_FRIENDLY
  sleep_flag = 1;
#else
  if(etimer_pending()
      && (etimer_next_expiration_time() - count - 1) > MAX_TICKS) {
    etimer_request_poll();
  }
#endif

  STIF = 0; /* IRCON.STIF */
  ENERGEST_OFF(ENERGEST_TYPE_IRQ);

  // Clear the SLEEP.MODE bits
  SLEEP &= 0xFC;

  ENABLE_INTERRUPTS();
}
#pragma restore
/*---------------------------------------------------------------------------*/
