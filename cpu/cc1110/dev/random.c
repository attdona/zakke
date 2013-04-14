/*
 * Copyright (c) 2013, Piccino Lab (piccino.lab@gmail.com)
 * All rights reserved.
 *
 */

/*
 * Copyright (c) 2011, George Oikonomou - <oikonomou@users.sourceforge.net>
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
 *         Random number generator routines exploiting the cc2530 hardware
 *         capabilities.
 *
 *         This file overrides core/lib/random.c.
 *
 * \author
 *         George Oikonomou - <oikonomou@users.sourceforge.net>
 *         Attilio Dona'    - <piccino.lab@gmail.com>
 */
#include "cc1110.h"
#include "sfr-bits.h"
#include "dev/cc1101-rf.h"
/*---------------------------------------------------------------------------*/
/**
 * \brief      Generates a new random number using the cc1110 RNG.
 * \return     The random number.
 */
unsigned short
random_rand(void)
{
  /* Clock the RNG LSFR once */
  ADCCON1 |= ADCCON1_RCTRL0;

  return (RNDL | (RNDH << 8));
}
/*---------------------------------------------------------------------------*/
/**
 * \brief      Seed the cc1110 random number generator.
 * \param      seed
 */
void
random_init(unsigned short seed)
{
  /* Make sure the RNG is on */
  ADCCON1 &= ~(ADCCON1_RCTRL1 | ADCCON1_RCTRL0);

  /* High byte first */
  RNDL = seed >> 8;
  RNDL = seed & 0xFF;

 }
