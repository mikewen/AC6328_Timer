/*********************************************************************************************
    *   Filename        : app_main.c

    *   Description     :

    *   Copyright:(c)JIELI  2011-2019  @ , All Rights Reserved.
*********************************************************************************************/
#include "system/includes.h"
#include "app_config.h"
#include "app_action.h"
#include "app_main.h"
#include "update.h"
#include "update_loader_download.h"
#include "app_charge.h"
#include "app_power_manage.h"
#include "asm/charge.h"

#if TCFG_KWS_VOICE_RECOGNITION_ENABLE
#include "jl_kws/jl_kws_api.h"
#endif /* #if TCFG_KWS_VOICE_RECOGNITION_ENABLE */


#define LOG_TAG_CONST       APP
#define LOG_TAG             "[APP]"
//#define LOG_ERROR_ENABLE
//#define LOG_DEBUG_ENABLE
//#define LOG_INFO_ENABLE
/* #define LOG_DUMP_ENABLE */
//#define LOG_CLI_ENABLE
#include "debug.h"

/*任务列表 */
const struct task_info task_info_table[] = {
    {"app_core",            1,     0,   640,   128  },
    {"sys_event",           7,     0,   256,   0    },
    {"btctrler",            4,     0,   512,   256  },
    {"btencry",             1,     0,   512,   128  },
    {"btstack",             3,     0,   768,   256   },
    {"systimer",		    7,	   0,   128,   0	},
    {"update",				1,	   0,   512,   0    },
    {"dw_update",		 	2,	   0,   256,   128  },
#if (RCSP_BTMATE_EN)
    {"rcsp_task",		    2,	   0,   640,	0},
#endif
#if(USER_UART_UPDATE_ENABLE)
    {"uart_update",	        1,	   0,   256,   128	},
#endif
#if (XM_MMA_EN)
    {"xm_mma",   		    2,	   0,   640,   256	},
#endif
    {"usb_msd",           	1,     0,   512,   128  },
#if TCFG_AUDIO_ENABLE
    {"audio_dec",           3,     0,   768,   128  },
    {"audio_enc",           4,     0,   512,   128  },
#endif/*TCFG_AUDIO_ENABLE*/
#if TCFG_KWS_VOICE_RECOGNITION_ENABLE
    {"kws",                 2,     0,   256,   64   },
#endif /* #if TCFG_KWS_VOICE_RECOGNITION_ENABLE */
#if (TUYA_DEMO_EN)
    {"user_deal",           7,     0,   512,   512  },//定义线程 tuya任务调度
#endif

    {0, 0},
};

APP_VAR app_var;

void app_var_init(void)
{
    app_var.play_poweron_tone = 1;

    app_var.auto_off_time =  TCFG_AUTO_SHUT_DOWN_TIME;
    app_var.warning_tone_v = 340;
    app_var.poweroff_tone_v = 330;
}

__attribute__((weak))
u8 get_charge_online_flag(void)
{
    return 0;
}

void clr_wdt(void);
void check_power_on_key(void)
{
#if TCFG_POWER_ON_NEED_KEY

    u32 delay_10ms_cnt = 0;
    while (1) {
        clr_wdt();
        os_time_dly(1);

        extern u8 get_power_on_status(void);
        if (get_power_on_status()) {
            log_info("+");
            delay_10ms_cnt++;
            if (delay_10ms_cnt > 70) {
                /* extern void set_key_poweron_flag(u8 flag); */
                /* set_key_poweron_flag(1); */
                return;
            }
        } else {
            log_info("-");
            delay_10ms_cnt = 0;
            log_info("enter softpoweroff\n");
            power_set_soft_poweroff();
        }
    }
#endif
}

