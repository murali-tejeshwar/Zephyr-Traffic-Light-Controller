#include <zephyr.h>
#include <drivers/gpio.h>
#include <stdio.h>
#include <device.h>
#include <devicetree.h>

#include "global.h"

#define TRAFFIC_LIGHT_SLEEP_MS      100
#define YELLOW_PHASE_MS             5000
#define TRAFFIC_LIGHT_PRIORITY      2
#define TRAFFIC_LIGHT_STACK_SIZE    2500

typedef struct _lednode 
{
	int pin;
	int flags;
} led_t;

volatile state_t global_state = RED_S;

K_MUTEX_DEFINE(state_mutex);

inline state_t getState(void)
{
    	state_t ret; 
    
	k_mutex_lock(&state_mutex, K_FOREVER);
    
	ret = global_state;
    
	k_mutex_unlock(&state_mutex);

    	return ret;
}

inline void setState(state_t newState)
{
    	k_mutex_lock(&state_mutex, K_FOREVER);
    
	global_state = newState;
    
	k_mutex_unlock(&state_mutex);
}

/* Get LED binding info from device tree */
#define LED0_NODE 	DT_ALIAS(led0)
led_t led0 = {
	DT_GPIO_PIN(LED0_NODE, gpios),
	DT_GPIO_FLAGS(LED0_NODE, gpios)
};

#define LED1_NODE 	DT_ALIAS(led1)
led_t led1 = {
	DT_GPIO_PIN(LED1_NODE, gpios),
	DT_GPIO_FLAGS(LED1_NODE, gpios)
};

#define LED2_NODE 	DT_ALIAS(led2)
led_t led2 = {
	DT_GPIO_PIN(LED2_NODE, gpios),
	DT_GPIO_FLAGS(LED2_NODE, gpios)
};

void traffic_light_task(void)
{
    	/* All LEDs are using the same GPIO port */
	const struct device *gpio_port = device_get_binding(DT_GPIO_LABEL(LED0_NODE, gpios));
	state_t local_state;
    	PinEvent_t event;

	gpio_pin_configure(gpio_port, led0.pin, GPIO_OUTPUT_INACTIVE | led0.flags);
	gpio_pin_configure(gpio_port, led1.pin, GPIO_OUTPUT_INACTIVE | led1.flags);
	gpio_pin_configure(gpio_port, led2.pin, GPIO_OUTPUT_INACTIVE | led2.flags);

    	for (;;) {
        	local_state = getState();
        	event = getButtonEvent();
				
        	switch(local_state) {
            		case RED_S:
                		gpio_pin_set(gpio_port, led0.pin, 1);
                		if (event == GO_EVT) {
                    			setState(GREEN_S);
                    			gpio_pin_set(gpio_port, led0.pin, 0);
                		}
            		break;

            		case GREEN_S:
				gpio_pin_set(gpio_port, led2.pin, 1);
                		if (event == STOP_EVT) {
					setState(YELLOW_S);
					gpio_pin_set(gpio_port, led2.pin, 0);
				}
            		break;

            		case YELLOW_S:
                		gpio_pin_set(gpio_port, led1.pin, 1);
                		k_msleep(YELLOW_PHASE_MS);
                		setState(RED_S);
                		gpio_pin_set(gpio_port, led1.pin, 0);
            		break;
        	}

        	k_msleep(TRAFFIC_LIGHT_SLEEP_MS);
    	}
}

K_THREAD_DEFINE(traffic_thread_tid, TRAFFIC_LIGHT_STACK_SIZE, traffic_light_task, NULL, NULL, NULL, TRAFFIC_LIGHT_PRIORITY, 0, 0);
