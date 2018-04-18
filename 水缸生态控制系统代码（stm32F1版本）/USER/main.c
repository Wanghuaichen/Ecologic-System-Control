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

/*****************************����������*****************************************/ 
void Get_Zhuodu(void);            //��ȡ�Ƕȴ�����ADֵ����
void Get_Temperature(void);       //��ȡ��ˮ���¶ȴ�����ADֵ����
void Get_Liquid_Level(void);      //��ȡҺλ������ADֵ����

/*****************************����������*****************************************/
	u8 page_turning_flag=0;           //��ҳ��־λ��0--��һҳ��ʾ��Ҫ��Ϣ��1-�ڶ�ҳ��
	u16 PWM_duty_cycle=120;           //����ռ�ձȣ�5/6    600/720��
  vu8 KEY=0;                        //��ֵ
	extern u8 page_turning_time;      //��һҳ�л���ʱ
	extern u8 page_turning_time_out;  //�ڶ�ҳ�л���ʱ

/******************************�������������****************************************/
 int main(void)                  //��������ʼ����
 {	 
	delay_init();	    	           //��ʱ������ʼ��	  
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);  //�����ж����ȼ�����Ϊ��2��2λ��ռ���ȼ���2λ��Ӧ���ȼ�
	uart_init(115200);	           //���ڳ�ʼ��Ϊ115200
 	LED_Init();			               //LED�˿ڳ�ʼ��		 
	KEY_Init();                    //��ʼ���밴�����ӵ�Ӳ���ӿ�	 
	OLED_Init();                   //OLED��ʼ��
	usmart_dev.init(SystemCoreClock/1000000);	   //��ʼ��USMART	
	Driver_Init_X();		           //X����������ʼ��
	Driver_Init_Y();		           //Y����������ʼ��
	TIM1_OPM_RCR_Init(999,72-1);   //1MHz����Ƶ��  ������+�ظ�����ģʽ��X�ᣩ
	TIM8_OPM_RCR_Init(999,72-1);   //1MHz����Ƶ��  ������+�ظ�����ģʽ��Y��)
	TIM6_Int_Init(9999,7199);      //10KHZ��������ʱ1s
	TIM3_PWM_Init(719,0);	         //����Ƶ��PWMƵ��=72000000/720=100Khz
	RTC_Init();	  			           //RTC��ʼ��
	Adc_Init();		  	      	     //ADC��ʼ��
  DS18B20_Init();                //18B20��ʼ��
	while(1)
	{		
	  KEY=KEY_Scan(0);	               //�õ���ֵ
		if(KEY)
		{						   
			switch(KEY)
			{				 
				case KEY2_PRES:	             //���ٵ��ת��
					PWM_duty_cycle-=10;        //��ʵ�Ǽ�ռ�ձ�
				  if(PWM_duty_cycle<=0)       //�������ޣ���ʵ�����ޣ�
					{
						PWM_duty_cycle=0;
					}
					else if(PWM_duty_cycle>=120)     //�������ޣ���ʵ�����ޣ�
					{
						PWM_duty_cycle=120;
					}	
					break;
				case KEY1_PRES:	             //���ٵ��ת�� 
          PWM_duty_cycle+=10;        //��ʵ�Ǽ�ռ�ձ�
					if(PWM_duty_cycle>=120)     //�������ޣ���ʵ�����ޣ�
					{
						PWM_duty_cycle=0;
					}		
          else if(PWM_duty_cycle<=0)       //�������ޣ���ʵ�����ޣ�
					{
						PWM_duty_cycle=0;
					}					
					break;  
				case KEY0_PRES:	             // ��--����ҳ   
					page_turning_flag=0;       //��ҳ��־���0 ������ҳ��1
				  page_turning_time=30;      //ҳ��1��ҳ��ҳ��2��ʱ�䣨30s��ˢ��
					break;
				case KEY3_PRES:	             // ��--���ҷ�ҳ
					page_turning_flag=1;       //��ҳ��־���1 ������ҳ��2
				  page_turning_time_out=30;  //ҳ��2��ҳ��ҳ��2��ʱ�䣨30s��ˢ��
					break;
			}
		}		
		
		TIM_SetCompare2(TIM3,PWM_duty_cycle);       //���ռ�ձȵ��ں���
		
		if(page_turning_flag==0)                    //OLED��һҳ
		{  
			Get_Zhuodu();		                          //��ȡ�Ƕȴ�����ֵ
		  Get_Temperature();                        //��ȡ�¶ȴ�����ֵ
			OLED_ShowString(0,0,"    /  /  ",16);     //��ʾ�������м�б��ʹ��
	    OLED_ShowString(0,16,"  :  :  ",16);      //��ʾʱ�����м�ð��ʹ��
			OLED_ShowNum(0,0,calendar.w_year,4,16);   //��ȡ��
			OLED_ShowNum(40,0,calendar.w_month,2,16); //��ȡ��
			OLED_ShowNum(64,0,calendar.w_date,2,16);  //��ȡ��
			switch(calendar.week)                     //�ж����ڵ��������ܼ�
			{
				case 0:
				OLED_ShowString(88,0,"Sun.",16);        //����     
					break;
				case 1:
				OLED_ShowString(88,0,"Mon.",16);        //��һ
					break;
				case 2:
				OLED_ShowString(88,0,"Tues.",16);       //�ܶ�
					break;
				case 3:
				OLED_ShowString(88,0,"Wed.",16);        //����
					break;
				case 4: 
				OLED_ShowString(88,0,"Thur.",16);       //����
					break;
				case 5:
				OLED_ShowString(88,0,"Fri.",16);        //����
					break;
				case 6:
				OLED_ShowString(88,0,"Sat.",16);        //����
					break;  
			}
			
			OLED_ShowNum(0,16,calendar.hour,2,16);	  //��ʾСʱ	
			OLED_ShowNum(24,16,calendar.min,2,16);    //��ʾ����
			OLED_ShowNum(48,16,calendar.sec,2,16);		//��ʾ��
   		OLED_Refresh_Gram();				              //������ʾ��OLED             			  
	 }
		
	 else if(page_turning_flag==1)                //OLED�ڶ�ҳ
	 {
			Get_Liquid_Level();                       //Һλ������ֵ
		  fill_the_spaces();                        //û�õĵط����Ͽո�
		  OLED_Refresh_Gram();                      //OLED���º���
	 }
	 }
 }
 
 /*******************************�Ƕȴ�����***************************************/
 void Get_Zhuodu()
{
	  u16 adcx;
  	adcx=Get_Adc_Average(ADC_Channel_12,40);		//�õ�ADCת��ֵ	  
 	  OLED_ShowString(0,32,"TUR:",16);            //�Ƕ���дTUR
    OLED_ShowNum(32,32,adcx,4,16);              //��ʾ�Ƕ�adcֵ
		OLED_ShowNum(80,32,PWM_duty_cycle,4,16);    //��ʾPWMռ�ձ�ֵ
	  OLED_Refresh_Gram();		                    //������ʾ��OLED 
}

/*****************************�¶ȴ�����*****************************************/
void Get_Temperature()
{
		u8 t=0;			    
	short temperature; 
	 		if(t%10==0)			                          //ÿ100ms��ȡһ��
		{									  
			temperature=DS18B20_Get_Temp();           //�õ��¶�ֵ
      OLED_ShowString(0,48,"TEMP:  . C",16);		//TEMP�¶�ֵ    ��λ���϶� ��ȷ��С�����һλ	  
			OLED_ShowNum(40,48,temperature/10,2,16);	//��ʾ��������	    
   		OLED_ShowNum(64,48,temperature%10,1,16);	//��ʾС������
      OLED_Refresh_Gram();		                  //������ʾ��OLED 			
		}
}

/****************************Һλ������*****************************************/
 void Get_Liquid_Level()
{
	  u16 adcx;
  	adcx=Get_Adc_Average(ADC_Channel_13,40);		//�õ�ADCת��ֵ	  
 	  OLED_ShowString(0,0,"LEVEL:",16);           //ˮλ�ȼ�LEVEL
    OLED_ShowNum(48,0,adcx,4,16);               //��ʾˮλADCֵ
	  OLED_Refresh_Gram();		                    //������ʾ��OLED 
}

