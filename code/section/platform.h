#ifndef __SECTION_PLATFORM_H__
#define __SECTION_PLATFORM_H__

#include <stddef.h>
#include <stdint.h>

#ifdef IS_PLECS
#include "plecs.h"
extern uint32_t plecs_time_1ms;
#define SECTION_SYS_TICK plecs_time_1ms
extern size_t __start_section;
extern size_t __stop_section;
#define SECTION_START __start_section
#define SECTION_STOP __stop_section
#define SYSTEM_RESET
#define AUTO_REG_SECTION __attribute__((__section__("section")))
#define FUNC_RAM
#define RAMFUNC

#elif defined(HC32F334) || defined(IS_HC32)
extern uint32_t systemtime;
#define SECTION_SYS_TICK systemtime
extern uint32_t Load$$SECTION$$Base;
extern uint32_t Load$$SECTION$$Limit;
#define SECTION_START Load$$SECTION$$Base
#define SECTION_STOP Load$$SECTION$$Limit
#define SYSTEM_RESET
#ifndef PLECS_LOG
#define PLECS_LOG(...)
#endif
#define AUTO_REG_SECTION __attribute__((used, __section__(".section")))
#define FUNC_RAM __attribute__((used, __section__(".ramfunc")))
#define RAMFUNC FUNC_RAM

#else
#include "gd32f30x.h"
#include "systick.h"
#define SECTION_SYS_TICK systick_gettime()
extern uint32_t Load$$SECTION$$Base;
extern uint32_t Load$$SECTION$$Limit;
#define SECTION_START Load$$SECTION$$Base
#define SECTION_STOP Load$$SECTION$$Limit
#define SYSTEM_RESET
#ifndef PLECS_LOG
#define PLECS_LOG(...)
#endif
#define AUTO_REG_SECTION __attribute__((used, __section__(".section")))
#define FUNC_RAM
#define RAMFUNC
#endif

#endif /* __SECTION_PLATFORM_H__ */
