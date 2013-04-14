/**
 * \file
 *         A very simple Contiki application showing how Contiki programs look
 * \author
 *         Attilio Dona' - <piccino.lab@gmail.com>
 */

#include "contiki.h"
#include "dev/button-sensor.h"
#include "dev/leds.h"
#include "net/rime.h"

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else
#define PRINTF(...)
#endif

static void
abc_recv(struct abc_conn *c)
{
  PRINTF("abc message received '%s'\n", (char *)packetbuf_dataptr());
}
static const struct abc_callbacks abc_call = {abc_recv};
static struct abc_conn abc;

/*---------------------------------------------------------------------------*/
PROCESS(hello_world_process, "Hello world process");
AUTOSTART_PROCESSES(&hello_world_process);
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(hello_world_process, ev, data)
{
  struct sensors_sensor *sensor;

  PROCESS_EXITHANDLER(abc_close(&abc);)

  PROCESS_BEGIN();

  abc_open(&abc, 128, &abc_call);

  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(ev == sensors_event);
    
    sensor = (struct sensors_sensor *)data;
    if(sensor == &button1) {
      leds_toggle(LEDS_GREEN);
      packetbuf_copyfrom("Hello", 6);
      abc_send(&abc);
    }
    else if(sensor == &button2) {
      leds_toggle(LEDS_RED);
    }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
