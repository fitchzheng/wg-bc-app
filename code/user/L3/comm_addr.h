#ifndef __COMM_ADDR_H__
#define __COMM_ADDR_H__

#define PC_ADDR 0x01
#define LLC_ADDR 0x02
#define PFC_ADDR 0x03
#define APP_ADDR 0x04

#define DC_ADDR LLC_ADDR
#define AC_ADDR PFC_ADDR

#ifndef LOCAL_ADDR
#define LOCAL_ADDR LLC_ADDR
#endif

#endif
