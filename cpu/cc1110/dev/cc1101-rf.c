/*
 * Copyright (c) 2013, Piccino Lab (piccino.lab@gmail.com)
 * All rights reserved.
 *
 */

/**
 * \file
 *         Implementation of the radio driver for the cc1110.
 *
 *
 *
 * \author
 *         Attilio Dona' - <piccino.lab@gmail.com>
 */


#include "contiki.h"
#include "dev/radio.h"
#include "dev/cc1101-rf.h"
#include "cc1110.h"
#include "sfr-bits.h"
#include "sys/clock.h"
#include "sys/rtimer.h"
#include "dev/dma.h"
#include "net/packetbuf.h"
#include "net/rime/rimestats.h"
#include "net/rime/rimeaddr.h"
#include "net/netstack.h"

#include <string.h>

/*---------------------------------------------------------------------------*/
#define CHECKSUM_LEN 2
/*---------------------------------------------------------------------------*/

#if RF_CONF_LEDS
#define RF_LEDS RF_CONF_LEDS
#else
#define RF_LEDS 0
#endif

#if RF_LEDS
#include "dev/leds.h"
#define RF_RX_LED_ON()		leds_on(LEDS_RED);
#define RF_RX_LED_OFF()		leds_off(LEDS_RED);
#define RF_TX_LED_ON()		leds_on(LEDS_GREEN);
#define RF_TX_LED_OFF()		leds_off(LEDS_GREEN);
#else
#define RF_RX_LED_ON()
#define RF_RX_LED_OFF()
#define RF_TX_LED_ON()
#define RF_TX_LED_OFF()
#endif
/*---------------------------------------------------------------------------*/
#define DEBUG 0
#if DEBUG
#include <stdio.h>
#include "debug.h"
#define PUTSTRING(...) putstring(__VA_ARGS__)
#define PUTHEX(...) puthex(__VA_ARGS__)
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PUTSTRING(...)
#define PUTHEX(...)
#define PRINTF(...)
#endif

/*---------------------------------------------------------------------------*/
/* Local RF Flags */
#define RX_ACTIVE  0x80
#define WAS_OFF    0x10
#define RF_ON      0x01


/* Bit Masks for the last byte in the RX FIFO */
#define CRC_BIT_MASK 0x80
#define LQI_BIT_MASK 0x7F
/* RSSI Offset */
#define RSSI_OFFSET    73

/* 192 ms, radio off -> on interval */
#define ONOFF_TIME                    RTIMER_ARCH_SECOND / 3125


/*---------------------------------------------------------------------------*/
static uint8_t CC_AT_DATA rf_flags;
static uint8_t CC_AT_DATA packet_pending;

static int on(void); /* prepare() needs our prototype */
static int off(void); /* transmit() needs our prototype */
static int channel_clear(void); /* transmit() needs our prototype */
/*---------------------------------------------------------------------------*/


static uint8_t radiobuff[(PACKETBUF_SIZE + PACKETBUF_HDR_SIZE)];


