/*******************************************************************************
						       ʹ����Դ
							�ϰ�����P2^2
							�°�����P2^3
							�󰴼���P2^4
							�Ұ�����P2^5
							ȷ�ϼ���P2^6
							�������ȣ�P1^0
							���͵���ƣ�P1^7
							OLED��
										ʱ�� D0��SCLK����P0^3
										D1��MOSI�� ���ݣ�P0^4
										RST��P0^5
										DS��P0^6
										CS��P0^7��Ƭѡһ��ֱ�ӽӵأ�
					    24C02���㲻��ʧ��
							      SCL��P2^1
										SDA��P2^0
*******************************************************************************/



/*******************************************************************************
							����ͷ�ļ�����
*******************************************************************************/
#include "reg52.h"			 //���ļ��ж����˵�Ƭ����һЩ���⹦�ܼĴ���				
#include "oled.h"			   //���ļ��ж�������ʾ��·����
#include "interrupt.h"	 //���ļ��ж����˵�Ƭ�������жϺ������жϷ�����
#include "i2c.h"         //���ļ��ж�����24C02�洢��ȡ����

/*******************************************************************************
							��ֵ������
*******************************************************************************/

#define K_up 1          //�ϰ�����ֵ1
#define K_down 2        //�°�����ֵ2
#define K_left 3        //�󰴼���ֵ3
#define K_right 4       //�Ұ�����ֵ4
#define K_ok 5          //ȷ�ϰ�����ֵ5

/*******************************************************************************
							��������
*******************************************************************************/
int s=0;			         //��
int min=0;			       //��
int hour=0;			       //Сʱ
int day=0;			       //��
int led_num=0;	   	   //����һ��1ms
int count_num=0;       //����ԤԼʱ��
int set_s=0;	         //������
int set_min=0;	       //���÷���
int rather_s=0;        //�Ƚ���
int led_replay;		     //�ظ�ledֵ
int pwm_regulation=0;  //PWM����ֵ
int pwm_num=0;		     //PWMֵ
int key;               //��ֵ

/*******************************************************************************
							���Ŷ���
*******************************************************************************/
sbit key_ok=P2^6;			     //ģʽ1ȷ�ϼ�
sbit key_up=P2^2;			     //�ϰ���	  
sbit key_down=P2^3;			   //�°���
sbit key_left=P2^4;			   //�󰴼�
sbit key_right=P2^5;		   //�Ұ���
sbit pwm=P1^0;				     //��������
sbit people_control=P1^7;	 //���͵�

/*******************************************************************************
							��������
*******************************************************************************/
void led_timer();						            //led����ʱ����ʾ
void delayms(unsigned int z);			      //��ʱ����
int KeyScan(int mode);						      //ѡ�񰴼�ģʽ
void oled_show();                       //oled��ʾ����
void count_timer();                     //����ʱ��
void pwm_regulation_led_luminance();    //����led���ȣ����ھ���ÿ��5%��
void Key_Control();                     //��������
void I2C_Write();                       //����洢
void I2C_Read();                        //���洢�����ݻָ����ϴε�������
void I2C_Write_zero();                  //���洢��������

/*******************************************************************************
							��־λ����
*******************************************************************************/
int key_up_flag=0;					  //�ϰ�����־λ
int key_down_flag=0;				  //�°�����־λ
int key_left_flag=0;				  //�󰴼���־λ
int key_right_flag=0;				  //�Ұ�����־λ
int led_switch_flag=1;			  //led���ر�־λ
int function_choose_flag=0;	  //����ѡ���־λ
int timer_flag=1;					    //��ʱ��ʼ��־λ
int key_ok_flag=0;					  //ȷ�ϱ�־λ