/*
void checkVoltage(){
    //if(adc_get_value(AD_CH_PC5)>200){    //  >120V
    if(adc_get_value(AD_CH_PC5)>280){    //  270 ~= 87V
        gpio_set_pull_up(IO_PORTA_05, 0);gpio_set_pull_down(IO_PORTA_05, 1);    // Turn on heater/oven
    }else{
        gpio_set_pull_up(IO_PORTA_05, 1);gpio_set_pull_down(IO_PORTA_05, 0);    // turn off
    }
}
*/
/*
void checkWater(){
    if(adc_get_value(AD_CH_PA0)>900){    // ~= 600 in water
        gpio_set_pull_up(IO_PORTB_05, 0);gpio_set_pull_down(IO_PORTB_05, 1);    //LED off
        gpio_set_pull_up(IO_PORTA_05, 0);gpio_set_pull_down(IO_PORTA_05, 1);    // Turn off pump
    }else{
        gpio_set_pull_up(IO_PORTB_05, 1);gpio_set_pull_down(IO_PORTB_05, 0);    // LED on
        gpio_set_pull_up(IO_PORTA_05, 1);gpio_set_pull_down(IO_PORTA_05, 0);    // turn on pump
    }
}
*/
/*
static u8 onoff=0;
void ledBlink(){
    if(onoff==0){
        onoff=1;
        gpio_set_pull_up(IO_PORTB_05, 0);gpio_set_pull_down(IO_PORTB_05, 1);
    }else{
        onoff=0;
        gpio_set_pull_up(IO_PORTB_05, 1);gpio_set_pull_down(IO_PORTB_05, 0);
    }
}

static u8 duty=0;static u8 start_stop=0;
void change_duty(){
    if(start_stop==0){ // start, increase duty
        if(duty<10000){
            duty += 100;
        }
    }else{  //stopping
        if(duty==0){ power_set_soft_poweroff(); }
        if(duty>0){
            duty -= 100;
        }
    }
    set_timer_pwm_duty(JL_TIMER3,duty);
    //set_timer_pwm_duty(JL_TIMER0,duty);   //SDK 占用了0/1/2，用户可以使用timer3/4/5
}

void stop(){
    //duty=10000;start_stop=1;
    gpio_set_pull_up(IO_PORTB_05, 0);gpio_set_pull_down(IO_PORTB_05, 1);
    gpio_set_pull_up(IO_PORTA_05, 0);gpio_set_pull_down(IO_PORTA_05, 1);
    power_set_soft_poweroff();
}

void checkBat(){    // adc_get_voltage(AD_CH_VBAT)*4/10
    if(adc_get_voltage(AD_CH_VBAT)*4/10>270){
        gpio_set_pull_up(IO_PORTB_05, 1);gpio_set_pull_down(IO_PORTB_05, 0);    //turn on LED or Burn resister
        //timer_pwm_init(JL_TIMER3, 10000, 3000, IO_PORTB_05, 0);   // PWM won't work in sleep mode, TCFG_LOWPOWER_LOWPOWER_SEL
    }else{
        //set_timer_pwm_duty(JL_TIMER3,0);
        gpio_set_pull_up(IO_PORTB_05, 0);gpio_set_pull_down(IO_PORTB_05, 1);
    }
}
*/

static u16 timerID, startTimerID, stopTimerID;
void initUSBPorts(){
    gpio_set_dieh(IO_PORT_DM, 0);gpio_set_die(IO_PORT_DM, 1);gpio_set_pull_down(IO_PORT_DM, 0);gpio_set_pull_up(IO_PORT_DM, 0);
    gpio_set_dieh(IO_PORT_DP, 0);gpio_set_die(IO_PORT_DP, 1);gpio_set_pull_down(IO_PORT_DP, 0);gpio_set_pull_up(IO_PORT_DP, 0);
    gpio_direction_output(IO_PORT_DM, 0); gpio_direction_output(IO_PORT_DP, 0);
}
void endRelayFunc(){
    gpio_direction_output(IO_PORT_DM, 0); gpio_direction_output(IO_PORT_DP, 0);
    sys_timeout_del(timerID);
}

/*
void timer_delay_ms(u8 ms)
{
    JL_TIMER2->CNT = 0;
    JL_TIMER2->PRD = ms * (24000000L / 1000);
    JL_TIMER2->CON = BIT(0) | (6 << 10) | BIT(14); //1分频,std 24m，24次就1us
    while (!(JL_TIMER2->CON & BIT(15))); //等pending
    JL_TIMER2->CON = 0;
}
*/

