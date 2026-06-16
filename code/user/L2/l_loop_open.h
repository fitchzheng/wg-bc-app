#ifndef __L_LOOP_OPEN_H
#define __L_LOOP_OPEN_H

#define OPEN_LOOP_FIX_DUTY 0.8f

float cal_boost_dcm_duty(float i_ref,
                         float t_s,
                         float v_in,
                         float v_out,
                         float l);

float cal_buck_dcm_duty(float i_ref,
                        float t_s,
                        float v_in,
                        float v_out,
                        float l);

void cal_buck_boost_dcm_duty(float v_in,
                             float v_out,
                             float i_ref,
                             float t,
                             float l,
                             float *p_buck_duty,
                             float *p_boost_duty);

#endif
