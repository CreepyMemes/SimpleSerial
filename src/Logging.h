#ifndef LOGGING_H
#define LOGGING_H

// Defined TAG for Serial logging
#define SIMPLE_SERIAL_LOG_TAG "simple_serial"

// Serial logging levels
#define SIMPLE_SERIAL_LOG_LEVEL_NONE  0
#define SIMPLE_SERIAL_LOG_LEVEL_ERROR 1
#define SIMPLE_SERIAL_LOG_LEVEL_WARN  2
#define SIMPLE_SERIAL_LOG_LEVEL_INFO  3
#define SIMPLE_SERIAL_LOG_LEVEL_DEBUG 4

// Serial logging toggles
#ifndef SIMPLE_SERIAL_LOG_LEVEL
#define SIMPLE_SERIAL_LOG_LEVEL SIMPLE_SERIAL_LOG_LEVEL_NONE
#endif

#if SIMPLE_SERIAL_LOG_LEVEL >= SIMPLE_SERIAL_LOG_LEVEL_ERROR
#define SS_LOG_E(format, ...) ESP_LOGE(SIMPLE_SERIAL_LOG_TAG, format, ##__VA_ARGS__)
#else
#define SS_LOG_E(format, ...)
#endif

#if SIMPLE_SERIAL_LOG_LEVEL >= SIMPLE_SERIAL_LOG_LEVEL_WARN
#define SS_LOG_W(format, ...) ESP_LOGW(SIMPLE_SERIAL_LOG_TAG, format, ##__VA_ARGS__)
#else
#define SS_LOG_W(format, ...)
#endif

#if SIMPLE_SERIAL_LOG_LEVEL >= SIMPLE_SERIAL_LOG_LEVEL_INFO
#define SS_LOG_I(format, ...) ESP_LOGI(SIMPLE_SERIAL_LOG_TAG, format, ##__VA_ARGS__)
#else
#define SS_LOG_I(format, ...)
#endif

#if SIMPLE_SERIAL_LOG_LEVEL >= SIMPLE_SERIAL_LOG_LEVEL_DEBUG
#define SS_LOG_D(format, ...) ESP_LOGD(SIMPLE_SERIAL_LOG_TAG, format, ##__VA_ARGS__)
#else
#define SS_LOG_D(format, ...)
#endif

#endif // LOGGING_H