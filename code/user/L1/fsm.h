#ifndef __FSM_H
#define __FSM_H

#include "stdint.h"

uint8_t fsm_get_fsm_state_is_fault(void);

uint8_t fsm_direction_soft_stop_is_pending(void);

#endif
