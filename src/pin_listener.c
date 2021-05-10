#include <zephyr.h>
#include <device.h>
#include <devicetree.h>
#include <drivers/gpio.h>
#include <stdio.h>

#include "pin_listener.h"

K_MSGQ_DEFINE(eventQueue, sizeof(PinEvent_t), EVENT_QUEUE_SIZE, 4);

#define SW0	DT_ALIAS(sw0)
#define SW1	DT_ALIAS(sw1)

static PinListener pinListeners[] = {
	{
		.label = DT_GPIO_LABEL(SW0, gpios),
		.pin = DT_GPIO_PIN(SW0, gpios),
		.flags = (GPIO_INPUT | DT_GPIO_FLAGS(SW0, gpios)),
		.status = LOW_L,
		.risingEdgeEvent = GO_EVT
    	},
    	{
		.label = DT_GPIO_LABEL(SW1, gpios),
		.pin = DT_GPIO_PIN(SW1, gpios),
		.flags = (GPIO_INPUT | DT_GPIO_FLAGS(SW1, gpios)),
		.status = LOW_L,
		.risingEdgeEvent = STOP_EVT
    	}
};

static PinListenerSet listenerSet = {
	.listeners = pinListeners,
	.num = 2,
	.pollingPeriod_ms = PIN_LISTENER_POLLING_PERIOD_MS
};

static void pollPin(PinListener *listener)
{
	int ret;

	ret = gpio_pin_get(listener->gpio, listener->pin);

	switch (listener->status) {
		case LOW_L:
			if (ret == 1)
				listener->status = RISING_L;
		break;

		case RISING_L:
			if (ret == 1)
				k_msgq_put(&eventQueue, &listener->risingEdgeEvent, K_FOREVER);
			listener->status = (ret ? HIGH_L : LOW_L);
		break;

		case HIGH_L:
			if (ret == 0)
				listener->status = FALLING_L;
		break;

		case FALLING_L:
			listener->status = (ret ? HIGH_L : LOW_L);
		break;
	}
}

static void pollPinsTask()
{
	for (int i = 0; i < listenerSet.num; i++) {
        	listenerSet.listeners[i].gpio = device_get_binding(listenerSet.listeners[i].label);
        	gpio_pin_configure(listenerSet.listeners[i].gpio, listenerSet.listeners[i].pin, (GPIO_INPUT | listenerSet.listeners[i].flags));
    	}
    
    	for (;;) {
        	for (int i = 0; i < listenerSet.num; i++)
            		pollPin(listenerSet.listeners + i);

		k_msleep(listenerSet.pollingPeriod_ms);
	}
}

PinEvent_t getButtonEvent(void)
{
	PinEvent_t pin_evt = NO_EVT;

	k_msgq_get(&eventQueue, &pin_evt, K_NO_WAIT);

	return pin_evt;
}

K_THREAD_DEFINE(pin_listener_tid, PIN_LISTENER_STACK_SIZE, pollPinsTask, NULL, NULL, NULL, PIN_LISTENER_PRIORITY, 0, 100);
