#include "section.h"
#include "can_packet.h"
#include "can_rmp.h"
#include "can_comm.h"
#include "can_wg.h"
#include "rvc_address.h"
#include "rvc_message_handler.h"
#include "client_can.h"

static can_packet_t can_rx_packet;

void can_test_1ms_task(void)
{
    // 轮询接收CAN0
    /*if (can_packet_receive(&can_rx_packet) == 0)
    {
        // 判断协议号
        if (can_rx_packet.id.id.protno == PROTNO_RMP)
        {
            can_comm_rmp_dispatch(&can_rx_packet);
        }
        else if (can_rx_packet.id.id.protno == PROTNO_WG)
        {
            can_comm_wg_dispatch(&can_rx_packet);
        }
    }*/
    
    if (can_packet_receive(&can_rx_packet) == 0)
    {
        #if(CAN_ON_OFF == 1)
        // 1. 先交给地址管理器处理（处理 ADDRESS_CLAIM）
        rvc_address_can_rx_callback(can_rx_packet.id.raw, can_rx_packet.data.raw, 8);
        
        // 2. 再交给消息处理器处理（处理其他 DGN）
        rvc_message_handler_process(can_rx_packet.id.raw, can_rx_packet.data.raw, 8);

        // 3. Modbus 桥接处理（处理私有 DGN 0xEF00）
        //rvc_modbus_bridge_process(can_rx_packet.id.raw, can_rx_packet.data.raw, 8);
        #elif(CAN_ON_OFF == 2) 
        Get_CAN_Communications_Content (can_rx_packet.id.raw, can_rx_packet.data.raw, 8);
        #endif
    }
    
}

#if(CAN_ON_OFF != 0)
REG_TASK(1, can_test_1ms_task)
#endif