/*---------------------------------------------------------------------------*/
/* Netstack API radio driver functions */
/*---------------------------------------------------------------------------*/
static int
init(void)
{
    // dma packet buffer data pointer
    void *packetptr;

    PUTSTRING("RF: Init\n");

    if(rf_flags & RF_ON)
    {
        return 0;
    }

    /**
     * CC1101 configuration registers - Default values extracted from SmartRF Studio
     *
     * Configuration:
     *
     * Deviation = 20.629883
     * Base frequency = 868.299866
     * Carrier frequency = 868.299866
     * Channel number = 0
     * Modulated = true
     * Modulation format = GFSK
     * Manchester enable = false
     * Data whitening = off
     * Sync word qualifier mode = 30/32 sync word bits detected
     * Preamble count = 4
     * Channel spacing = 199.951172
     * Carrier frequency = 867.999939
     * Data rate = 38.3835 Kbps
     * RX filter BW = 101.562500
     * Data format = Normal mode
     * Length config = Variable packet length mode. Packet length configured by the first byte after sync word
     * CRC enable = true
     * Packet length = 255
     * Device address = 1
     * Address config = Enable address check
     * Append status = Append two status bytes to the payload of the packet. The status bytes contain RSSI and
     * LQI values, as well as CRC OK
     * CRC autoflush = false
     * PA ramping = false
     * TX power = 7
     * Settings optimized for high sensitivity
     */
    PKTCTRL0  = 0x05; // packet automation control
    FSCTRL1   = 0x06; // frequency synthesizer control

#ifdef FREQ_868MHZ
    FREQ2     = 0x21; // frequency control word, high byte
    FREQ1     = 0x62; // frequency control word, middle byte
    FREQ0     = 0x76; // frequency control word, low byte
#else
    // Base frequency = 901.999969
    FREQ2     = 0x22; // frequency control word, high byte
    FREQ1     = 0xB1; // frequency control word, middle byte
    FREQ0     = 0x3B; // frequency control word, low byte
#endif
    MDMCFG4   = 0xCA; // modem configuration
    MDMCFG3   = 0x83; // modem configuration
    MDMCFG2   = 0x13; // modem configuration
    DEVIATN   = 0x35; // modem deviation setting
    MCSM0     = 0x18; // main radio control state machine configuration
    FOCCFG    = 0x16; // frequency offset compensation configuration
    AGCCTRL2  = 0x43; // agc control
    FSCAL3    = 0xE9; // frequency synthesizer calibration
    FSCAL2    = 0x2A; // frequency synthesizer calibration
    FSCAL1    = 0x00; // frequency synthesizer calibration
    FSCAL0    = 0x1F; // frequency synthesizer calibration
    TEST1     = 0x31; // various test settings
    TEST0     = 0x09; // various test settings
    //PA_TABLE0 = 0xCB; // pa power setting 0: +7 dbm
    PA_TABLE0 = 0x8F; // pa power setting 0: -5 dbm


    /* TEST1	Various Test Settings
     Always set this register to 0x31 when being in TX. At low data rates, the
     sensitivity can be improved by changing it to 0x35 in RX.
     (MDMCFG2.DEM_DCFILT_OFF should be 0).
    */

    //SYNC1 = 0xD3; /* Sync Word, High Byte */
    //SYNC0 = 0x91; /* Sync Word, Low Byte */

    SYNC1 = 0xB5; /* Sync Word, High Byte */
    SYNC0 = 0x47; /* Sync Word, Low Byte */
    PKTLEN = 0xFF; /* Packet Length */


    /* The reset default value
     * Packet Automation Control: PQT == 0 => sync word always accepted
     * APPEND_STATUS = 1
     * no hw address check
     */
    //PKTCTRL1 = 0x04;

    //ADDR = 0x00; /* ADDR		Device Address */

    /* MCSM2	Main Radio Control State Machine Configuration

       UNUSED RX_TIME_RSSI RX_TIME_QUAL RX_TIME[2:0]
         000     0              0          111

       RX_TIME[2:0] Timeout for sync word search in RX. The timeout is relative to the
                    programmed tEvent0.
                    111: Until end of packet
    */
    //MCSM2 = 0x07; // the default value

    /* MCSM1	Main Radio Control State Machine Configuration

       UNUSED CCA_MODE RXOFF_MODE TXOFF_MODE
         00     11         11          00

        CCA_MODE[1:0] Selects CCA_MODE; Reflected in CCA signal
                      11: If RSSI below threshold unless currently receiving a packet

        RXOFF_MODE[1:0] Select what should happen (next state) when a packet has been received
                        11: Stay in RX

        TXOFF_MODE[1:0] Select what should happen (next state) when a packet has been sent (TX)
                        11: RX
    */
    MCSM1 = 0x3F;

    /* MCSM0	Main Radio Control State Machine Configuration

       UNUSED  FS_AUTOCAL[1:0]  RESERVD  CLOSE_IN_RX[1:0]
         00      11             10        00

       FS_AUTOCAL=11 Calibration mode: Every 4th time when going from RX or TX to IDLE automatically
    */
    //MCSM0 = 0x38; // calibrate Every 4th time when going from RX or TX to IDLE automatically


    /*
     * GDO0_CFG[5:0] : 001001 CCA high when RSSI level is below threshold on P1_5
     */
    //IOCFG0 = 0x09;

    /*
    configure the channel for RADIO
    */
    dma_conf[DMA_RADIO_CHANNEL].src_h = 0xDF;  //SFRX(X_RFD, 0xDFD9);
    dma_conf[DMA_RADIO_CHANNEL].src_l = 0xD9;

    dma_conf[DMA_RADIO_CHANNEL].dst_h = ((uint16_t)radiobuff)>>8;
    dma_conf[DMA_RADIO_CHANNEL].dst_l = (uint8_t)radiobuff;

    /*
    packetptr = packetbuf_dataptr();
    dma_conf[DMA_RADIO_CHANNEL].dst_h = ((uint16_t)packetptr)>>8;
    dma_conf[DMA_RADIO_CHANNEL].dst_l = (uint8_t)packetptr;
    */

    dma_conf[DMA_RADIO_CHANNEL].len_l = DMA_VLEN_N3;
    dma_conf[DMA_RADIO_CHANNEL].len_h = PACKETBUF_SIZE;
    dma_conf[DMA_RADIO_CHANNEL].wtt = DMA_SINGLE | DMA_T_RADIO;
    dma_conf[DMA_RADIO_CHANNEL].inc_prio = DMA_DST_INC_1 | DMA_IRQ_MASK_ENABLE | DMA_PRIO_HIGH;

    //memcpy(&dma_conf[DMA_RADIO_CHANNEL].dst_h, &packetbuf_dataptr(), 2);

    // enable DMA interrupt
    IEN1 |= DMAIE;

    RF_TX_LED_OFF();
    RF_RX_LED_OFF();

    // enable RFIF interrupt
    //RFIM = IM_DONE|IM_RXOVF;
    //IEN2 |= IEN2_RFIE;

    rf_flags |= RF_ON;

    PUTSTRING("RF: Init DONE\n");

    return 1;
}

