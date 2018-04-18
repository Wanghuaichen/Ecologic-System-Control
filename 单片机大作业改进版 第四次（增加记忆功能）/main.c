/*******************************************************************************
						       使用资源
							上按键：P2^2
							下按键：P2^3
							左按键：P2^4
							右按键：P2^5
							确认键：P2^6
							调节亮度：P1^0
							热释电控制：P1^7
							OLED：
										时钟 D0（SCLK）：P0^3
										D1（MOSI） 数据：P0^4
										RST：P0^5
										DS：P0^6
										CS：P0^7（片选一般直接接地）
					    24C02掉点不丢失：
							      SCL：P2^1
										SDA：P2^0
*******************************************************************************/



/*******************************************************************************
							引用头文件定义
*******************************************************************************/
#include "reg52.h"			 //此文件中定义了单片机的一些特殊功能寄存器				
#include "oled.h"			   //此文件中定义了显示电路函数
#include "interrupt.h"	 //此文件中定义了单片机所有中断函数及中断服务函数
#include "i2c.h"         //此文件中定义了24C02存储读取函数

/*******************************************************************************
							键值定义区
*******************************************************************************/

#define K_up 1          //上按键赋值1
#define K_down 2        //下按键赋值2
#define K_left 3        //左按键赋值3
#define K_right 4       //右按键赋值4
#define K_ok 5          //确认按键赋值5

/*******************************************************************************
							变量定义
*******************************************************************************/
int s=0;			         //秒
int min=0;			       //分
int hour=0;			       //小时
int day=0;			       //天
int led_num=0;	   	   //计数一次1ms
int count_num=0;       //设置预约时间
int set_s=0;	         //设置秒
int set_min=0;	       //设置分钟
int rather_s=0;        //比较秒
int led_replay;		     //重复led值
int pwm_regulation=0;  //PWM调节值
int pwm_num=0;		     //PWM值
int key;               //键值

/*******************************************************************************
							引脚定义
*******************************************************************************/
sbit key_ok=P2^6;			     //模式1确认键
sbit key_up=P2^2;			     //上按键	  
sbit key_down=P2^3;			   //下按键
sbit key_left=P2^4;			   //左按键
sbit key_right=P2^5;		   //右按键
sbit pwm=P1^0;				     //调节亮度
sbit people_control=P1^7;	 //热释电

/*******************************************************************************
							函数声明
*******************************************************************************/
void led_timer();						            //led点亮时间显示
void delayms(unsigned int z);			      //延时函数
int KeyScan(int mode);						      //选择按键模式
void oled_show();                       //oled显示界面
void count_timer();                     //计数时间
void pwm_regulation_led_luminance();    //调节led亮度（调节精度每次5%）
void Key_Control();                     //按键控制
void I2C_Write();                       //掉电存储
void I2C_Read();                        //将存储的数据恢复到上次掉电数据
void I2C_Write_zero();                  //将存储数据清零

/*******************************************************************************
							标志位定义
*******************************************************************************/
int key_up_flag=0;					  //上按键标志位
int key_down_flag=0;				  //下按键标志位
int key_left_flag=0;				  //左按键标志位
int key_right_flag=0;				  //右按键标志位
int led_switch_flag=1;			  //led开关标志位
int function_choose_flag=0;	  //功能选择标志位
int timer_flag=1;					    //计时开始标志位
int key_ok_flag=0;					  //确认标志位

