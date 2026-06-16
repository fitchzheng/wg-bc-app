#include "bsp_iic.h"
#include "gd32f30x.h"

void bsp_iic_init(void)
{
    rcu_periph_clock_enable(RCU_GPIO_I2C);
    rcu_periph_clock_enable(RCU_I2C);
    
    gpio_init(I2C_SCL_PORT, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, I2C_SCL_PIN);
    gpio_init(I2C_SDA_PORT, GPIO_MODE_AF_OD, GPIO_OSPEED_50MHZ, I2C_SDA_PIN);

    i2c_clock_config(I2CX, I2C_SPEED, I2C_DTCY_2);
    i2c_mode_addr_config(I2CX, I2C_I2CMODE_ENABLE, I2C_ADDFORMAT_7BITS, I2CX_SLAVE_ADDRESS7);
    i2c_enable(I2CX);
    i2c_ack_config(I2CX, I2C_ACK_ENABLE);
}

uint8_t IICx_Write_Byte(uint32_t i2c_periph,uint32_t Address,uint8_t* ndata,uint32_t size,uint32_t Timeout)
{
	uint32_t Timeout_t=0;
	uint8_t i=0;
	
	Timeout_t = Timeout;
	/* wait until I2C bus is idle */
	while(i2c_flag_get(i2c_periph, I2C_FLAG_I2CBSY))
	{
		if(Timeout_t > 0)
        {
            Timeout_t--;
        }
		else
        {
            return Write_TIMEOUT_FAULT;
        }
	}
	
	/* send a start condition to I2C bus */
	i2c_start_on_bus(i2c_periph);
	
	Timeout_t = Timeout;
	/* wait until SBSEND bit is set */
	while(!i2c_flag_get(i2c_periph, I2C_FLAG_SBSEND))
	{
		if(Timeout_t > 0)
        {
            Timeout_t--;
        }
		else
        {
            return Write_TIMEOUT_FAULT;
        }
	}
	
	/* send slave address to I2C bus*/
	i2c_master_addressing(i2c_periph, I2CX_SLAVE_ADDRESS7, I2C_TRANSMITTER);
	
	Timeout_t = Timeout;
	/* wait until ADDSEND bit is set*/
	while(!i2c_flag_get(i2c_periph, I2C_FLAG_ADDSEND))
	{
		if(Timeout_t > 0)
        {
            Timeout_t--;
        }
		else
        {            
            return Write_TIMEOUT_FAULT;
        }
	}
	/* clear ADDSEND bit */
	i2c_flag_clear(i2c_periph, I2C_FLAG_ADDSEND);	
		
	/* send a data byte */
	i2c_data_transmit(i2c_periph,(uint8_t)(Address>>8));
	
	Timeout_t = Timeout;
	/* wait until the transmission data register is empty*/
	while(!i2c_flag_get(i2c_periph, I2C_FLAG_TBE))
	{
		if(Timeout_t > 0)
        {
            Timeout_t--;
        }
		else
        {				
            return Write_TIMEOUT_FAULT;
        }
	}
    
	/* send a data byte */
	i2c_data_transmit(i2c_periph,(uint8_t)(Address&0xff));
	
	Timeout_t = Timeout;
	/* wait until the transmission data register is empty*/
	while(!i2c_flag_get(i2c_periph, I2C_FLAG_TBE))
	{
		if(Timeout_t > 0)
        {
            Timeout_t--;
        }
		else
        {				
            return Write_TIMEOUT_FAULT;
        }
	}
    
	for(i=0;i<size;i++)
	{
		/* send a data byte */
		i2c_data_transmit(i2c_periph, (*ndata));
		
		Timeout_t = Timeout;
		/* wait until the transmission data register is empty*/
		while(!i2c_flag_get(i2c_periph, I2C_FLAG_TBE))
		{
			if(Timeout_t > 0)
            {
                Timeout_t--;
            }
			else
            {
                return Write_TIMEOUT_FAULT;
            }
		}
		
		ndata++;
	}
	/* send a stop condition to I2C bus*/
	i2c_stop_on_bus(i2c_periph);
	
	Timeout_t = Timeout;
	/* wait until stop condition generate */ 
	while(I2C_CTL0(i2c_periph)&0x0200)
	{
		if(Timeout_t > 0)
        {
            Timeout_t--;
        }
		else
        {           
            return Write_TIMEOUT_FAULT;
        }
	}
    for(uint16_t i = 0;i < 50000;i++)  // 延时1.9~3ms才能写入下一页数据
    {
        __NOP();
        __NOP();
    }
    return Write_SUCCEED;
}

