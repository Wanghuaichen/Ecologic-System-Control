/*********************ͷ�ļ�������***************************************/
#include "reg52.h"
#include "oled.h"
#include "i2c.h"


/*********************define������***************************************/
#define K_up 1              //�������
#define K_down 2            //�������
#define K_corotation 3      //��ת��ֵ
#define K_reversal 4        //��ת��ֵ
/*********************λ����������***************************************/
sbit IN1=P0^2;              //�������תͨ�����ϵ���ת��
sbit IN2=P0^1;
sbit ENA=P0^0;              //ʹ�ܶ�
sbit Key_up=P3^2;           //�������
sbit Key_down=P3^3;         //�������
sbit Key_corotation=P3^4;   //�����ת
sbit Key_reversal=P3^5;     //�����ת
/*********************����������***************************************/
int turn_flag=1;            //ת����־λ��0ֹͣ��1��ת��2��ת��3�����ٶȣ�     
int n=300;                  //����ת�ٿ�ʼ��Ϊ��ռ�ձ�
int z;                      //ռ�ձȷ�ĸ
int KEY;                    //��ȡ������ֵ          
int s=0;			              //��
int min=0;			            //��
int hour=0;			            //Сʱ
int day=0;			            //��
int led_num=0;	   	        //����һ��1ms
/*********************����������***************************************/
void led_timer();						//led����ʱ����ʾ
void Key_Control();         //��������
void TimerConfiguration();  //��ʱ��0��ʼ��
void Int0Configuration();   //�ⲿ�ж�0��ʼ��
void delay_us(int i);       //��ʱ1us��12M��
int KeyScan(int mode);      //����ģʽ��0--����  1--������
void I2C_Write();           //����洢
void I2C_Read();            //���洢�����ݻָ����ϴε�������
/*********************������***************************************/
void main()                  //��������ʼ
{
	I2C_Read();                //��ȡʱ��
	OLED_Init();               //oled��ʾ����ʼ��
	TimerConfiguration();      //��ʼ����ʱ��0
	ENA=1;                     //�ϵ�����ʼת������ת��
	IN1=1;
	IN2=0;
	OLED_ShowString(16,0, ":");	          
	OLED_ShowString(40,0, ":");
	OLED_ShowString(64,0, ":");         
	OLED_ShowString(0,0,  "00");        //��
	OLED_ShowString(24,0, "00");        //ʱ
	OLED_ShowString(48,0, "00");        //��
	OLED_ShowString(72,0, "00");        //��
	
	while(1)
	{
		Key_Control();          //��������   
		led_timer();            //װ��ʹ��ʱ���¼
		
	  if(z<n)                 //ģ��PWM�����
	 	  ENA=1;
    else  
	    ENA=0; 
				 
	}
}
/*********************��ʱ��0��ʼ������***************************************/
void TimerConfiguration()
{
    TMOD = 0x01; //ѡ������ʽ1
    TH0 = 0xff;	 //���ó�ʼֵ��1ms��
    TL0 = 0xb0; 
    ET0 = 1;		 //�򿪶�ʱ��0�ж�
    TR0 = 1;		 //������ʱ��0
	  EA = 1;			 //�����ж�
}

/*********************��ʱ��0�жϺ���***************************************/
void Timer0() interrupt 1
{
	
	  TH0 = 0xff;	 //���ó�ʼֵ��1ms��
    TL0 = 0xb0;     
	  led_num++; 
	  z++;         //ռ�ձ������ڣ�300ms��
	  if(z>300)         
	  {
		   z=0;
	  }
}
/*********************�������ƺ���***************************************/
void Key_Control()   //��������
{
	KEY=KeyScan(0);    //��ȡ��ֵ
	switch(KEY)
	{
		case 0:
		{
		}
		break;
		
		case 1:
		{
			turn_flag=0;     //��ת��־
		}
		break;
		
		case 2:
		{
			turn_flag=1;     //��ת��־
		}
		break;
		
		case 3:
		{ 
			turn_flag=2;     //ֹͣ��־
		}
		break;
		
		case 4:
		{	
			turn_flag=3;     //ת�ٵ��ڱ�־
			n-=5;
		}
		break;
		
		default:
			break;
	}

	if(turn_flag==0)    //���ֹͣ
	{
		ENA=1; 
		IN1=0;
		IN2=0; 		
	}
	if(turn_flag==1)   //�����ת
	{
		ENA=1;
		IN1=1;
		IN2=0;            
	}
	if(turn_flag==2)   //�����ת
	{
		ENA=1;
		IN1=0;
		IN2=1; 
	}
	
	if(n<100)          //����ģ��PWMռ�ձ�
	{
		 n=300;
	} 
}
/*********************����ģʽ����***************************************/
int KeyScan(int mode)
{
	static int Keyen=1;
	if(mode==1)
	{
		Keyen=1;
	}
	if(Keyen==1&&(Key_up==0||Key_down==0||Key_corotation==0||Key_reversal==0))
	{
		delay_ms(10);
		Keyen=0;
		if(Key_up==0) return K_up;
		else if(Key_down==0) return K_down;
		else if(Key_corotation==0) return K_corotation;
		else if(Key_reversal==0) return K_reversal;
 	}
	else if(Key_up==1&&Key_down==1&&Key_corotation==1&&Key_reversal==1)
	{
		Keyen=1;
	}
	return 0;
}
/**********************��ʱ����***************************************/
void led_timer()			   //��ʱ����
{
	if(led_num==12500)
	{
		s++;
		I2C_Write();
		led_num=0;
		OLED_ShowNum(72,0,s,2,16);
		if(s==60)
		{
			s=0;
			min++;
			OLED_ShowNum(48,0,min,2,16);
			if(min==60)
			{
				min=0;
				hour++;
				OLED_ShowNum(24,0,hour,2,16);
				if(hour==60)
				{
					hour=0;
					day++;
					OLED_ShowNum(0,0,day,2,16);
					if(day==30)
					{
						day=0;
					}
				}
			}
		}
	}		
}

void I2C_Write()
{
	At24c02Write(1,s);      //������
	At24c02Write(2,min);    //�����
	At24c02Write(3,hour);   //����ʱ
	At24c02Write(4,day);    //������
}

void I2C_Read()
{
	s=At24c02Read(1);       //������
	min=At24c02Read(2);     //�����
	hour=At24c02Read(3);    //����ʱ
	day=At24c02Read(4);     //������

/*********************1us��ʱ����***************************************/
void delay_us(int i)  //��ʱ����1us��12M����
{
	while(i--);         //ִ��һ��1us
}