/*******************************************************************************
							主函数运行
*******************************************************************************/
void main()
{
//	I2C_Read(); 
  Timer0Init();	    
//	UsartInit();		            //串口中断初始化
	OLED_Init();	                //oled显示屏初始化
	
	OLED_ShowString(16,0,":");	       
	OLED_ShowString(40,0,":");
	OLED_ShowString(64,0,":");         
	OLED_ShowString(0,0,"00");         //天
	OLED_ShowString(24,0,"00");        //时
	OLED_ShowString(48,0,"00");        //分
	OLED_ShowString(72,0,"00");        //秒
	OLED_ShowString(16,2,"mode:");     //模式
	OLED_ShowString(16,4,"set_s:");    //设置预约
	OLED_ShowString(64,6,"000");       //显示预约时间过了多久
	OLED_ShowString(64,4,"000");       //设置预约时间
	OLED_ShowString(0,6,"000");        //PWM调节灯光亮度（5%）
	OLED_ShowNum(56,2,function_choose_flag,1,16);   //此时设置某个模式
	OLED_ShowNum(72,0,s,2,16);         //秒
	OLED_ShowNum(48,0,min,2,16);       //分
	OLED_ShowNum(24,0,hour,2,16);      //时
	OLED_ShowNum(0,0,day,2,16);        //天
	while(1)					           //死循环
	{	
	led_replay=pwm_regulation;   //重复定义led目前的状态
	Key_Control();               //按键控制
	if(key_up_flag==1)			     //判断模式
	{
		function_choose_flag++;
		
		if(function_choose_flag>4)
		{
			function_choose_flag=0;
		}
		OLED_ShowNum(56,2,function_choose_flag,1,16);
	}

	if(key_down_flag==1)
	{
		function_choose_flag--;
		
		if(function_choose_flag<0)
		{
			function_choose_flag=4;
		}
		OLED_ShowNum(56,2,function_choose_flag,1,16);
	}

	if(function_choose_flag==0)			//模式0，左按键led灭，定时器0关闭；
	{									              //右按键led亮，定时器0开启；
		if(led_switch_flag==1)
		{
			pwm_regulation=10;
			TR0=1;	
		}
		else if(led_switch_flag==0)
		{
			pwm_regulation=0;
			TR0=0;
		}
	}
	else if(function_choose_flag==1)   //模式1，上按键加延时时间，下按键减延时时间，确认键进行确认，led亮变灭，灭变亮；
	{
		if(key_right_flag==1)
		{
			set_s++;
			if(set_s<5)
			{
				set_s=600;
			}
			if(set_s>600)
			{
				set_s=5;
			}
			OLED_ShowNum(64,4,set_s,4,16);				
		}
		if(key_left_flag==1)
		{
			set_s--;
			if(set_s>600)
			{
				set_s=5;
			}
			if(set_s<5)
			{
				set_s=600;
			}
			OLED_ShowNum(64,4,set_s,4,16);
		}
		if(key_ok_flag==1&&pwm_regulation==0)
		{	
			if(set_s==rather_s)
			{
				pwm_regulation=10;
				rather_s=0;
				key_ok_flag=0;
				TR0=1;
			}	
		}
		else if(key_ok_flag==1&&pwm_regulation==10)
		{
			if(set_s==rather_s)
			{
				pwm_regulation=0;
				rather_s=0;
				key_ok_flag=0;
				TR0=0;
			}
		}
	}

	else if(function_choose_flag==2)	  //模式2，热释电控制，监测到人时led亮
	{
		if(people_control==1)
		{
			pwm_regulation=10;
			TR0=1;
		}
		else if(people_control==0)
		{
			pwm_regulation=0;
			TR0=0;	
		}
	}

	else if(function_choose_flag==3)	  //模式3，调节led亮度
	{
		if(key_right_flag==1)
		{
			pwm_regulation+=1;
			if(pwm_regulation>10)
			{
				pwm_regulation=0;
			}
			OLED_ShowNum(0,6,pwm_regulation,3,16);	
		}
		if(key_left_flag==1)
		{
			pwm_regulation-=1;
			if(pwm_regulation<0)
			{
				pwm_regulation=10;
			}
			OLED_ShowNum(0,6,pwm_regulation,3,16);
		}
	}
		else if(function_choose_flag==4)	  //模式4，调节led亮度
		{
			if(key_ok_flag==1)
			{
			I2C_Write_zero();
			I2C_Read();
			OLED_ShowNum(72,0,s,2,16);
			OLED_ShowNum(48,0,min,2,16);
			OLED_ShowNum(24,0,hour,2,16);
			OLED_ShowNum(0,0,day,2,16);
			key_ok_flag=0;
			}
		}
		  led_timer();
			pwm_regulation_led_luminance();   	
	}
}

