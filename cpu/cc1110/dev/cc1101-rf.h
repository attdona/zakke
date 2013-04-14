/*
 * Copyright (c) 2013, Piccino Lab (piccino.lab@gmail.com)
 * All rights reserved.
 *
 */

#ifndef CC1101_RF_H_
#define CC1101_RF_H_

/*---------------------------------------------------------------------------*/
#define CC1110_RF_MAX_PACKET_LEN      127
#define CC1110_RF_MIN_PACKET_LEN        4
/*---------------------------------------------------------------------------*/
#define CC1110_RF_CCA_CLEAR             1
#define CC1110_RF_CCA_BUSY              0

#define SFSTXON 0x00
#define SCAL    0x01
#define SRX     0x02
#define STX     0x03
#define SIDLE   0x04

// RFIF register
#define IRQ_TXUNF    0x80
#define IRQ_RXOVF    0x40
#define IRQ_TIMEOUT  0x20
#define IRQ_DONE     0x10 //Packet received/transmitted. Also used to detect underflow/overflow conditions
#define IRQ_CS       0x08
#define IRQ_PQT      0x04
#define IRQ_CCA      0x02
#define IRQ_SFD      0x01 //Start of Frame Delimiter, sync word detected

#define  SLEEP_STATE 0 //0b00000
#define  IDLE_STATE  1 //0b00001

#define  VCOON_MC_STATE 3 //0b00011  //MANCAL
#define  REGON_MC_STATE 4 //0b00100  //MANCAL
#define  MANCAL_STATE   5 //0b00101 // MANCAL

#define  VCOON_STATE 6 //0b00110 // FS_WAKEUP
#define  REGON_STATE 7 //0b00111 // FS_WAKEUP

#define  STARTCAL_STATE 8 //0b01000 // CALIBRATE
#define  ENDCAL_STATE 12 //0b01100 // CALIBRATE

#define  BWBOOST_STATE 9  //0b01001 // SETTLING
#define  FS_LOCK_STATE 10 //0b01010 // SETTLING
#define  IFADCON_STATE 11 //0b01011 // SETTLING

#define  RX_STATE 13 //0b01101 // RX_STATE
#define  RX_END_STATE 14 //0b01110 // RX_STATE
#define  RX_RST_STATE 15 //0b01111 // RX_STATE

#define  TXRX_SETTLING_STATE 16 //0b10000

#define  RX_OVERFLOW_STATE 17 //0b10001

#define  FSTXON_STATE 18 //0b10010

#define  TX_STATE 19 //0b10011
#define  TX_END_STATE 20 //0b10100

#define  RXTX_SETTLING_STATE 21 //0b10101

#define  TX_UNDERFLOW_STATE 22 //0b10110


#endif /* CC1101_RF_H_ */