void startPower(){
    gpio_direction_output(IO_PORT_DM, 1); gpio_direction_output(IO_PORT_DP, 0);
    timerID = sys_timeout_add(NULL, endRelayFunc, 50);
    //timer_delay_ms(50);
    //gpio_direction_output(IO_PORT_DM, 0); gpio_direction_output(IO_PORT_DP, 0);
    sys_timeout_del(startTimerID);
}

void stopPower(){
    //gpio_set_pull_up(IO_PORTA_09, 0);gpio_set_pull_down(IO_PORTA_09, 1);
    gpio_direction_output(IO_PORT_DM, 0); gpio_direction_output(IO_PORT_DP, 1);
    timerID = sys_timeout_add(NULL, endRelayFunc, 50);
    sys_timeout_del(stopTimerID);
    //power_set_soft_poweroff();
    sys_timeout_add(NULL, power_set_soft_poweroff, 500);
}

#define AUTO_RUN    1
#define APP_CTRL    0

#if(AUTO_RUN == 1)

static u8 idle_query(void){ return 0; }     //if(can_enter_lp){
REGISTER_LP_TARGET(idle_lp_target) = {
    .name = "idel_lp",
    .is_idle = idle_query,
};

#endif

u8 minutes[1];
void app_main()
{
    /*
    if(!syscfg_read(RUN_MINUTES, minutes, 1)){
        minutes[0] = 2;
    }
    */
    syscfg_read(RUN_MINUTES, minutes, 1);
    if(minutes[0]<2){
        minutes[0] = 3;
        syscfg_write(RUN_MINUTES, minutes, 1);
    }

    initUSBPorts();
#if(AUTO_RUN == 1)  // Also need to change sleep_enter_callback
    //gpio_set_pull_up(IO_PORTA_05, 1);gpio_set_pull_down(IO_PORTA_05, 0);    // Turn on pump
    //gpio_set_pull_up(IO_PORTA_09, 1);gpio_set_pull_down(IO_PORTA_09, 0);

    //timer_pwm_init(JL_TIMER0, 1000, 10000, IO_PORTA_05, 0);  //timer_pwm_init(JL_TIMER0, freq, duty, IO_PORTA_05, 0);
    startTimerID = sys_timeout_add(NULL, startPower , 2000); // start power after few seconds
    stopTimerID = sys_timeout_add(NULL, stopPower , minutes[0]*60000); // stop after x minutes
    //gpio_direction_output(IO_PORT_DM, 1); gpio_direction_output(IO_PORT_DP, 0);
    //timerID = sys_timeout_add(NULL, endRelayFunc, 50);
    //timerID = sys_timer_add(NULL, ledBlink , 1000); //every x seconds
#endif

#if(APP_CTRL == 1)
    //gpio_set_pull_up(IO_PORTA_05, 0);gpio_set_pull_down(IO_PORTA_05, 1);    // Turn off pump
#endif

    //timer_pwm_init(JL_TIMER1, 1000, 0, IO_PORTC_04, 0);  // start when call set_timer_pwm_duty
    //timer_pwm_init(JL_TIMER3, 10000, 0, IO_PORTB_05, 0);
    //timer_pwm_init(JL_TIMER0, 10000, 0, IO_PORTA_05, 0);
    //timerID = sys_timer_add(NULL, change_duty , 1000); // adjust duty
    //stopTimerID = sys_timeout_add(NULL, stop , 60000); // stop after x minutes

    struct intent it;

    if (!UPDATE_SUPPORT_DEV_IS_NULL()) {
        int update = 0;
        update = update_result_deal();
    }

    printf(">>>>>>>>>>>>>>>>>app_main...\n");

    //adc_add_sample_ch(AD_CH_PA0);adc_set_sample_freq(AD_CH_PA0, 3000);  // NTC
    //adc_add_sample_ch(AD_CH_PA3);adc_set_sample_freq(AD_CH_PA9, 3000);  // VCC
    //adc_add_sample_ch(AD_CH_DM);adc_set_sample_freq(AD_CH_DM, 3000);    // HV test
    //adc_add_sample_ch(AD_CH_VBAT);adc_set_sample_freq(AD_CH_VBAT, 20000);adc_add_sample_ch(AD_CH_LDOREF);adc_set_sample_freq(AD_CH_LDOREF, 20000);

    //timerID = sys_timer_add(NULL, checkBat, 20000); //every 20 seconds
    //timerID = sys_timer_add(NULL, checkVoltage , 60000); //every 60 seconds
    //timerID = sys_timer_add(NULL, checkWater , 5000); //every 5 seconds

    if (get_charge_online_flag()) {
#if(TCFG_SYS_LVD_EN == 1)
        vbat_check_init();
#endif
    } else {
        check_power_on_voltage();   // adc_get_value(AD_CH_VBAT) //adc_set_vbat_vddio_tieup(1);
    }

#if TCFG_POWER_ON_NEED_KEY
    check_power_on_key();
#endif

#if TCFG_AUDIO_ENABLE
    extern int audio_dec_init();
    extern int audio_enc_init();
    audio_dec_init();
    audio_enc_init();
#endif/*TCFG_AUDIO_ENABLE*/

#if TCFG_KWS_VOICE_RECOGNITION_ENABLE
    jl_kws_main_user_demo();
#endif /* #if TCFG_KWS_VOICE_RECOGNITION_ENABLE */

    init_intent(&it);

#if CONFIG_APP_SPP_LE
    it.name = "spp_le";
    it.action = ACTION_SPPLE_MAIN;

#elif CONFIG_APP_AT_COM || CONFIG_APP_AT_CHAR_COM
    it.name = "at_com";
    it.action = ACTION_AT_COM;

#elif CONFIG_APP_DONGLE
    it.name = "dongle";
    it.action = ACTION_DONGLE_MAIN;

#elif CONFIG_APP_MULTI
    it.name = "multi_conn";
    it.action = ACTION_MULTI_MAIN;

#elif CONFIG_APP_NONCONN_24G
    it.name = "nonconn_24g";
    it.action = ACTION_NOCONN_24G_MAIN;

#elif CONFIG_APP_LL_SYNC
    it.name = "ll_sync";
    it.action = ACTION_LL_SYNC;

#elif CONFIG_APP_TUYA
    it.name = "tuya";
    it.action = ACTION_TUYA;

#elif CONFIG_APP_CENTRAL
    it.name = "central";
    it.action = ACTION_CENTRAL_MAIN;

#elif CONFIG_APP_DONGLE
    it.name = "dongle";
    it.action = ACTION_DONGLE_MAIN;

#elif CONFIG_APP_BEACON
    it.name = "beacon";
    it.action = ACTION_BEACON_MAIN;

#elif CONFIG_APP_IDLE
    it.name = "idle";
    it.action = ACTION_IDLE_MAIN;

#elif CONFIG_APP_CONN_24G
    it.name = "conn_24g";
    it.action = ACTION_CONN_24G_MAIN;

#else
    while (1) {
        printf("no app!!!");
    }
#endif


    log_info("run app>>> %s", it.name);
    log_info("%s,%s", __DATE__, __TIME__);

    start_app(&it);

#if TCFG_CHARGE_ENABLE
    set_charge_event_flag(1);
#endif
}

/*
 * app模式切换
 */
void app_switch(const char *name, int action)
{
    struct intent it;
    struct application *app;

    log_info("app_exit\n");

    init_intent(&it);
    app = get_current_app();
    if (app) {
        /*
         * 退出当前app, 会执行state_machine()函数中APP_STA_STOP 和 APP_STA_DESTORY
         */
        it.name = app->name;
        it.action = ACTION_BACK;
        start_app(&it);
    }

    /*
     * 切换到app (name)并执行action分支
     */
    it.name = name;
    it.action = action;
    start_app(&it);
}

int eSystemConfirmStopStatus(void)
{
    /* 系统进入在未来时间里，无任务超时唤醒，可根据用户选择系统停止，或者系统定时唤醒(100ms) */
    //1:Endless Sleep
    //0:100 ms wakeup
    /* log_info("100ms wakeup"); */
    return 1;
}

__attribute__((used)) int *__errno()
{
    static int err;
    return &err;
}


