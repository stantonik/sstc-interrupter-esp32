/**
 * \file logging.h
 * \brief 
 *
 * \author Stanley Arnaud
 * \date 2026-06-05
 */

#ifndef LOGGING_H
#define LOGGING_H

// -----------------------------------------------------------------------------
// Includes
// -----------------------------------------------------------------------------
#include <stdint.h>
#include "esp_log.h"

#ifdef __cplusplus
extern "C"
{
#endif /* __cplusplus */

/* 
 * Map our log levels to ESP-IDF's native log levels.
 * This ensures compatibility with menuconfig and runtime controls.
 */
#define LOG_LEVEL_NONE    ESP_LOG_NONE
#define LOG_LEVEL_ERROR   ESP_LOG_ERROR
#define LOG_LEVEL_WARN    ESP_LOG_WARN
#define LOG_LEVEL_INFO    ESP_LOG_INFO
#define LOG_LEVEL_DEBUG   ESP_LOG_DEBUG
#define LOG_LEVEL_VERBOSE ESP_LOG_VERBOSE

/* 
 * Compile-time Log Level
 * Logs below this level will be completely stripped from the binary (zero overhead).
 * You can override this in your CMakeLists.txt or compiler flags if needed.
 */
#ifndef COMPILE_TIME_LOG_LEVEL
#define COMPILE_TIME_LOG_LEVEL LOG_LEVEL_DEBUG
#endif

/* 
 * Logging Macros
 * These map directly to ESP-IDF's native macros. 
 * This prevents the stack bloat you experienced with custom vsnprintf implementations.
 * The ##__VA_ARGS__ handles cases where no extra arguments are passed.
 */
#if COMPILE_TIME_LOG_LEVEL >= LOG_LEVEL_ERROR
#define LOG_ERROR(tag, format, ...) ESP_LOGE(tag, format, ##__VA_ARGS__)
#else
#define LOG_ERROR(tag, format, ...) (void)0
#endif

#if COMPILE_TIME_LOG_LEVEL >= LOG_LEVEL_WARN
#define LOG_WARN(tag, format, ...) ESP_LOGW(tag, format, ##__VA_ARGS__)
#else
#define LOG_WARN(tag, format, ...) (void)0
#endif

#if COMPILE_TIME_LOG_LEVEL >= LOG_LEVEL_INFO
#define LOG_INFO(tag, format, ...) ESP_LOGI(tag, format, ##__VA_ARGS__)
#else
#define LOG_INFO(tag, format, ...) (void)0
#endif

#if COMPILE_TIME_LOG_LEVEL >= LOG_LEVEL_DEBUG
#define LOG_DEBUG(tag, format, ...) ESP_LOGI(tag, format, ##__VA_ARGS__)
#else
#define LOG_DEBUG(tag, format, ...) (void)0
#endif

#if COMPILE_TIME_LOG_LEVEL >= LOG_LEVEL_VERBOSE
#define LOG_VERBOSE(tag, format, ...) ESP_LOGV(tag, format, ##__VA_ARGS__)
#else
#define LOG_VERBOSE(tag, format, ...) (void)0
#endif

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* !LOGGING_H */
