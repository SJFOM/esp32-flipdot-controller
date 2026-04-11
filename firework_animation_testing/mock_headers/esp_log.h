/**
 * Mock ESP-IDF Logging Headers for Test Harness
 * Provides minimal stubs to allow firework.c to compile standalone.
 */

#ifndef ESP_LOG_H
#define ESP_LOG_H

#include <stdio.h>

#define ESP_LOGI(tag, fmt, ...)                         \
    do                                                  \
    {                                                   \
        printf("[%-8s] " fmt "\n", tag, ##__VA_ARGS__); \
    } while (0)

#define ESP_LOGW(tag, fmt, ...)                                  \
    do                                                           \
    {                                                            \
        printf("[%-8s] WARNING: " fmt "\n", tag, ##__VA_ARGS__); \
    } while (0)

#define ESP_LOGE(tag, fmt, ...)                                         \
    do                                                                  \
    {                                                                   \
        fprintf(stderr, "[%-8s] ERROR: " fmt "\n", tag, ##__VA_ARGS__); \
    } while (0)

#endif // ESP_LOG_H
