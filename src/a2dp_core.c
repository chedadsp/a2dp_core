// C includes
#include <stdint.h>
#include <string.h>
#include <stdbool.h>
// FreeRTOS includes
#include "freertos/xtensa_api.h"
#include "freertos/FreeRTOSConfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"
// Logging includes
#include "esp_log.h"
// My includes
#include "a2dp_core.h"

static const char *const A2DP_CORE_TAG = "A2DP_CORE";

static void task_handler(void *arg);
static bool send_msg(const a2dp_core_msg_t *msg);
static void invoke_cb(const a2dp_core_msg_t *msg);

static xQueueHandle task_queue = NULL;
static xTaskHandle task_handle = NULL;

bool a2dp_core_dispatch(
	a2dp_core_cb_t cb,
	uint16_t event,
	void *param,
	int param_len)
{
    ESP_LOGD(
    	A2DP_CORE_TAG,
		"%s event 0x%x, param len %d",
		__func__,
		event,
		param_len);

    a2dp_core_msg_t msg =
    {
    	.sig = A2DP_CORE_SIG_WORK_DISPATCH,
		.event = event,
		.cb = cb,
    };

    if (param_len == 0)
    {
        return send_msg(&msg);
    }
    else if (param && param_len > 0)
    {
        if ((msg.param = malloc(param_len)) != NULL)
        {
            memcpy(msg.param, param, param_len);
            return send_msg(&msg);
        }
    }

    return false;
}

static bool send_msg(const a2dp_core_msg_t *msg)
{
    if (msg == NULL)
        return false;

    if (xQueueSend(task_queue, msg, 10 / portTICK_RATE_MS) != pdTRUE)
    {
        ESP_LOGE(
        	A2DP_CORE_TAG,
			"%s xQueue send failed",
			__func__);

        return false;
    }

    return true;
}

static void invoke_cb(const a2dp_core_msg_t *msg)
{
    if (msg->cb)
        msg->cb(msg->event, msg->param);
}

static void task_handler(void *arg)
{
    for (;;)
    {
    	a2dp_core_msg_t msg;

        if (pdTRUE == xQueueReceive(task_queue, &msg, (portTickType)portMAX_DELAY))
        {
            ESP_LOGD(
            	A2DP_CORE_TAG,
				"%s, sig 0x%x, 0x%x",
				__func__,
				msg.sig,
				msg.event);

            switch (msg.sig)
            {
            case A2DP_CORE_SIG_WORK_DISPATCH:
                invoke_cb(&msg);
                break;

            default:
                ESP_LOGW(
                	A2DP_CORE_TAG,
					"%s, unhandled sig: %d",
					__func__,
					msg.sig);
                break;
            }

            if (msg.param)
                free(msg.param);
        }
    }
}

void a2dp_core_start(void)
{
    task_queue = xQueueCreate(10, sizeof(a2dp_core_msg_t));

    xTaskCreate(
    	task_handler,
		"a2dp_core_task",
		2048,
		NULL,
		configMAX_PRIORITIES - 3,
		&task_handle);
}

void a2dp_core_stop(void)
{
    if (task_handle)
    {
        vTaskDelete(task_handle);
        task_handle = NULL;
    }

    if (task_queue)
    {
        vQueueDelete(task_queue);
        task_queue = NULL;
    }
}