/*
 prepare not needed for this driver
*/
static int
prepare(const void *payload, unsigned short payload_len)
{
    payload;
    payload_len;
    return 0;
}

/*
 *
 */
static int
transmit(unsigned short transmit_len)
{
    uint8_t counter;
    int ret = RADIO_TX_ERR;
    rtimer_clock_t t0;
    uint8_t *dataptr;

    PRINTF("TX: %d\n", rf_flags);

    if(!(rf_flags & RX_ACTIVE))
    {
        t0 = RTIMER_NOW();
        on();
        rf_flags |= WAS_OFF;
        while(RTIMER_CLOCK_LT(RTIMER_NOW(), t0 + ONOFF_TIME));
    }

    if(channel_clear() == CC1110_RF_CCA_BUSY)
    {
        RIMESTATS_ADD(contentiondrop);
        return RADIO_TX_COLLISION;
    }

    //PRINTF("setting TX\n");

    // disable DMA channel 0 (RX)
    DMA_ABORT(0);

    /* Start the transmission */
    RF_TX_LED_ON();
    ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
    ENERGEST_ON(ENERGEST_TYPE_TRANSMIT);

    RFST = STX;

    // send the packet
    dataptr = packetbuf_hdrptr();

    /* SWRS033G
     * The packet length is defined as the
    * payload data, excluding the length byte and
    * the optional CRC
    */

    while(MARCSTATE != TX_STATE) {}

    PRINTF("mcs: %2X, len:%d, RF:%d\n", MARCSTATE, transmit_len, RFIF);

    // Print the crystal status
    PRINTF("dptr: %4X\n", dataptr);

    for(counter=0; counter<transmit_len; counter++)
    {
        PRINTF("[%d]", dataptr[counter]);
    }
    while(!RFTXRXIF);
    RFTXRXIF = 0;
    RFD = transmit_len;
    for(counter=0; counter<transmit_len; counter++)
    {
        while(!RFTXRXIF); // wait radio to be TX ready
        RFTXRXIF = 0;
        RFD = dataptr[counter];

    }
    while (!(RFIF & IRQ_DONE)) {}

    PRINTF("\nTX OK:%d\n", RFIF);

    ret = RADIO_TX_OK;

    ENERGEST_OFF(ENERGEST_TYPE_TRANSMIT);
    ENERGEST_ON(ENERGEST_TYPE_LISTEN);

    if(rf_flags & WAS_OFF)
    {
        off();
    }
    else
    {
        // enable DMA channel 0 (RX)
        DMA_ARM(0);
    }

    RIMESTATS_ADD(lltx);

    RF_TX_LED_OFF();

    /* OK, sent. We are now ready to send more */
    return ret;
}

