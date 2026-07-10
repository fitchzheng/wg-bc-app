#ifndef __APP_FEATURES_H__
#define __APP_FEATURES_H__

/* Set to 1 for lightweight field diagnostics.
 * Heavy debug modules below stay independent to protect Flash space. */
#ifndef APP_DEBUG_FEATURES
#define APP_DEBUG_FEATURES 1
#endif

/* Heavy debug modules. Enable only the module required by the current test. */
#ifndef APP_SHELL_FEATURES
#define APP_SHELL_FEATURES 0
#endif

#ifndef APP_SCOPE_FEATURES
#define APP_SCOPE_FEATURES 0
#endif

#ifndef APP_PERF_FEATURES
#define APP_PERF_FEATURES 0
#endif

/* Set to 1 only when USART3 shell/scope debug link is required. */
#ifndef APP_USART3_DEBUG_LINK_FEATURES
#define APP_USART3_DEBUG_LINK_FEATURES 0
#endif

/* Release default: remove the lightweight APP event ring and debug read paths.
 * Set to 1 for field EEPROM/factory debug logs. */
#ifndef APP_DEBUG_EVENT_FEATURES
#define APP_DEBUG_EVENT_FEATURES 0
#endif

/* Set to 1 only when RS485/CAN needs to read back APP debug events. */
#ifndef APP_DEBUG_EVENT_READ_FEATURES
#define APP_DEBUG_EVENT_READ_FEATURES 0
#endif

#if (APP_DEBUG_EVENT_FEATURES != 1)
#undef APP_DEBUG_EVENT_READ_FEATURES
#define APP_DEBUG_EVENT_READ_FEATURES 0
#endif

/* Keep CAN OTA trigger enabled for field firmware upgrade. */
#ifndef APP_CAN_OTA_FEATURES
#define APP_CAN_OTA_FEATURES 1
#endif

#if (APP_SHELL_FEATURES == 1) && (APP_CAN_OTA_FEATURES == 1)
#error "APP_SHELL_FEATURES does not fit in WG-BOOT-APP-RV while APP_CAN_OTA_FEATURES is enabled."
#endif

#endif
