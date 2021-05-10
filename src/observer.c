#include <zephyr.h>
#include <device.h>
#include <sys/__assert.h>

#include "global.h"

#define OBSERVER_STACK_SIZE		2500
#define OBSERVER_POLLING_PERIOD_MS	10
#define OBSERVER_PRIORITY		0

void observer_task(void)
{
	uint32_t entry_time, exit_time, time_diff;

	while (true) {
		while (getState() != GREEN_S)
			k_msleep(OBSERVER_POLLING_PERIOD_MS);

		entry_time = k_uptime_get_32();

		while (getState() == GREEN_S)
			k_msleep(OBSERVER_POLLING_PERIOD_MS);

		exit_time = k_uptime_get_32();

		time_diff = (exit_time - entry_time) / 1000;

		__ASSERT(time_diff >= 10, "Error: The Green phase exited in %d seconds", time_diff);
	}
}

K_THREAD_DEFINE(observer_thread_tid, OBSERVER_STACK_SIZE, observer_task, NULL, NULL, NULL, OBSERVER_PRIORITY, 0, 0);
