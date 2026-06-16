#include "bsp_gpio.h"
#include "plecs.h"

void bsp_gpio_set_bit(bsp_gpio_table_e num, uint8_t val)
{
    switch (num)
    {
    case AUXOFF:
        plecs_set_output(PLECS_OUTPUT_AUXOFF, val);
        break;
    case RED:
        plecs_set_output(PLECS_OUTPUT_RED, val);
        break;
    case Green:
        plecs_set_output(PLECS_OUTPUT_GREEN, val);
        break;
    case BLED1:
        plecs_set_output(PLECS_OUTPUT_BLED1, val);
        break;
    case BLED2:
        plecs_set_output(PLECS_OUTPUT_BLED2, val);
        break;
    case LEDS:
        plecs_set_output(PLECS_OUTPUT_LEDS, val);
        break;
    case DB2:
        plecs_set_output(PLECS_OUTPUT_DB2, val);
        break;
    case DB1:
        plecs_set_output(PLECS_OUTPUT_DB1, val);
        break;
    case DA2:
        plecs_set_output(PLECS_OUTPUT_DA2, val);
        break;
    case DA1:
        plecs_set_output(PLECS_OUTPUT_DA1, val);
        break;
    case RE:
        plecs_set_output(PLECS_OUTPUT_RE, val);
        break;
    }
}

void bsp_gpio_get_bit(bsp_gpio_table_e num, uint8_t *val)
{
    switch (num)
    {
    case PG_EN:
        *val = plecs_get_input(PLECS_INPUT_PG_EN);
        break;
    }
}
