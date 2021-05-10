#ifndef _PIN_LISTENER_H
#define _PIN_LISTENER_H

#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include "global.h"

#define PIN_LISTENER_STACK_SIZE             2500
#define PIN_LISTENER_PRIORITY               1
#define EVENT_QUEUE_SIZE                    10
#define PIN_LISTENER_POLLING_PERIOD_MS      10

/* PIN states */
enum States {
	LOW_L = 0, RISING_L = 1, HIGH_L = 2, FALLING_L = 3
};

typedef struct {
	const char *label;              /* label of the gpio port */
    	int pin;                        /* pin the button is connected to */
    	int flags;                      /* gpio flags */
    	const struct device *gpio;      /* pointer to gpio port */
    	PinEvent_t risingEdgeEvent;     /* event for rising edge detection */
    	PinEvent_t fallingEdgeEvent;    /* event for falling edge detection */
    	uint8_t status;                 /* used internally */
} PinListener;

typedef struct {
    	PinListener *listeners;         /* array of all the pins to listen to */
    	int num;                        /* number of pins/buttons */
    	uint32_t pollingPeriod_ms;      /* time between two polls */
} PinListenerSet;

#endif /* _PIN_LISTENER_H */
