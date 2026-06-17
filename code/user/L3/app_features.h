#ifndef __APP_FEATURES_H__
#define __APP_FEATURES_H__

/* Set to 1 for field diagnostics that need shell/scope/perf.
 * Keep 0 for production APP builds to recover Flash space. */
#ifndef APP_DEBUG_FEATURES
#define APP_DEBUG_FEATURES 0
#endif

/* Release default: remove the lightweight APP event ring and debug read paths.
 * Set to 1 for field EEPROM/factory debug logs. */
#ifndef APP_DEBUG_EVENT_FEATURES
#define APP_DEBUG_EVENT_FEATURES 1
#endif

#endif
