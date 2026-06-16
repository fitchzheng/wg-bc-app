#include "led_ctrl.h"
#include "gpio.h"

// Turn on individual LEDs (active-low logic)
static void led_ctrl_set_red(int on)
{
    gpio_set_led2_red(!on);
}

static void led_ctrl_set_green(int on)
{
    gpio_set_led3_green(!on);
}

static void led_ctrl_set_yellow(int on)
{
    gpio_set_led1_green(!on);
}

static void led_ctrl_set_led_off(void)
{
//    gpio_set_red(1);
//    gpio_set_green(1);
//    gpio_set_leds(1);
}

// Control bi-color LED: set RED or GREEN or turn OFF
static void led_ctrl_set_bled_red(void)
{
    gpio_set_lg(0);
    gpio_set_lr(0);
    gpio_set_lg(1);
}

static void led_ctrl_set_bled_green(void)
{
    gpio_set_lg(0);
    gpio_set_lr(0);
    gpio_set_lr(1);
}

static void led_ctrl_set_bled_off(void)
{
    gpio_set_lg(0);
    gpio_set_lr(0);
}

// Public function to control LEDs by mode and value
void led_ctrl_set_led(LED_CTRL_MODE_E mode, uint8_t val)
{
    switch (mode)
    {
    case LED_CTRL_RED:
        led_ctrl_set_red(val);
        break;

    case LED_CTRL_GREEN:
        led_ctrl_set_green(val);
        break;

    case LED_CTRL_YELLOW:
        led_ctrl_set_yellow(val);
        break;

    case LED_CTRL_LED_OFF:
        led_ctrl_set_led_off();

    case LED_CTRL_BLED_RED:
        if (val)
            led_ctrl_set_bled_red();
        else
            led_ctrl_set_bled_off();
        break;

    case LED_CTRL_BLED_GREEN:
        if (val)
            led_ctrl_set_bled_green();
        else
            led_ctrl_set_bled_off();
        break;

    case LED_CTRL_BLED_OFF:
        led_ctrl_set_bled_off();
        break;

    default:
        // Invalid mode, do nothing
        break;
    }
}