/*******************************************************************************
							����������
*******************************************************************************/
void main()
{
//	I2C_Read(); 
  Timer0Init();	    
//	UsartInit();		            //�����жϳ�ʼ��
	OLED_Init();	                //oled��ʾ����ʼ��
	
	OLED_ShowString(16,0,":");	       
	OLED_ShowString(40,0,":");
	OLED_ShowString(64,0,":");         
	OLED_ShowString(0,0,"00");         //��
	OLED_ShowString(24,0,"00");        //ʱ
	OLED_ShowString(48,0,"00");        //��
	OLED_ShowString(72,0,"00");        //��
	OLED_ShowString(16,2,"mode:");     //ģʽ
	OLED_ShowString(16,4,"set_s:");    //����ԤԼ
	OLED_ShowString(64,6,"000");       //��ʾԤԼʱ����˶��
	OLED_ShowString(64,4,"000");       //����ԤԼʱ��
	OLED_ShowString(0,6,"000");        //PWM���ڵƹ����ȣ�5%��
	OLED_ShowNum(56,2,function_choose_flag,1,16);   //��ʱ����ĳ��ģʽ
	OLED_ShowNum(72,0,s,2,16);         //��
	OLED_ShowNum(48,0,min,2,16);       //��
	OLED_ShowNum(24,0,hour,2,16);      //ʱ
	OLED_ShowNum(0,0,day,2,16);        //��
	while(1)					           //��ѭ��
	{	
	led_replay=pwm_regulation;   //�ظ�����ledĿǰ��״̬
	Key_Control();               //��������
	if(key_up_flag==1)			     //�ж�ģʽ
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

	if(function_choose_flag==0)			//ģʽ0���󰴼�led�𣬶�ʱ��0�رգ�
	{									              //�Ұ���led������ʱ��0������
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
	else if(function_choose_flag==1)   //ģʽ1���ϰ�������ʱʱ�䣬�°�������ʱʱ�䣬ȷ�ϼ�����ȷ�ϣ�led�������������
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

	else if(function_choose_flag==2)	  //ģʽ2�����͵���ƣ���⵽��ʱled��
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

	else if(function_choose_flag==3)	  //ģʽ3������led����
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
		else if(function_choose_flag==4)	  //ģʽ4������led����
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
* �� �� ��         : Key_Control()
* ��������		     :����ɨ�����
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/
void Key_Control()
{
	key=KeyScan(0);
	switch(key)		     //����ɨ�践�ز�ֵͬ
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
* �� �� ��         : delayms()
* ��������		   :��ʱ����
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/
void delayms(unsigned int z)//��ʱ����1ms��12M����
{
	unsigned int x,y;
	for(x=z; x>0; x--)
	for(y=1848; y>0; y--);
}

/*******************************************************************************
* �� �� ��         :int KeyScan(int mode)
* ��������		     :����ģʽѡ����
* ��    ��         : ��
* ��    ��         : ��
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
* �� �� ��         :  I2C_Write_zero()
* ��������		   :��ʱ����
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/
void I2C_Write_zero()
{
	At24c02Write(1,0);
	At24c02Write(2,0);
	At24c02Write(3,0);
	At24c02Write(4,0);
}

/*******************************************************************************
* �� �� ��         :  I2C_Write()
* ��������		   :��ʱ����
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/
void I2C_Write()
{
	At24c02Write(1,s);
	At24c02Write(2,min);
	At24c02Write(3,hour);
	At24c02Write(4,day);
}

/*******************************************************************************
* �� �� ��         : I2C_Read()
* ��������		   :��ʱ����
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/
void I2C_Read()
{
	s=At24c02Read(1);
	min=At24c02Read(2);
	hour=At24c02Read(3);
	day=At24c02Read(4);
}

 /*******************************************************************************
* �� �� ��         : len_timer()
* ��������		   : ��ʱ��ʾ����
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/
void led_timer()			   //��ʱ����
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
* �� �� ��         : pwm_regulation_led_luminance() 
* ��������		   : PWM��������
* ��    ��         : ��
* ��    ��         : ��
*******************************************************************************/
void pwm_regulation_led_luminance()		//����led����
{
	if(pwm_num>10)			                //10��1ms�����ڣ�
	{
		pwm_num=0;
	}		
	if(pwm_num < pwm_regulation)	      //�ı����ֵ���Ըı�ֱ��������ٶ�
	{
		pwm=1;
	}
	else
	{
		pwm=0;
		
	}
}

