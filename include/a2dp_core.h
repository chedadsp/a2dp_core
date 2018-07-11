#ifndef A2DP_CORE_H
#define A2DP_CORE_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#define A2DP_CORE_SIG_WORK_DISPATCH 0x01

/**
 * @brief     handler for the dispatched work
 */
typedef void (* a2dp_core_cb_t)(uint16_t event, void *param);

/* message to be sent */
typedef struct
{
    uint16_t sig;      /*!< signal to bt_app_task */
    uint16_t event;    /*!< message event id */
    a2dp_core_cb_t cb;       /*!< context switch callback */
    void *param;   /*!< parameter area needs to be last */
} a2dp_core_msg_t;


/**
 * @brief     work dispatcher for the application task
 */
bool a2dp_core_dispatch(
	a2dp_core_cb_t cb,
	uint16_t event,
	void *params,
	int param_len);

void a2dp_core_start(void);
void a2dp_core_stop(void);

#endif