/*---------------------------------------------------------------------------*/
static int
send(void *payload, unsigned short payload_len)
{
    //prepare(payload, payload_len);
    return transmit(payload_len);
}

/*
 * Read the received packet from the radio.
 *
 *
 */
read(void *buf, unsigned short bufsize)
{

    uint8_t pktlen;

    // when a packet is read it is no more pending, so another packet may be processed by DMA
    packet_pending = 0;
    /*
      pktlen = *(uint8_t*)packetbuf_dataptr();
      if(packetbuf_hdrreduce(sizeof(pktlen)) == 0) {
        PUTSTRING("chameleon-raw: too short packet\n");
        return 0;
      }
    */

    pktlen = radiobuff[0];

    memcpy(buf,radiobuff+1,pktlen);

    // TODO: get the status bytes and set the packet attributes
    // PACKETBUF_ATTR_LINK_QUALITY and PACKETBUF_ATTR_RSSI

    // (re)ARM the channel for the next packet
    DMA_ARM(0);

    return pktlen;
}

#if 0
static int
read(void *buf, unsigned short bufsize)
{
    uint8_t i;
    uint8_t len;
    uint8_t crc_corr;
    int8_t rssi;

    PUTSTRING("RF: Read\n");

    /* Check the length */
    while(!RFTXRXIF); // wait packet ready
    len = RFD;

    /* Check for validity */
    if(len > CC1110_RF_MAX_PACKET_LEN)
    {
        /* if here, the best assumption is that we must be out of sync. */
        PUTSTRING("RF: bad sync\n");

        RIMESTATS_ADD(badsynch);
        //CC2530_CSP_ISFLUSHRX();
        return 0;
    }

    if(len <= CC1110_RF_MIN_PACKET_LEN)
    {
        PUTSTRING("RF: too short\n");

        RIMESTATS_ADD(tooshort);
        //CC2530_CSP_ISFLUSHRX();
        return 0;
    }

    if(len - CHECKSUM_LEN > bufsize)
    {
        PUTSTRING("RF: too long\n");

        RIMESTATS_ADD(toolong);
        //CC2530_CSP_ISFLUSHRX();
        return 0;
    }

#if CC1110_RF_CONF_HEXDUMP
    /* If we reach here, chances are the FIFO is holding a valid frame */
    io_arch_writeb(magic[0]);
    io_arch_writeb(magic[1]);
    io_arch_writeb(magic[2]);
    io_arch_writeb(magic[3]);
    io_arch_writeb(len);
#endif

    RF_RX_LED_ON();

    PUTSTRING("RF: read (0x");
    PUTHEX(len);
    PUTSTRING(" bytes) = ");
    len -= CHECKSUM_LEN;
    for(i = 0; i < len; ++i)
    {
        while(!RFTXRXIF); // wait byte packet ready to read
        ((unsigned char *)(buf))[i] = RFD;
#if CC1110_RF_CONF_HEXDUMP
        io_arch_writeb(((unsigned char *)(buf))[i]);
#endif
        PUTHEX(((unsigned char *)(buf))[i]);
    }
    PUTSTRING("\n");

    /* Read the RSSI and CRC/Corr bytes */
    while(!RFTXRXIF); // wait packet ready
    rssi = ((int8_t) RFD) - RSSI_OFFSET;
    while(!RFTXRXIF); // wait packet ready
    crc_corr = RFD;

#if CC1110_RF_CONF_HEXDUMP
    io_arch_writeb(rssi);
    io_arch_writeb(crc_corr);
    io_arch_flush();
#endif

    /* MS bit CRC OK/Not OK, 7 LS Bits, Correlation value */
    if(crc_corr & CRC_BIT_MASK)
    {
        packetbuf_set_attr(PACKETBUF_ATTR_RSSI, rssi);
        packetbuf_set_attr(PACKETBUF_ATTR_LINK_QUALITY, crc_corr & LQI_BIT_MASK);
        RIMESTATS_ADD(llrx);
    }
    else
    {
        RIMESTATS_ADD(badcrc);
        //CC2530_CSP_ISFLUSHRX();
        RF_RX_LED_OFF();
        return 0;
    }

    RF_RX_LED_OFF();

    return (len);
}
#endif

