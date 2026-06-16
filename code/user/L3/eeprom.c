#include "eeprom.h"
#include "bsp_iic.h"
#include "stdint.h"

uint8_t Eeprom_Write(uint32_t Address,uint8_t* ndata,uint32_t size)
{
    uint32_t CurrentAddress = 0;
    uint32_t WriteByteSize = 0;
    CurrentAddress = Address;
    WriteByteSize = size;

    if(size == 0)
    {
        return EEPROM_WRITE_SUCCEED;
    }

    while(WriteByteSize > 0)
    {
        if(WriteByteSize >= PAGE_SIZE)
        {
            IICx_Write_Byte(CurrentAddress,(uint8_t *)ndata,PAGE_SIZE);

            CurrentAddress += PAGE_SIZE;
            ndata += PAGE_SIZE;
            WriteByteSize -= PAGE_SIZE;
        }
        else
        {
            IICx_Write_Byte(CurrentAddress,(uint8_t *)ndata,WriteByteSize);
            {
                return EEPROM_TIMEOUT_FAULT;
            }
            CurrentAddress += WriteByteSize;
            ndata += WriteByteSize;
            WriteByteSize -= WriteByteSize;
        }
    } 

    return EEPROM_WRITE_SUCCEED;
}


void Eeprom_Read(uint32_t Address,uint8_t* ndata,uint32_t size)
{
    uint32_t CurrentAddress = 0;
    uint32_t ReadByteSize = 0;

    CurrentAddress = Address;
    ReadByteSize = size;
    while(ReadByteSize > 0)
    {
        if(ReadByteSize >= PAGE_SIZE)
        {
            IICx_Read_Byte(CurrentAddress,(uint8_t *)ndata,PAGE_SIZE);
            CurrentAddress += PAGE_SIZE;
            ndata += PAGE_SIZE;
            ReadByteSize -= PAGE_SIZE;
        }
        else
        {
            IICx_Read_Byte(CurrentAddress,(uint8_t *)ndata,ReadByteSize);
            CurrentAddress += ReadByteSize;
            ndata += ReadByteSize;
            ReadByteSize -= ReadByteSize;
        }
    } 
}
