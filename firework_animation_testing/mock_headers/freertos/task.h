/**
 * Mock FreeRTOS Task Headers for Test Harness
 */

#ifndef FREERTOS_TASK_H
#define FREERTOS_TASK_H

#include <unistd.h>

#define vTaskDelay(ms) usleep((ms) * 1000)

#endif // FREERTOS_TASK_H