uint8_t IICx_Read_Byte(uint32_t i2c_periph,uint32_t Address,uint8_t* ndata,uint32_t size,uint32_t Timeout)
{
	uint32_t Timeout_t=0;
	uint8_t i=0;
	
	/******************************************************/
	/*	Send Slave address and Specified Register Address */
	/******************************************************/
	
	Timeout_t = Timeout;
	/* wait until I2C bus is idle */
	while(i2c_flag_get(i2c_periph, I2C_FLAG_I2CBSY))
	{
		if(Timeout_t > 0)
        {
            Timeout_t--;
        }
		else
        {
            return Read_TIMEOUT_FAULT;
        }
	}
	/* send a start condition to I2C bus */
	i2c_start_on_bus(i2c_periph);
	
	Timeout_t = Timeout;
	/* wait until SBSEND bit is set */
	while(!i2c_flag_get(i2c_periph, I2C_FLAG_SBSEND))
	{
		if(Timeout_t > 0)
        {
            Timeout_t--;
        }
		else
        {				
            return Read_TIMEOUT_FAULT;
        }
	}
	
	/* send slave address to I2C bus*/
	i2c_master_addressing(i2c_periph, I2CX_SLAVE_ADDRESS7, I2C_TRANSMITTER);//I2C_RECEIVER		I2C_TRANSMITTER
	
	Timeout_t = Timeout;
	/* wait until ADDSEND bit is set*/
	while(!i2c_flag_get(i2c_periph, I2C_FLAG_ADDSEND))
	{
		if(Timeout_t > 0)
        {
            Timeout_t--;
        }
		else
        {
            return Read_TIMEOUT_FAULT;
        }
	}
	/* clear ADDSEND bit */
	i2c_flag_clear(i2c_periph, I2C_FLAG_ADDSEND);	

	/* send a data byte */
	i2c_data_transmit(i2c_periph,(uint8_t)(Address>>8));
	
	Timeout_t = Timeout;
	/* wait until the transmission data register is empty*/
	while(!i2c_flag_get(i2c_periph, I2C_FLAG_TBE))
	{
		if(Timeout_t > 0)
        {
            Timeout_t--;
        }
		else
        {
            return Read_TIMEOUT_FAULT;
        }
	}	
    
	/* send a data byte */
	i2c_data_transmit(i2c_periph,(uint8_t)(Address&0xff));
	
	Timeout_t = Timeout;
	/* wait until the transmission data register is empty*/
	while(!i2c_flag_get(i2c_periph, I2C_FLAG_TBE))
	{
		if(Timeout_t > 0)
        {
            Timeout_t--;
        }
		else
        {
            return Read_TIMEOUT_FAULT;
        }
	}

	/* send a stop condition to I2C bus*/
	i2c_stop_on_bus(i2c_periph);
	
	Timeout_t = Timeout;
	/* wait until stop condition generate */ 
	while(I2C_CTL0(i2c_periph)&0x0200)
	{
		if(Timeout_t > 0)
        {
            Timeout_t--;
        }
		else
        {
            return Read_TIMEOUT_FAULT;
        }
	}	
	
	/* enable acknowledge */
	i2c_ack_config(i2c_periph, I2C_ACK_ENABLE);

	/******************************************************/
	/*	    Send Slave address and Read Data 	 		  */
	/******************************************************/
	
	Timeout_t = Timeout;
	/* wait until I2C bus is idle */
	while(i2c_flag_get(i2c_periph, I2C_FLAG_I2CBSY))
	{
		if(Timeout_t > 0)
        {
            Timeout_t--;
        }
		else
        {
            return Read_TIMEOUT_FAULT;
        }
	}
	/* send a start condition to I2C bus */
	i2c_start_on_bus(i2c_periph);
	
	Timeout_t = Timeout;
	/* wait until SBSEND bit is set */
	while(!i2c_flag_get(i2c_periph, I2C_FLAG_SBSEND))
	{
		if(Timeout_t > 0)
        {
            Timeout_t--;
        }
		else
        {
            return Read_TIMEOUT_FAULT;
        }
	}

	/* send slave address to I2C bus*/
	i2c_master_addressing(i2c_periph, I2CX_SLAVE_ADDRESS7, I2C_RECEIVER);
    
	Timeout_t = Timeout;
	/* wait until ADDSEND bit is set*/
	while(!i2c_flag_get(i2c_periph, I2C_FLAG_ADDSEND))
	{
		if(Timeout_t > 0)
        {
            Timeout_t--;
        }
		else
        {
            return Read_TIMEOUT_FAULT;
        }
	}
	/* clear ADDSEND bit */
	i2c_flag_clear(i2c_periph, I2C_FLAG_ADDSEND);
    

    
	for(i=0;i<size;i++)
	{
        if(( i==(size - 3) )||(size < 3))
        {
            Timeout_t = Timeout;
            /* wait until the second last data byte is received into the shift register */
            while(!i2c_flag_get(i2c_periph, I2C_FLAG_BTC))
            {
                if(Timeout_t > 0)
                {
                    Timeout_t--;
                }
                else
                {
                    return Read_TIMEOUT_FAULT;
                }                    
            }
            /* disable acknowledge */
            i2c_ack_config(i2c_periph, I2C_ACK_DISABLE);
        }
        
		Timeout_t = Timeout;
		/* wait until the RBNE bit is set */
	    while(!i2c_flag_get(i2c_periph, I2C_FLAG_RBNE))
		{
			if(Timeout_t > 0)
            {
                Timeout_t--;
            }
			else
            {
                return Read_TIMEOUT_FAULT;
            }
		}
    	/* read data from I2C_DATA */
    	(*ndata) = i2c_data_receive(i2c_periph);	
		ndata++;
	}
    
	/* send a stop condition to I2C bus*/
  	i2c_stop_on_bus(i2c_periph);
	
	Timeout_t = Timeout;
	/* wait until stop condition generate */ 
 	while(I2C_CTL0(i2c_periph)&0x0200)
	{
		if(Timeout_t > 0)
        {            
            Timeout_t--;
        }
		else
        {
            return Read_TIMEOUT_FAULT;
        }
	}	
	
	/* enable acknowledge */
  	i2c_ack_config(i2c_periph, I2C_ACK_ENABLE); 
    return Read_SUCCEED;
}

