#ifndef __APP_FEATURES_H__
#define __APP_FEATURES_H__

/* Set to 1 for lightweight field diagnostics.
 * Heavy debug modules below stay independent to protect Flash space. */
#ifndef APP_DEBUG_FEATURES
#define APP_DEBUG_FEATURES 0
#endif

/* Heavy debug modules. Enable only the module required by the current test. */
#ifndef APP_SHELL_FEATURES
#define APP_SHELL_FEATURES 0
#endif

/* Text command parser is not required by the FRAME parameter read/write and
 * waveform protocol. Keep it off when Flash space is tight. */
#ifndef APP_SHELL_TEXT_CLI_FEATURES
#define APP_SHELL_TEXT_CLI_FEATURES 0
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

/* Set to 1 to output ADC sampling probe pulses from the SCMP ISR:
 * PB0 = ILA/SCMP_A, PB2 = ILB/SCMP_B. These are ISR markers. */
#ifndef APP_ADC_SAMPLE_GPIO_PROBE_FEATURES
#define APP_ADC_SAMPLE_GPIO_PROBE_FEATURES 0
#endif

/* Test only: route SCMP_A/B through the hardware Event Port path to PB0/PB2.
 * If this produces no waveform, PB0/PB2 are not usable for true hardware
 * sample-point marking on this package/mux. */
#ifndef APP_ADC_SAMPLE_EVENT_PORT_PROBE_FEATURES
#define APP_ADC_SAMPLE_EVENT_PORT_PROBE_FEATURES 0
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

/* Parallel preparation layer. CAN/RS485 only negotiate readiness; real current
 * sharing is handled by local droop control. */
#ifndef APP_PARALLEL_MODE_FEATURES
#define APP_PARALLEL_MODE_FEATURES 0
#endif

#ifndef APP_PARALLEL_CAN_FEATURES
#define APP_PARALLEL_CAN_FEATURES 1
#endif

#ifndef APP_PARALLEL_RS485_FEATURES
#define APP_PARALLEL_RS485_FEATURES 1
#endif

#if (APP_PARALLEL_MODE_FEATURES != 1)
#undef APP_PARALLEL_CAN_FEATURES
#define APP_PARALLEL_CAN_FEATURES 0
#undef APP_PARALLEL_RS485_FEATURES
#define APP_PARALLEL_RS485_FEATURES 0
#endif

#if (APP_SHELL_FEATURES == 1) && (APP_CAN_OTA_FEATURES == 1) && (APP_PARALLEL_MODE_FEATURES == 1)
#error "APP_SHELL_FEATURES does not fit in WG-BOOT-APP-RV while APP_CAN_OTA_FEATURES and APP_PARALLEL_MODE_FEATURES are enabled."
#endif

#endif



