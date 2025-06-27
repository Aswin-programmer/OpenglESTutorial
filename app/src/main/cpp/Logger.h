#pragma once

#include "android/log.h"

#define LOG_INFO(...) __android_log_print(ANDROID_LOG_INFO, "LOG", __VA_ARGS__)
#define LOG_WARN(...) __android_log_print(ANDROID_LOG_WARN, "LOG", __VA_ARGS__)
#define LOG_ERROR(...) __android_log_print(ANDROID_LOG_ERROR, "LOG", __VA_ARGS__)
