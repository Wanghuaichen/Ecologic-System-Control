/*********************头文件定义区***************************************/
#include "reg52.h"
#include "oled.h"
#include "i2c.h"


/*********************define定义区***************************************/
#define K_up 1              //电机加速
#define K_down 2            //电机减速
#define K_corotation 3      //正转键值
#define K_reversal 4        //反转键值
/*********************位操作定义区***************************************/
sbit IN1=P0^2;              //电机正反转通道（上电正转）
sbit IN2=P0^1;
sbit ENA=P0^0;              //使能端
sbit Key_up=P3^2;           //电机加速
sbit Key_down=P3^3;         //电机减速
sbit Key_corotation=P3^4;   //电机正转
sbit Key_reversal=P3^5;     //电机反转
/*********************变量定义区***************************************/
int turn_flag=1;            //转动标志位（0停止，1正转，2反转，3调节速度）     
int n=300;                  //调节转速开始是为满占空比
int z;                      //占空比分母
int KEY;                    //读取按键键值          
int s=0;			              //秒
int min=0;			            //分
int hour=0;			            //小时
int day=0;			            //天
int led_num=0;	   	        //计数一次1ms
/*********************函数定义区***************************************/
void led_timer();						//led点亮时间显示
void Key_Control();         //按键控制
void TimerConfiguration();  //定时器0初始化
void Int0Configuration();   //外部中断0初始化
void delay_us(int i);       //延时1us（12M）
int KeyScan(int mode);      //按键模式（0--单按  1--连按）
void I2C_Write();           //掉电存储
void I2C_Read();            //将存储的数据恢复到上次掉电数据
/*********************主函数***************************************/
void main()                  //主函数开始
{
	I2C_Read();                //读取时间
	OLED_Init();               //oled显示屏初始化
	TimerConfiguration();      //初始化定时器0
	ENA=1;                     //上电电机开始转动（正转）
	IN1=1;
	IN2=0;
	OLED_ShowString(16,0, ":");	          
	OLED_ShowString(40,0, ":");
	OLED_ShowString(64,0, ":");         
	OLED_ShowString(0,0,  "00");        //天
	OLED_ShowString(24,0, "00");        //时
	OLED_ShowString(48,0, "00");        //分
	OLED_ShowString(72,0, "00");        //秒
	
	while(1)
	{
		Key_Control();          //按键控制   
		led_timer();            //装置使用时间记录
		
	  if(z<n)                 //模拟PWM波输出
	 	  ENA=1;
    else  
	    ENA=0; 
				 
	}
}
/*********************定时器0初始化函数***************************************/
void TimerConfiguration()
{
    TMOD = 0x01; //选择工作方式1
    TH0 = 0xff;	 //设置初始值（1ms）
    TL0 = 0xb0; 
    ET0 = 1;		 //打开定时器0中断
    TR0 = 1;		 //启动定时器0
	  EA = 1;			 //打开总中断
}

/*********************定时器0中断函数***************************************/
void Timer0() interrupt 1
{
	
	  TH0 = 0xff;	 //设置初始值（1ms）
    TL0 = 0xb0;     
	  led_num++; 
	  z++;         //占空比总周期（300ms）
	  if(z>300)         
	  {
		   z=0;
	  }
}
/*********************按键控制函数***************************************/
void Key_Control()   //按键控制
{
	KEY=KeyScan(0);    //读取键值
	switch(KEY)
	{
		case 0:
		{
		}
		break;
		
		case 1:
		{
			turn_flag=0;     //正转标志
		}
		break;
		
		case 2:
		{
			turn_flag=1;     //反转标志
		}
		break;
		
		case 3:
		{ 
			turn_flag=2;     //停止标志
		}
		break;
		
		case 4:
		{	
			turn_flag=3;     //转速调节标志
			n-=5;
		}
		break;
		
		default:
			break;
	}

	if(turn_flag==0)    //电机停止
	{
		ENA=1; 
		IN1=0;
		IN2=0; 		
	}
	if(turn_flag==1)   //电机正转
	{
		ENA=1;
		IN1=1;
		IN2=0;            
	}
	if(turn_flag==2)   //电机反转
	{
		ENA=1;
		IN1=0;
		IN2=1; 
	}
	
	if(n<100)          //控制模拟PWM占空比
	{
		 n=300;
	} 
}
/*********************按键模式函数***************************************/
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
/**********************计时函数***************************************/
void led_timer()			   //计时函数
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
	At24c02Write(1,s);      //存入秒
	At24c02Write(2,min);    //存入分
	At24c02Write(3,hour);   //存入时
	At24c02Write(4,day);    //存入天
}

void I2C_Read()
{
	s=At24c02Read(1);       //存入秒
	min=At24c02Read(2);     //存入分
	hour=At24c02Read(3);    //存入时
	day=At24c02Read(4);     //存入天

/*********************1us延时函数***************************************/
void delay_us(int i)  //延时函数1us（12M晶振）
{
	while(i--);         //执行一次1us
}

