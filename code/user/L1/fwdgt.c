/*!
    \file    fwdgt.c
    \brief   watchdog timer implementation
    \version 2024-07-09
*/

#include "fwdgt.h"
#include "bsp_fwdgt.h"
#include "section.h"

/*!
    \brief      initialize watchdog timer
    \param[in]  none
    \param[out] none
    \retval     none
*/
void fwdgt_init(void)
{
    // 调用底层看门狗初始化
    bsp_fwdgt_init();
}

/*!
    \brief      watchdog feed task, runs every 10ms
    \param[in]  none
    \param[out] none
    \retval     none
*/
void fwdgt_feed_task(void)
{
    // 喂狗操作，防止看门狗超时复位
    bsp_fwdgt_feed();
}

// 注册初始化函数
REG_INIT(fwdgt_init)

// 注册定时任务，每10ms执行一次喂狗操作
REG_TASK(10, fwdgt_feed_task)
