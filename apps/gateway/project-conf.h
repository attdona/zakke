/*
 * Copyright (c) 2013, Piccino Lab (piccino.lab@gmail.com)
 * All rights reserved.
 *
 */

#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

#define STARTUP_CONF_VERBOSE 0

// contikimac does not fit (40 kb)
//#define NETSTACK_CONF_RDC     contikimac_driver
// also without phase lock optimization the image is too big (36kb)
#define WITH_PHASE_OPTIMIZATION 0

//#define NETSTACK_CONF_RDC cxmac_driver
#define WITH_ENCOUNTER_OPTIMIZATION 0
#define CXMAC_CONF_ANNOUNCEMENTS 0
#define CXMAC_CONF_COMPOWER 0
#define WITH_STREAMING 0
#define WITH_ACK_OPTIMIZATION 0

#define NETSTACK_CONF_RDC nullrdc_noframer_driver

#define NETSTACK_CONF_MAC nullmac_driver

#define CHAMELEON_CONF_MODULE chameleon_raw


// disable energester
#define ENERGEST_CONF_ON 0

//#define LPM_CONF_MODE LPM_MODE_PM1

#endif /* PROJECT_CONF_H_ */
