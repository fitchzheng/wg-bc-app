#ifndef BSP_PWM_H
#define BSP_PWM_H

#include "hc32_ll.h"
#include "hc32_ll_hrpwm.h"
#include "my_math.h"
#include "hc32f334.h"

#define TIMER_FREQ (120000000) //主频120M
#define DEAD_TIME  (100)   //死区时间，100ns
#define CTRL_PERIOD ((uint32_t)(((float)TIMER_FREQ / PWM_FREQ/2)*64))
#define DEAD_SET_VAL (DEAD_TIME*12/100*64)

#define PWM_UNIT      CM_HRPWM1
#define A1_PWM_UNIT   CM_HRPWM4//CM_HRPWM4
#define A2_PWM_UNIT   CM_HRPWM5
#define B1_PWM_UNIT   CM_HRPWM3
#define B2_PWM_UNIT   CM_HRPWM2//CM_HRPWM2

#define SW_PWM_UNIT      HRPWM_SW_SYNC_UNIT1
#define A1_SW_PWM_UNIT   HRPWM_SW_SYNC_UNIT4//CM_HRPWM4
#define A2_SW_PWM_UNIT   HRPWM_SW_SYNC_UNIT5
#define B1_SW_PWM_UNIT   HRPWM_SW_SYNC_UNIT3
#define B2_SW_PWM_UNIT   HRPWM_SW_SYNC_UNIT2//CM_HRPWM2

#define HRPWM_PWM_FCG        (FCG2_PERIPH_HRPWM_1 | FCG2_PERIPH_HRPWM_2 | FCG2_PERIPH_HRPWM_3 | FCG2_PERIPH_HRPWM_4 | FCG2_PERIPH_HRPWM_5)
#define HRPWM_PWM_MUL_UNIT   (HRPWM_UNIT1 | HRPWM_UNIT2 | HRPWM_UNIT3 | HRPWM_UNIT4 | HRPWM_UNIT5)
#define HRPWM_PWM_SYNC_UNIT  (HRPWM_SW_SYNC_UNIT1 | HRPWM_SW_SYNC_UNIT2 | HRPWM_SW_SYNC_UNIT3 | HRPWM_SW_SYNC_UNIT4 | HRPWM_SW_SYNC_UNIT5)


#define HRPWM_HA1_PORT  GPIO_HRPWM4_PWMA  /* PB12 */
#define HRPWM_LA1_PORT  GPIO_HRPWM4_PWMB  /* PB13 */
#define HRPWM_LA2_PORT  GPIO_HRPWM5_PWMA  /* PB14 */
#define HRPWM_HA2_PORT  GPIO_HRPWM5_PWMB  /* PB15 */
#define HRPWM_HB1_PORT  GPIO_HRPWM3_PWMA  /* PA10 */
#define HRPWM_LB1_PORT  GPIO_HRPWM3_PWMB  /* PA11 */
#define HRPWM_HB2_PORT  GPIO_HRPWM2_PWMA  /* PA8  */
#define HRPWM_LB2_PORT  GPIO_HRPWM2_PWMB  /* PA9  */

#define HA1_EN_BIT  (0x01)  //通道使能
#define LA1_EN_BIT  (0x02)
#define HA2_EN_BIT  (0x04)
#define LA2_EN_BIT  (0x08)
#define HA1_INV_BIT (0x10)  //有效极性翻转
#define LA1_INV_BIT (0x20)
#define HA2_INV_BIT (0x40)
#define LA2_INV_BIT (0x80)

#define HB1_EN_BIT  (0x01)  //通道使能
#define LB1_EN_BIT  (0x02)
#define HB2_EN_BIT  (0x04)
#define LB2_EN_BIT  (0x08)
#define HB1_INV_BIT (0x10)  //有效极性翻转
#define LB1_INV_BIT (0x20)
#define HB2_INV_BIT (0x40)
#define LB2_INV_BIT (0x80)

#define PWM_DEAD_TIME (DEAD_SET_VAL)   //死区时间设置,100ns

#define HRPWM_PHSCMP1_VALUE (0)           //相位值设置1
#define HRPWM_PHSCMP2_VALUE (((CTRL_PERIOD * 2 / 2U) - 64U)) //相位值设置2

#define FULL_FREQ_REPEAT_CNT (0x04)
#define HALF_FREQ_REPEAT_CNT (0x02)

