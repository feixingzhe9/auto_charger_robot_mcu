#include "stm32f10x.h"
#include "delay.h"
#include "usart.h"
#include "timer.h"
#include "i2c.h"
#include "VL6180x.h"
#include "remote.h"
#include "switch.h"
#include "global.h"
#include "tools.h"
#include "can_protocol.h"
#include "can.h"
#include "led.h"
#include "platform.h"
#include "watchdog.h"


void sys_init()
{
    systick_init();
    delay_init();/* delay function init */
    uart_init(COM_PORT1, 115200);/* uasrt init*/
    usart1_dma_init(115200);
    timer3_init(9999, 7199);/* 72000/(7199+1) = 10kHz 10k/(9999+1) = 10ms */
    i2c_init();
    priority_group_config(NVIC_PriorityGroup_2);
    time4_ch1_pwm_init(1895,0);/* 72000/(1895+1) = 37.99K */
    remote_init();/* infrared data recv init */
//    vl6180x_init();/* vl6180 small laser light init */
    swtich_init();
    led_init();
    can_init();
    wdg_init();
    wdg_enable();
}


int main(void)
{
    sys_init();

    while(1)
    {
        wdg_feed();
        remote_scan();/* recv infrared data */
//        calculate_length();/* get small laser len data */
        remote_calculate();/* get infrared data */
        update_status();/* update system status */
        can_protocol_period();/* can recv data process */
        led_indicator();/* system led */
    }
}

