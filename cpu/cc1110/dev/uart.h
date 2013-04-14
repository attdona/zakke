#ifndef UART_H
#define UART_H

#include "contiki-conf.h"

#include "cc1110.h"
#include "8051def.h"

/*---------------------------------------------------------------------------*/
/* UART BAUD Rates */
/*
 * Macro to set speed of UART N by setting the UnBAUD SFR to M and the
 * UnGCR SRF to E. See the cc2530 datasheet for possible values of M and E
 */
#define UART_SET_SPEED(N, M, E) do{ U##N##BAUD = M; U##N##GCR = E; } while(0)

/*
 * Sample Values for M and E in the macro above to achieve some common BAUD
 * rates. For more values, see the cc1110Fx/cc1111Fx datasheet
 */
/* 115200 */
#define UART_115_M    34
#define UART_115_E    12
/* 38400 */
#define UART_38_M     131
#define UART_38_E     10
/* 9600 */
#define UART_9_M     131
#define UART_9_E       8

#endif /* UART_H */