//正极性
//     /\        /\
//    /  \      /  \
//   /    \    /    \
// -/------\--/------\--
// /        \/        \
//   ______    ______
// __|     |___|     |__


//PWM有效极性
#define P_HIGH  (1)  //正极性
#define P_LOW   (0)  //反极性
//#define HRPWM_PWM_ACTIVE  (P_HIGH)    //默认有效高 向上计数匹配为高，向下计数匹配为低
#define HRPWM_PWM_ACTIVE  (P_LOW)   //默认有效低 向上计数匹配为低，向下计数匹配为高

    //HRPWM4极性 HA1 LA1，默认正极性
    #if (HRPWM_PWM_ACTIVE == P_HIGH)
        #define HRPWM4_HA1_START         HRPWM_PWM_START_HIGH
        #define HRPWM4_HA1_UP_MATCH      HRPWM_PWM_UP_MATCH_A_LOW
        #define HRPWM4_HA1_DOWN_MATCH    HRPWM_PWM_DOWN_MATCH_A_HIGH
        #define HRPWM4_LA1_START         HRPWM_PWM_START_LOW
        #define HRPWM4_LA1_UP_MATCH      HRPWM_PWM_UP_MATCH_A_HIGH
        #define HRPWM4_LA1_DOWN_MATCH    HRPWM_PWM_DOWN_MATCH_A_LOW
    #else
        
        
        #define HRPWM4_HA1_START         HRPWM_PWM_START_LOW  
        #define HRPWM4_HA1_UP_MATCH      HRPWM_PWM_UP_MATCH_A_HIGH
        #define HRPWM4_HA1_DOWN_MATCH    HRPWM_PWM_DOWN_MATCH_A_LOW
        #define HRPWM4_LA1_START         HRPWM_PWM_START_HIGH
        #define HRPWM4_LA1_UP_MATCH      HRPWM_PWM_UP_MATCH_A_LOW
        #define HRPWM4_LA1_DOWN_MATCH    HRPWM_PWM_DOWN_MATCH_A_HIGH
    #endif

    //HRPWM5极性 HA2 LA2，默认反极性
    #if (HRPWM_PWM_ACTIVE == P_HIGH)
        #define HRPWM5_HA2_START         HRPWM_PWM_START_LOW
        #define HRPWM5_HA2_UP_MATCH      HRPWM_PWM_UP_MATCH_A_HIGH
        #define HRPWM5_HA2_DOWN_MATCH    HRPWM_PWM_DOWN_MATCH_A_LOW
        #define HRPWM5_LA2_START         HRPWM_PWM_START_HIGH
        #define HRPWM5_LA2_UP_MATCH      HRPWM_PWM_UP_MATCH_A_LOW
        #define HRPWM5_LA2_DOWN_MATCH    HRPWM_PWM_DOWN_MATCH_A_HIGH  
    #else
        

        #define HRPWM5_HA2_START         HRPWM_PWM_START_HIGH  
        #define HRPWM5_HA2_UP_MATCH      HRPWM_PWM_UP_MATCH_A_LOW
        #define HRPWM5_HA2_DOWN_MATCH    HRPWM_PWM_DOWN_MATCH_A_HIGH
        #define HRPWM5_LA2_START         HRPWM_PWM_START_LOW
        #define HRPWM5_LA2_UP_MATCH      HRPWM_PWM_UP_MATCH_A_HIGH
        #define HRPWM5_LA2_DOWN_MATCH    HRPWM_PWM_DOWN_MATCH_A_LOW
    #endif

    //HRPWM3极性 HB1 LB1，默认正极性
    #if (HRPWM_PWM_ACTIVE == P_HIGH)
        #define HRPWM3_HB1_START         HRPWM_PWM_START_LOW
        #define HRPWM3_HB1_UP_MATCH      HRPWM_PWM_UP_MATCH_A_HIGH
        #define HRPWM3_HB1_DOWN_MATCH    HRPWM_PWM_DOWN_MATCH_A_LOW
        #define HRPWM3_LB1_START         HRPWM_PWM_START_HIGH
        #define HRPWM3_LB1_UP_MATCH      HRPWM_PWM_UP_MATCH_A_LOW
        #define HRPWM3_LB1_DOWN_MATCH    HRPWM_PWM_DOWN_MATCH_A_HIGH
    #else      
        #define HRPWM3_HB1_START         HRPWM_PWM_START_HIGH  
        #define HRPWM3_HB1_UP_MATCH      HRPWM_PWM_UP_MATCH_A_LOW
        #define HRPWM3_HB1_DOWN_MATCH    HRPWM_PWM_DOWN_MATCH_A_HIGH
        #define HRPWM3_LB1_START         HRPWM_PWM_START_LOW
        #define HRPWM3_LB1_UP_MATCH      HRPWM_PWM_UP_MATCH_A_HIGH
        #define HRPWM3_LB1_DOWN_MATCH    HRPWM_PWM_DOWN_MATCH_A_LOW
    #endif

    //HRPWM2极性 HB2 LB2，默认反极性
    #if (HRPWM_PWM_ACTIVE == P_HIGH)
        #define HRPWM2_HB2_START         HRPWM_PWM_START_HIGH 
        #define HRPWM2_HB2_UP_MATCH      HRPWM_PWM_UP_MATCH_A_LOW
        #define HRPWM2_HB2_DOWN_MATCH    HRPWM_PWM_DOWN_MATCH_A_HIGH
        #define HRPWM2_LB2_START         HRPWM_PWM_START_LOW
        #define HRPWM2_LB2_UP_MATCH      HRPWM_PWM_UP_MATCH_A_HIGH
        #define HRPWM2_LB2_DOWN_MATCH    HRPWM_PWM_DOWN_MATCH_A_LOW
    #else
        #define HRPWM2_HB2_START         HRPWM_PWM_START_LOW
        #define HRPWM2_HB2_UP_MATCH      HRPWM_PWM_UP_MATCH_A_HIGH
        #define HRPWM2_HB2_DOWN_MATCH    HRPWM_PWM_DOWN_MATCH_A_LOW
        #define HRPWM2_LB2_START         HRPWM_PWM_START_HIGH
        #define HRPWM2_LB2_UP_MATCH      HRPWM_PWM_UP_MATCH_A_LOW
        #define HRPWM2_LB2_DOWN_MATCH    HRPWM_PWM_DOWN_MATCH_A_HIGH

    #endif

    //HRPWM1极性 HB1 LB1，默认正极性
    #if (HRPWM_PWM_ACTIVE == P_HIGH)
        #define HRPWM1_HB1_START         HRPWM_PWM_START_LOW
        #define HRPWM1_HB1_UP_MATCH      HRPWM_PWM_UP_MATCH_A_HIGH
        #define HRPWM1_HB1_DOWN_MATCH    HRPWM_PWM_DOWN_MATCH_A_LOW
        #define HRPWM1_LB1_START         HRPWM_PWM_START_HIGH
        #define HRPWM1_LB1_UP_MATCH      HRPWM_PWM_UP_MATCH_A_LOW
        #define HRPWM1_LB1_DOWN_MATCH    HRPWM_PWM_DOWN_MATCH_A_HIGH
    #else      
        #define HRPWM1_HB1_START         HRPWM_PWM_START_HIGH  
        #define HRPWM1_HB1_UP_MATCH      HRPWM_PWM_UP_MATCH_A_LOW
        #define HRPWM1_HB1_DOWN_MATCH    HRPWM_PWM_DOWN_MATCH_A_HIGH
        #define HRPWM1_LB1_START         HRPWM_PWM_START_LOW
        #define HRPWM1_LB1_UP_MATCH      HRPWM_PWM_UP_MATCH_A_HIGH
        #define HRPWM1_LB1_DOWN_MATCH    HRPWM_PWM_DOWN_MATCH_A_LOW
    #endif



typedef struct
{
    float left_duty;
    uint8_t left_up_en;
    uint8_t left_dn_en;
    float right_duty;
    uint8_t right_up_en;
    uint8_t right_dn_en;
} bsp_pwm_t;

void bsp_pwm_init(void);

void bsp_pwm_set_a_duty(bsp_pwm_t *str);
void bsp_pwm_set_b_duty(bsp_pwm_t *str);
void bsp_pwm_update_chclt2(void);


void bsp_pwm_change_freq_pwma(void);
void bsp_pwm_change_full_freq_pwma(void);
void bsp_pwm_change_full_freq_pwmb(void);
void bsp_pwm_change_half_freq_pwma(void);
void bsp_pwm_change_half_freq_pwmb(void);

void bsp_hrpwm1_update_trig(void);
void bsp_pwm_deinit(void);
#endif

