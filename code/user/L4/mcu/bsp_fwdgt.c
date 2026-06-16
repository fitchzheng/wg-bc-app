/*!
    \file    bsp_fwdgt.c
    \brief   free watchdog timer driver
    \version 2024-07-09
*/

#include "bsp_fwdgt.h"

/*!
    \brief      configure the free watchdog timer
    \param[in]  none
    \param[out] none
    \retval     none
*/
void bsp_fwdgt_init(void)
{
    /* configure FWDGT counter clock: 40KHz(IRC40K) / 64 = 0.625KHz */
    fwdgt_config(625, FWDGT_PSC_DIV64);
    /* enable FWDGT and set counter reload value, timeout period: 625 * (1/0.625KHz) = 1000ms = 1s */
    fwdgt_enable();
}

/*!
    \brief      feed the free watchdog timer
    \param[in]  none
    \param[out] none
    \retval     none
*/
void bsp_fwdgt_feed(void)
{
    /* reload the FWDGT counter */
    fwdgt_counter_reload();
}
