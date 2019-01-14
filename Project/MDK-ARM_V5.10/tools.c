#include "tools.h"
#include "platform.h"

#define REMOTE_ID_POWER_1     0x33
#define REMOTE_ID_POWER_2     0x66
#define REMOTE_ID_POWER_3     0x99
#define REMOTE_ID_POWER_4     0xbb
#define REMOTE_ID_POWER_MAX   0xFF

#define VL6180_IDLE_PERIOD      15 / SYSTICK_PERIOD
#define VL6180_MEASURE_PERIOD   50 / SYSTICK_PERIOD
uint8_t calculate_length(void)
{
    static uint8_t status = 0;
    static uint8_t m_state = 0;
    static uint32_t start_tick = 0;

    switch(m_state)
    {
        case 0:
            if(get_tick() -  start_tick > VL6180_IDLE_PERIOD)
            {
                status = vl6180x_start();
                start_tick = get_tick();
                m_state = 1;
            }
            break;
        case 1:
            if(get_tick() -  start_tick > VL6180_MEASURE_PERIOD)
            {
                status = vl6180x_finish();
                start_tick = get_tick();
                m_state = 0;
            }
            break;
        default: break;
    }


    if(status == 0)
    {
        power_ctl.err_type &= (0xff-ERR_VL6180X);
    }
    else
    {
        power_ctl.err_type |= ERR_VL6180X;
    }

    return status;
}


#define SWITCH_ON_DELAY_TICK    80 / SYSTICK_PERIOD
void update_status(void)
{
    static uint8_t switch_status = SWITCH_OFF;
    static uint8_t prev_status = SWITCH_OFF;
    static uint8_t time_out = 0;
//    static uint8_t time_wait = 0;
    static uint32_t start_tick = 0;

    prev_status = switch_status;
    switch_status = switch_scan();

    if((switch_status != SWITCH_ON) && (switch_status != SWITCH_OFF))
    {
        power_ctl.err_type |= ERR_SWITCH;
    }
    else
    {
        power_ctl.err_type &= (0xff-ERR_SWITCH);
        power_ctl.err_type &= (0xff-ERR_NO_LIGHT_RECEIVE);
    }

    power_ctl.switch_status = switch_status;
    if(switch_status == SWITCH_ON)
    {
        if(prev_status != SWITCH_ON)
        {
//            time_wait = 8;
            start_tick = get_tick();
        }

//        if(time_wait !=0)
//        {
//            time_wait --;
//        }
        if(get_tick() - start_tick >= SWITCH_ON_DELAY_TICK)
        {
        }
        else
        {
            power_ctl.power_state = POWER_ON;

            if(timer_1s_flag == TIME_UP)
            {
                timer_1s_flag = TIME_NOT_UP;

                TIM_ITConfig( TIM4,TIM_IT_Update | TIM_IT_CC2 | TIM_IT_CC3 | TIM_IT_CC4, DISABLE);
                DMA_ITConfig(DMA1_Channel4,DMA_IT_TC, DISABLE);
                TIM_ITConfig(TIM3,TIM_IT_Update, DISABLE );
                time4_ch1_pwm_init(1895,0);

                if(power_ctl.vol < 25)
                {
                    send_remote_data(REMOTE_ID_POWER_1, REMOTE_ID_POWER_1);
                }
                else if(power_ctl.vol < 50)
                {
                    send_remote_data(REMOTE_ID_POWER_2, REMOTE_ID_POWER_2);
                }
                else if(power_ctl.vol < 75)
                {
                    send_remote_data(REMOTE_ID_POWER_3, REMOTE_ID_POWER_3);
                }
                else if(power_ctl.vol < 100)
                {
                    send_remote_data(REMOTE_ID_POWER_4, REMOTE_ID_POWER_4);
                }
                else if(power_ctl.vol == 100)
                {
                    send_remote_data(REMOTE_ID_POWER_MAX, REMOTE_ID_POWER_MAX);
                }

                TIM_ITConfig( TIM4,TIM_IT_Update | TIM_IT_CC2 |TIM_IT_CC3 | TIM_IT_CC4, ENABLE);
                DMA_ITConfig(DMA1_Channel4,DMA_IT_TC, ENABLE);
                TIM_ITConfig(TIM3,TIM_IT_Update, ENABLE );
                remote_init();
                time_out = (time_out + 1) % 5;
            }
        }
    }
    else
    {
        power_ctl.power_state = POWER_OFF;
    }
}