/*---------------------------------------------------------------------------*/
static int
channel_clear(void)
{
#if 0
    if(P1_5)
    {
        return CC1110_RF_CCA_CLEAR;
    }
    return CC1110_RF_CCA_BUSY;
#endif
    return (PKTSTATUS & PKTSTATUS_CCA);
}

/*---------------------------------------------------------------------------*/
static int
receiving_packet(void)
{
    PUTSTRING("RF: Receiving\n");

    /*
     * SFD high while transmitting and receiving.
     * TX_ACTIVE high only when transmitting
     *
     * FSMSTAT1 & (TX_ACTIVE | SFD) == SFD <=> receiving
     */
    return (PKTSTATUS & PKTSTATUS_SFD);
}

/*---------------------------------------------------------------------------*/
static int
pending_packet(void)
{
    return packet_pending;
}

/*---------------------------------------------------------------------------*/
static int on(void)
{
    PUTSTRING("RF: switching RADIO on\n");

    if (!(rf_flags & RX_ACTIVE))
    {
        PRINTF("mcst-on-0: %X\n", MARCSTATE);
        switch (MARCSTATE)
        {
        case IDLE_STATE:
            RFST = SRX;
            break;
        case RX_STATE:
            break;
        default:
            /* if in an unexpected state reset the radio
               and go to RX
            */
            RFST = SIDLE;
            while(MARCSTATE!=IDLE_STATE);
            RFST = SRX;
        }

        // wait RX state
        //while(MARCSTATE!=RX_STATE);

        // ARM the DMA radio channel
        DMA_ARM(0);

        rf_flags |= RX_ACTIVE;
    }

    PRINTF("mcst-on-1: %X\n", MARCSTATE);
    ENERGEST_ON(ENERGEST_TYPE_LISTEN);
    return 1;
}

/*
 * Radio is switched off
 */
static int
off(void)
{
    RFST = SIDLE;
    rf_flags &= ~RX_ACTIVE;

    // Abort the DMA radio channel
    DMA_ABORT(0);

    ENERGEST_OFF(ENERGEST_TYPE_LISTEN);
    return 1;
}

// For what I understand from the data sheet this is invoked if and only if a complete packet is received
void rf_dma_callback_isr(void)
{
    packet_pending = 1;
}

#if 0
/* avoid referencing bits since we're not using them */
#pragma save
#if CC_CONF_OPTIMIZE_STACK_SIZE
#pragma exclude bits
#endif
void
rfif_isr(void) __interrupt(RF_VECTOR)
{
    ENERGEST_ON(ENERGEST_TYPE_IRQ);

    if(RFIF & IRQ_DONE)
    {

    }

    ENERGEST_OFF(ENERGEST_TYPE_IRQ);
}
#pragma restore
#endif

/*---------------------------------------------------------------------------*/
const struct radio_driver cc1101_rf_driver =
{
    init,
    prepare,
    transmit,
    send,
    read,
    channel_clear,
    receiving_packet,
    pending_packet,
    on,
    off,
};
/*---------------------------------------------------------------------------*/
