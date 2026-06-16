#include "led.h"
#include "led_ctrl.h"
#include "section.h"
#include "ctrl_app.h"
#include "wg_com_v2.h"
#include "adc.h"
#include "fault.h"
#include "get_com_data.h"

// Track the previous mode to avoid redundant LED updates
static CTRL_MODE_E prev_mode = CTRL_IDLE;

// Initialize all LEDs to OFF
static void led_init(void)
{
    led_ctrl_set_led(LED_CTRL_LED_OFF, 0);
    led_ctrl_set_led(LED_CTRL_BLED_OFF, 0);
    prev_mode = CTRL_IDLE;
}

// Register initialization function
REG_INIT(led_init)

// Main LED update function, called every 1ms
static void led_func(void)
{
    CTRL_MODE_E curr_mode = ctrl_app_get_ctrl_mode();

    // Update LEDs only if mode has changed
    if (curr_mode == prev_mode)
        return;

    prev_mode = curr_mode;

    // Turn off all related LEDs first
    led_ctrl_set_led(LED_CTRL_RED, 0);
    led_ctrl_set_led(LED_CTRL_GREEN, 0);
    led_ctrl_set_led(LED_CTRL_YELLOW, 0);

    // Set the appropriate LED based on mode
    switch (curr_mode)
    {
    case CTRL_IDLE:
        // All LEDs OFF
        break;

    case CTRL_FORWARD:
        led_ctrl_set_led(LED_CTRL_GREEN, 1);
        break;

    case CTRL_BACKWARD:
        led_ctrl_set_led(LED_CTRL_RED, 1);
        break;

    case CTRL_BIDIRECTIONAL:
        led_ctrl_set_led(LED_CTRL_YELLOW, 1);
        break;

    default:
        // Unknown mode — leave all LEDs off
        break;
    }
}

// Handle red LED blinking at 500ms when fault occurs
static void handle_fault_mode(void)
{
    static uint16_t tick_500ms = 0;
    static uint8_t blink_state = 0;

    if (++tick_500ms >= LED_BLINK_500MS_TICKS)
    {
        tick_500ms = 0;
        blink_state ^= 1;
    }

    led_ctrl_set_led(LED_CTRL_BLED_RED, blink_state);
}

static uint32_t ChgCurrLedRednDelay = 0;
static uint32_t ChgCurrLedGreenDelay = 0;
// Forward mode charging logic (constant on)
static void handle_forward_mode(float ilv, float chg_curr, float full_curr)
{
    static uint8_t is_green = 0;
    static uint8_t SwitchStateFlag = 0;
    switch(get_wg_com_v2_data.com_realtime_data.StateCharge)
    {
        case ePRE_CHARGE:
            is_green = 0;
            if(++ChgCurrLedGreenDelay >= LED_BLINK_3S_TICKS)
            {
                (SwitchStateFlag == 0) ? (SwitchStateFlag = 1) : (SwitchStateFlag = 0);
                ChgCurrLedGreenDelay = 0;
            }
            break;
        case eCC_CHARGE:
        case eCV_CHARGE:
            is_green = 0;
            SwitchStateFlag = 1;
            break;
        case eFLOAT_CHARGE:
        case eFULL_CHARGE:
            is_green = 1;
            SwitchStateFlag = 1;
            break;
        case eSTOP_CHARGE:
            is_green = 1;
            if(++ChgCurrLedGreenDelay >= LED_BLINK_3S_TICKS)
            {
                (SwitchStateFlag == 0) ? (SwitchStateFlag = 1) : (SwitchStateFlag = 0);
                ChgCurrLedGreenDelay = 0;
            }
            break;
        case eIDIE_CHARGE:
            is_green = 0;
            if(++ChgCurrLedGreenDelay >= LED_BLINK_1S_TICKS)
            {
                (SwitchStateFlag == 0) ? (SwitchStateFlag = 1) : (SwitchStateFlag = 0);
                ChgCurrLedGreenDelay = 0;
            }
            break;
        case eFAULT_CHARGE:
            is_green = 0;
            if(++ChgCurrLedGreenDelay >= LED_BLINK_500MS_TICKS)
            {
                (SwitchStateFlag == 0) ? (SwitchStateFlag = 1) : (SwitchStateFlag = 0);
                ChgCurrLedGreenDelay = 0;
            }
            break;
        case ePOWER_CHARGE:
            if ((is_green) && (ilv > chg_curr))
            {
                ChgCurrLedRednDelay = 0;
                ChgCurrLedGreenDelay++;
                    
            }
            else if((is_green == 0) && (ilv < full_curr))
            {
                ChgCurrLedGreenDelay = 0;
                ChgCurrLedRednDelay++;
            }

            if(ChgCurrLedGreenDelay >= LED_DELAY_200MS_TICKS)
            {
                is_green = 0;
                ChgCurrLedGreenDelay = 0;
            }
            if(ChgCurrLedRednDelay >= LED_DELAY_200MS_TICKS)
            {
                is_green = 1;
                ChgCurrLedRednDelay = 0;
            }
            SwitchStateFlag = 1;
            break;
    }
    led_ctrl_set_led(is_green ? LED_CTRL_BLED_GREEN : LED_CTRL_BLED_RED, SwitchStateFlag);
}

static void bled_func(void)
{
    CTRL_MODE_E curr_mode = ctrl_app_get_ctrl_mode();
    FAULT_E fault = fault_get_fault();

    if (fault_get_all_fault() != 0)
    {
        handle_fault_mode();
        return;
    }

//    float ihv = fabsf(get_show_ihv_show());
//    float ilv = fabsf(get_show_ilv_show());
    float curr = 0.0f;

    float ilv_chg_led_curr = 0.0f; 
    float ilv_full_led_curr = 0.0f;
    if(charge_state_data.check_state == eADDRS_BACKWARD){
        curr = fabsf(get_show_ihv_show());
        ilv_chg_led_curr = get_wg_com_v2_data.com_param.SetInpChargLedCurr;
        ilv_full_led_curr = get_wg_com_v2_data.com_param.SetInpFullLedCurr;
    }else{
        curr = fabsf(get_show_ilv_show());
        ilv_chg_led_curr = get_wg_com_v2_data.com_param.SetOutChargLedCurr;
        ilv_full_led_curr = get_wg_com_v2_data.com_param.SetOutFullLedCurr;
    }

    handle_forward_mode(curr, ilv_chg_led_curr, ilv_full_led_curr);
}

void led_run(void)
{
    led_func();
    bled_func();
}

REG_TASK(100, led_run)