/*******************************************************************************
* 函 数 名         : Key_Control()
* 函数功能		     :按键扫描控制
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
void Key_Control()
{
	key=KeyScan(0);
	switch(key)		     //按键扫描返回不同值
	{
		case 0:
			{	key_up_flag=0;
				key_down_flag=0;
				key_left_flag=0;
				key_right_flag=0;
			}	break;
		case 1:
				key_up_flag=1;
				break;
		case 2:
				key_down_flag=1;
				break; 
		case 3:
				key_left_flag=1;
				led_switch_flag=0;
				break;
		case 4:
				key_right_flag=1;
				led_switch_flag=1;
				break;
		case 5:
				key_ok_flag=1;
				TR0=1;
				break;
		default:
				break;		
	}

}

/*******************************************************************************
* 函 数 名         : delayms()
* 函数功能		   :延时函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
void delayms(unsigned int z)//延时函数1ms（12M晶振）
{
	unsigned int x,y;
	for(x=z; x>0; x--)
	for(y=1848; y>0; y--);
}

/*******************************************************************************
* 函 数 名         :int KeyScan(int mode)
* 函数功能		     :按键模式选择函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
int KeyScan(int mode)
{
	static int Keyen=1;
	if(mode==1)
	{
		Keyen=1;
	}
	if(Keyen==1&&(key_up==0||key_down==0||key_left==0||key_right==0||key_ok==0))
	{
		delayms(10);
		Keyen=0;
		if(key_up==0) return K_up;
		else if(key_down==0) return K_down;
		else if(key_left==0) return K_left;
		else if(key_right==0) return K_right;
		else if(key_ok==0) return K_ok;
 	}
	else if(key_up==1&&key_down==1&&key_left==1&&key_right==1&&key_ok==1)
	{
		Keyen=1;
	}
	return 0;
}

/*******************************************************************************
* 函 数 名         :  I2C_Write_zero()
* 函数功能		   :延时函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
void I2C_Write_zero()
{
	At24c02Write(1,0);
	At24c02Write(2,0);
	At24c02Write(3,0);
	At24c02Write(4,0);
}

/*******************************************************************************
* 函 数 名         :  I2C_Write()
* 函数功能		   :延时函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
void I2C_Write()
{
	At24c02Write(1,s);
	At24c02Write(2,min);
	At24c02Write(3,hour);
	At24c02Write(4,day);
}

/*******************************************************************************
* 函 数 名         : I2C_Read()
* 函数功能		   :延时函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
void I2C_Read()
{
	s=At24c02Read(1);
	min=At24c02Read(2);
	hour=At24c02Read(3);
	day=At24c02Read(4);
}

 /*******************************************************************************
* 函 数 名         : len_timer()
* 函数功能		   : 计时显示函数
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
void led_timer()			   //计时函数
{
    if(count_num==12500)
	{
		rather_s++;	
		count_num=0;
		OLED_ShowNum(64,6,rather_s,4,16);
	}

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
				I2C_Write();
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


  /*******************************************************************************
* 函 数 名         : pwm_regulation_led_luminance() 
* 函数功能		   : PWM调节亮度
* 输    入         : 无
* 输    出         : 无
*******************************************************************************/
void pwm_regulation_led_luminance()		//调节led光亮
{
	if(pwm_num>10)			                //10乘1ms（周期）
	{
		pwm_num=0;
	}		
	if(pwm_num < pwm_regulation)	      //改变这个值可以改变直流电机的速度
	{
		pwm=1;
	}
	else
	{
		pwm=0;
		
	}
}

