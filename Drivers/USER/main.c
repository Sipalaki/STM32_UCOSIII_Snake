#include "led.h"
#include "includes.h"
#include "systemclock.h"
#include "usart.h"
#include "oled0561.h"
#include "lm75a.h"
#include "adc.h"
#include "JoyStick.h"
#include "queue.h"
#include "lib_math.h"
#include "stdlib.h"
/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
	
//任务优先级
#define START_TASK_PRIO		3
//任务堆栈大小	
#define START_STK_SIZE 		64
//任务控制块
OS_TCB StartTaskTCB;
//任务堆栈	
CPU_STK START_TASK_STK[START_STK_SIZE];
//任务函数
void start_task(void *p_arg);


//任务优先级
#define OLED_TASK_PRIO		4
//任务堆栈大小	
#define OLED_STK_SIZE 		128
//任务控制块
OS_TCB OLEDTaskTCB;
//任务堆栈	
CPU_STK OLED_TASK_STK[OLED_STK_SIZE];
void oled_task(void *p_arg);

//任务优先级
#define CTR_TASK_PRIO		4
//任务堆栈大小	
#define CTR_STK_SIZE 		64
//任务控制块
OS_TCB CTRTaskTCB;
//任务堆栈	
CPU_STK CTR_TASK_STK[CTR_STK_SIZE];
void ctr_task(void *p_arg);

extern vu16 ADC_DMA_IN[2];
struct Queue q_x; 
struct Queue q_y;
u8 vector = 0;

u8 score = 0;

int main(void)
{
 
  /* Add your application code here
     */
	OS_ERR err;
	CPU_SR_ALLOC();
	
	systemdelay_init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //中断分组配置
	I2C_Configuration();
	LED_Init();
	init(&q_x,16);
	init(&q_y,16);
	ADC_Configuration(); 
	JoyStick_Init(); 
	uart_init(115200);    //串口波特率设置
	OSInit(&err);		//初始化UCOSIII
	OS_CRITICAL_ENTER();//进入临界区
	//创建开始任务
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//任务控制块
				 (CPU_CHAR	* )"start task", 		//任务名字
                 (OS_TASK_PTR )start_task, 			//任务函数
                 (void		* )0,					//传递给任务函数的参数
                 (OS_PRIO	  )START_TASK_PRIO,     //任务优先级
                 (CPU_STK   * )&START_TASK_STK[0],	//任务堆栈基地址
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//任务堆栈深度限位
                 (CPU_STK_SIZE)START_STK_SIZE,		//任务堆栈大小
                 (OS_MSG_QTY  )0,					//任务内部消息队列能够接收的最大消息数目,为0时禁止接收消息
                 (OS_TICK	  )0,					//当使能时间片轮转时的时间片长度，为0时为默认长度，
                 (void   	* )0,					//用户补充的存储区
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //任务选项
                 (OS_ERR 	* )&err);				//存放该函数错误时的返回值
//								 printf("%d",err);
	OS_CRITICAL_EXIT();	//退出临界区	 
	OSStart(&err);  //开启UCOSIII

  /* Infinite loop */
  while (1);
  
	
	
}

//开始任务函数
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//统计任务                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//如果使能了测量中断关闭时间
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //当使用时间片轮转的时候
	 //使能时间片轮转调度功能,时间片长度为1个系统时钟节拍，既1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		
	
	OS_CRITICAL_ENTER();	//进入临界区
	//创建LED0任务
								 
	OSTaskCreate((OS_TCB 	* )&OLEDTaskTCB,		
				 (CPU_CHAR	* )"oled task", 		
                 (OS_TASK_PTR )oled_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )OLED_TASK_PRIO,     
                 (CPU_STK   * )&OLED_TASK_STK[0],	
                 (CPU_STK_SIZE)OLED_STK_SIZE/10,	
                 (CPU_STK_SIZE)OLED_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);		
								 
	OSTaskCreate((OS_TCB 	* )&CTRTaskTCB,		
				 (CPU_CHAR	* )"ctr task", 		
                 (OS_TASK_PTR )ctr_task, 			
                 (void		* )0,					
                 (OS_PRIO	  )CTR_TASK_PRIO,     
                 (CPU_STK   * )&CTR_TASK_STK[0],	
                 (CPU_STK_SIZE)CTR_STK_SIZE/10,	
                 (CPU_STK_SIZE)CTR_STK_SIZE,		
                 (OS_MSG_QTY  )0,					
                 (OS_TICK	  )0,					
                 (void   	* )0,					
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR,
                 (OS_ERR 	* )&err);		

			 						 
	OS_TaskSuspend((OS_TCB*)&StartTaskTCB,&err);		//挂起开始任务			 
	OS_CRITICAL_EXIT();	//进入临界区
}


