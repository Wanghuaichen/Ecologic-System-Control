#include "led.h"
#include "delay.h"
#include "key.h"
#include "sys.h"
#include "usart.h"	
#include "usmart.h"	 
#include "rtc.h" 
#include "oled.h"
#include "ds18b20.h"
#include "adc.h"
#include "timer.h"
#include "driver.h"

/*****************************函数定义区*****************************************/ 
void Get_Zhuodu(void);            //读取浊度传感器AD值函数
void Get_Temperature(void);       //读取防水型温度传感器AD值函数
void Get_Liquid_Level(void);      //读取液位传感器AD值函数

/*****************************变量定义区*****************************************/
	u8 page_turning_flag=0;           //换页标志位（0--第一页显示主要信息，1-第二页）
	u16 PWM_duty_cycle=120;           //开机占空比（5/6    600/720）
  vu8 KEY=0;                        //键值
	extern u8 page_turning_time;      //第一页切换计时
	extern u8 page_turning_time_out;  //第二页切换计时

/******************************主函数程序代码****************************************/
 int main(void)                  //主函数开始运行
 {	 
	delay_init();	    	           //延时函数初始化	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //设置中断优先级分组为组2：2位抢占优先级，2位响应优先级
	uart_init(115200);	           //串口初始化为115200
 	LED_Init();			               //LED端口初始化		 
	KEY_Init();                    //初始化与按键连接的硬件接口	 
	OLED_Init();                   //OLED初始化
	usmart_dev.init(SystemCoreClock/1000000);	   //初始化USMART	
	Driver_Init_X();		           //X轴驱动器初始化
	Driver_Init_Y();		           //Y轴驱动器初始化
	TIM1_OPM_RCR_Init(999,72-1);   //1MHz计数频率  单脉冲+重复计数模式（X轴）
	TIM8_OPM_RCR_Init(999,72-1);   //1MHz计数频率  单脉冲+重复计数模式（Y轴)
	TIM6_Int_Init(9999,7199);      //10KHZ计数，延时1s
	TIM3_PWM_Init(719,0);	         //不分频。PWM频率=72000000/720=100Khz
	RTC_Init();	  			           //RTC初始化
	Adc_Init();		  	      	     //ADC初始化
  DS18B20_Init();                //18B20初始化
	while(1)
	{		
	  KEY=KEY_Scan(0);	               //得到键值
		if(KEY)
		{						   
			switch(KEY)
			{				 
				case KEY2_PRES:	             //加速电机转动
					PWM_duty_cycle-=10;        //其实是加占空比
				  if(PWM_duty_cycle<=0)       //设置下限（其实是上限）
					{
						PWM_duty_cycle=0;
					}
					else if(PWM_duty_cycle>=120)     //设置上限（其实是下限）
					{
						PWM_duty_cycle=120;
					}	
					break;
				case KEY1_PRES:	             //减速电机转动 
          PWM_duty_cycle+=10;        //其实是减占空比
					if(PWM_duty_cycle>=120)     //设置上限（其实是下限）
					{
						PWM_duty_cycle=0;
					}		
          else if(PWM_duty_cycle<=0)       //设置下限（其实是上限）
					{
						PWM_duty_cycle=0;
					}					
					break;  
				case KEY0_PRES:	             // 左--向左翻页   
					page_turning_flag=0;       //另翻页标志变成0 即进入页面1
				  page_turning_time=30;      //页面1翻页至页面2的时间（30s）刷新
					break;
				case KEY3_PRES:	             // 右--向右翻页
					page_turning_flag=1;       //另翻页标志变成1 即进入页面2
				  page_turning_time_out=30;  //页面2翻页至页面2的时间（30s）刷新
					break;
			}
		}		
		
		TIM_SetCompare2(TIM3,PWM_duty_cycle);       //电机占空比调节函数
		
		if(page_turning_flag==0)                    //OLED第一页
		{  
			Get_Zhuodu();		                          //读取浊度传感器值
		  Get_Temperature();                        //读取温度传感器值
			OLED_ShowString(0,0,"    /  /  ",16);     //显示年月日中间斜杠使用
	    OLED_ShowString(0,16,"  :  :  ",16);      //显示时分秒中间冒号使用
			OLED_ShowNum(0,0,calendar.w_year,4,16);   //读取年
			OLED_ShowNum(40,0,calendar.w_month,2,16); //读取月
			OLED_ShowNum(64,0,calendar.w_date,2,16);  //读取日
			switch(calendar.week)                     //判断现在的日期是周几
			{
				case 0:
				OLED_ShowString(88,0,"Sun.",16);        //周日     
					break;
				case 1:
				OLED_ShowString(88,0,"Mon.",16);        //周一
					break;
				case 2:
				OLED_ShowString(88,0,"Tues.",16);       //周二
					break;
				case 3:
				OLED_ShowString(88,0,"Wed.",16);        //周三
					break;
				case 4: 
				OLED_ShowString(88,0,"Thur.",16);       //周四
					break;
				case 5:
				OLED_ShowString(88,0,"Fri.",16);        //周五
					break;
				case 6:
				OLED_ShowString(88,0,"Sat.",16);        //周六
					break;  
			}
			
			OLED_ShowNum(0,16,calendar.hour,2,16);	  //显示小时	
			OLED_ShowNum(24,16,calendar.min,2,16);    //显示分钟
			OLED_ShowNum(48,16,calendar.sec,2,16);		//显示秒
   		OLED_Refresh_Gram();				              //更新显示到OLED             			  
	 }
		
	 else if(page_turning_flag==1)                //OLED第二页
	 {
			Get_Liquid_Level();                       //液位传感器值
		  fill_the_spaces();                        //没用的地方填上空格
		  OLED_Refresh_Gram();                      //OLED更新函数
	 }
	 }
 }
 
 /*******************************浊度传感器***************************************/
 void Get_Zhuodu()
{
	  u16 adcx;
  	adcx=Get_Adc_Average(ADC_Channel_12,40);		//得到ADC转换值	  
 	  OLED_ShowString(0,32,"TUR:",16);            //浊度缩写TUR
    OLED_ShowNum(32,32,adcx,4,16);              //显示浊度adc值
		OLED_ShowNum(80,32,PWM_duty_cycle,4,16);    //显示PWM占空比值
	  OLED_Refresh_Gram();		                    //更新显示到OLED 
}

/*****************************温度传感器*****************************************/
void Get_Temperature()
{
		u8 t=0;			    
	short temperature; 
	 		if(t%10==0)			                          //每100ms读取一次
		{									  
			temperature=DS18B20_Get_Temp();           //得到温度值
      OLED_ShowString(0,48,"TEMP:  . C",16);		//TEMP温度值    单位摄氏度 精确到小数点后一位	  
			OLED_ShowNum(40,48,temperature/10,2,16);	//显示正数部分	    
   		OLED_ShowNum(64,48,temperature%10,1,16);	//显示小数部分
      OLED_Refresh_Gram();		                  //更新显示到OLED 			
		}
}

/****************************液位传感器*****************************************/
 void Get_Liquid_Level()
{
	  u16 adcx;
  	adcx=Get_Adc_Average(ADC_Channel_13,40);		//得到ADC转换值	  
 	  OLED_ShowString(0,0,"LEVEL:",16);           //水位等级LEVEL
    OLED_ShowNum(48,0,adcx,4,16);               //显示水位ADC值
	  OLED_Refresh_Gram();		                    //更新显示到OLED 
}

