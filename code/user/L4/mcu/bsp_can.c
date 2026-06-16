#include "bsp_can.h"
#include "gd32f30x.h"
#include "string.h"

void bsp_can_init(void)
{
    can_parameter_struct can_parameter;
    can_filter_parameter_struct can_filter;

    /* enable CAN clock */
    rcu_periph_clock_enable(RCU_CAN0);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_AF);
    gpio_pin_remap_config(GPIO_CAN_PARTIAL_REMAP, ENABLE);
    /* configure CAN0 GPIO */
    gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_8);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_9);

    can_deinit(CAN0);

    can_struct_para_init(CAN_INIT_STRUCT, &can_parameter);
    can_parameter.time_triggered = DISABLE;
    can_parameter.auto_bus_off_recovery = ENABLE;
    can_parameter.auto_wake_up = DISABLE;
    can_parameter.auto_retrans = DISABLE;
    can_parameter.rec_fifo_overwrite = DISABLE;
    can_parameter.trans_fifo_order = ENABLE;
    can_parameter.working_mode = CAN_NORMAL_MODE;
    can_parameter.prescaler = 24;                    // 实际分频 = 5
    can_parameter.time_segment_1 = CAN_BT_BS1_15TQ; // 实际 BS1 = 16 tq
    can_parameter.time_segment_2 = CAN_BT_BS2_4TQ;  // 实际 BS2 = 5 tq
    can_parameter.resync_jump_width = CAN_BT_SJW_1TQ;

    can_init(CAN0, &can_parameter);

    can_filter.filter_number = 0;
    can_filter.filter_mode = CAN_FILTERMODE_MASK;
    can_filter.filter_bits = CAN_FILTERBITS_32BIT;
    can_filter.filter_list_high = 0x0000;
    can_filter.filter_list_low = 0x0000;
    can_filter.filter_mask_high = 0x0000;
    can_filter.filter_mask_low = 0x0000;
    can_filter.filter_fifo_number = CAN_FIFO0;
    can_filter.filter_enable = ENABLE;

    can_filter_init(&can_filter);
}

void bsp_can_tx(uint32_t id, uint8_t *p_data)
{
    can_trasnmit_message_struct tx_msg;

    tx_msg.tx_efid = id;
    tx_msg.tx_ff = CAN_FF_EXTENDED;
    tx_msg.tx_ft = CAN_FT_DATA;
    tx_msg.tx_dlen = 8;
    memcpy(&tx_msg.tx_data[0], p_data, 8);

    // 等待有邮箱空闲，保证连续发送不会丢包
    while (((CAN_TSTAT(CAN0) & CAN_TSTAT_TME0) == 0) &&
           ((CAN_TSTAT(CAN0) & CAN_TSTAT_TME1) == 0) &&
           ((CAN_TSTAT(CAN0) & CAN_TSTAT_TME2) == 0))
    {
        // 等待至少有一个邮箱空闲
    }
    can_message_transmit(CAN0, &tx_msg);
}

int bsp_can_rx(uint32_t *p_raw, uint8_t *p_data)
{
    can_receive_message_struct rx_msg = {0};

    // 判断FIFO是否有数据
    if (can_receive_message_length_get(CAN0, CAN_FIFO0) == 0)
    {
        *p_raw = 0;

        return -1;
    }

    can_message_receive(CAN0, CAN_FIFO0, &rx_msg);

    // 只处理扩展帧且数据长度为8
    if (rx_msg.rx_ff != CAN_FF_EXTENDED || rx_msg.rx_dlen != 8)
    {
        // 主动清空FIFO，防止历史数据残留
        while (can_receive_message_length_get(CAN0, CAN_FIFO0) > 0)
        {
            can_fifo_release(CAN0, CAN_FIFO0);
        }
        memset(p_data, 0, 8);
        return -1;
    }

    // 解析29位ID
    *p_raw = rx_msg.rx_efid;

    // 拷贝8字节数据
    for (int i = 0; i < 8; ++i)
    {
        p_data[i] = rx_msg.rx_data[i];
    }

    // 主动清空FIFO，防止历史数据残留
    while (can_receive_message_length_get(CAN0, CAN_FIFO0) > 0)
    {
        can_fifo_release(CAN0, CAN_FIFO0);
    }
    return 0;
}