void oled_task(void *p_arg)
{
	OS_ERR err;
	u8 x = 0;
	u8 pos_x = 0,pos_y = 0;
	u8 rand_x,rand_y;
	u8 flag = 1;
	p_arg = p_arg;
	delay_ms(100);
	OLED0561_Init();
	enQueue(&q_x,0);
	enQueue(&q_y,0);
	enQueue(&q_x,8);
	enQueue(&q_y,0);
	enQueue(&q_x,16);
	enQueue(&q_y,0);
	OLED_DISPLAY_8x8(0,0,0);
	OLED_DISPLAY_8x8(0,8,0);
	OLED_DISPLAY_8x8(0,16,0);
	rand_x = rand()%16;
	rand_y = rand()%8;
	while(1)
	{
		OSTimeDlyHMSM(0,0,0,200,OS_OPT_TIME_HMSM_STRICT,&err); //延时500ms    
		if(flag){
		OLED_DISPLAY_8x8(rand_y,rand_x*8,2);
		switch(vector){
		case 0:
			x = q_x.rear;
			if(x - 1 < 0){x = 16;} else {x = x-1;}
			if(q_x.data[x]+8>127)
				{ enQueue(&q_x,0);
					OLED_DISPLAY_8x8(q_y.data[x],0,0);
					pos_x = 0;
					pos_y = q_y.data[x];
				}
			else
				{ enQueue(&q_x,(q_x.data[x]+8)%128);
					OLED_DISPLAY_8x8((q_y.data[x]),(q_x.data[x]+8)%128,0);
					pos_x = (q_x.data[x]+8)%128;
					pos_y = q_y.data[x];
				}
			enQueue(&q_y,(q_y.data[x]));
			break;
		case 1:
			x = q_x.rear;
			if(x - 1 < 0){x = 16;} else {x = x-1;}
			if(q_x.data[x]-8<0)
				{ enQueue(&q_x,120);
					OLED_DISPLAY_8x8(q_y.data[x],120,0);
					pos_x = 120;
					pos_y = q_y.data[x];
				}
			else
				{ enQueue(&q_x,(q_x.data[x]-8)%128);
					OLED_DISPLAY_8x8((q_y.data[x]),(q_x.data[x]-8)%128,0);
					pos_x = (q_x.data[x]-8)%128;
					pos_y = q_y.data[x];
				}
			enQueue(&q_y,(q_y.data[x]));
			
			break;
		case 2:
			x = q_x.rear;
			if(x - 1 < 0){x = 16;} else {x = x-1;}
			if(q_y.data[x]-1<0)
				{ enQueue(&q_y,7);
					OLED_DISPLAY_8x8(7,q_x.data[x],0);
					pos_x = q_x.data[x];
					pos_y = 7;
				}
			else
				{ enQueue(&q_y,(q_y.data[x]-1)%8);
					OLED_DISPLAY_8x8((q_y.data[x]-1)%8,q_x.data[x],0);
					pos_x = q_x.data[x];
					pos_y = (q_y.data[x]-1)%8;
				}
			enQueue(&q_x,(q_x.data[x]));
			break;
		case 3:
			x = q_x.rear;
			if(x - 1 < 0){x = 16;} else {x = x-1;}
			if(q_y.data[x]+1>7)
				{ enQueue(&q_y,0);
					OLED_DISPLAY_8x8(0,q_x.data[x],0);
					pos_x = q_x.data[x];
					pos_y = 0;
				}
			else
				{ enQueue(&q_y,(q_y.data[x]+1)%8);
					OLED_DISPLAY_8x8((q_y.data[x]+1)%8,q_x.data[x],0);
					pos_x = q_x.data[x];
					pos_y = (q_y.data[x]+1)%8;
				}
			enQueue(&q_x,(q_x.data[x]));
			break;
	
	} 
		if(searchQueue(pos_x,pos_y,&q_x,&q_y)){
			OLED_DISPLAY_CLEAR();
			OLED_DISPLAY_8x16_BUFFER(2,"   GAME OVER");
			OLED_DISPLAY_8x16_BUFFER(5," 	 SCORE:");
			OLED_DISPLAY_8x16(5,80,score/10+0x30);
			OLED_DISPLAY_8x16(5,88,score%10+0x30);
			flag = 0;
			score = 0;
	}
		else if(pos_x == rand_x*8 && pos_y == rand_y){
			rand_x = rand()%16;
			rand_y = rand()%8;
			score++;
	}else{
			OLED_DISPLAY_8x8(q_y.data[q_y.front],q_x.data[q_x.front],1);
			deQueue(&q_x);
			deQueue(&q_y);
	}
	}
	if(GPIO_ReadInputDataBit(JoyStickPORT,JoyStick_KEY)==0){
		OLED_DISPLAY_CLEAR();
		flag = 1;
		clearQueue(&q_x,&q_y);
		enQueue(&q_x,0);
		enQueue(&q_y,0);
		enQueue(&q_x,8);
		enQueue(&q_y,0);
		enQueue(&q_x,16);
		enQueue(&q_y,0);
		OLED_DISPLAY_8x8(0,0,0);
		OLED_DISPLAY_8x8(0,8,0);
		OLED_DISPLAY_8x8(0,16,0);
		vector = 0;
		rand_x = rand()%16;
		rand_y = rand()%8;
	}
	}

}

void ctr_task(void *p_arg)
{
	OS_ERR err;
	p_arg = p_arg;
	while(1)
	{
		OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_HMSM_STRICT,&err); //延时500ms
		if(ADC_DMA_IN[0]>3000){
				vector = 0;
		}else if(ADC_DMA_IN[0]<1000){
				vector = 1;
		}else if(ADC_DMA_IN[1]>3000){
				vector = 2;
		}else if(ADC_DMA_IN[1]<1000){
				vector = 3;
		}
	}
}


