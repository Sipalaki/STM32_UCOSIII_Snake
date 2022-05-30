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
	
//?????????
#define START_TASK_PRIO		3
//?????????	
#define START_STK_SIZE 		64
//????????
OS_TCB StartTaskTCB;
//??????	
CPU_STK START_TASK_STK[START_STK_SIZE];
//??????
void start_task(void *p_arg);


//?????????
#define OLED_TASK_PRIO		4
//?????????	
#define OLED_STK_SIZE 		128
//????????
OS_TCB OLEDTaskTCB;
//??????	
CPU_STK OLED_TASK_STK[OLED_STK_SIZE];
void oled_task(void *p_arg);

//?????????
#define CTR_TASK_PRIO		4
//?????????	
#define CTR_STK_SIZE 		64
//????????
OS_TCB CTRTaskTCB;
//??????	
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
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); //??????????
	I2C_Configuration();
	LED_Init();
	init(&q_x,24);
	init(&q_y,24);
	ADC_Configuration(); 
	JoyStick_Init(); 
	uart_init(115200);    //?????????????
	OSInit(&err);		//??'??UCOSIII
	OS_CRITICAL_ENTER();//?????????
	//??????'????
	OSTaskCreate((OS_TCB 	* )&StartTaskTCB,		//????????
				 (CPU_CHAR	* )"start task", 		//????????
                 (OS_TASK_PTR )start_task, 			//??????
                 (void		* )0,					//????????????????
                 (OS_PRIO	  )START_TASK_PRIO,     //?????????
                 (CPU_STK   * )&START_TASK_STK[0],	//???????????
                 (CPU_STK_SIZE)START_STK_SIZE/10,	//????????????
                 (CPU_STK_SIZE)START_STK_SIZE,		//?????????
                 (OS_MSG_QTY  )0,					//???????????????????????????????,?0???????????
                 (OS_TICK	  )0,					//??'??????????????????????0??I??????
                 (void   	* )0,					//?û?????J???
                 (OS_OPT      )OS_OPT_TASK_STK_CHK|OS_OPT_TASK_STK_CLR, //???????
                 (OS_ERR 	* )&err);				//??Ÿú?????????k????
//								 printf("%d",err);
	OS_CRITICAL_EXIT();	//????????	 
	OSStart(&err);  //????UCOSIII

  /* Infinite loop */
  while (1);
  
	
	
}

//??'??????
void start_task(void *p_arg)
{
	OS_ERR err;
	CPU_SR_ALLOC();
	p_arg = p_arg;

	CPU_Init();
#if OS_CFG_STAT_TASK_EN > 0u
   OSStatTaskCPUUsageInit(&err);  	//???????                
#endif
	
#ifdef CPU_CFG_INT_DIS_MEAS_EN		//???'???????????????
    CPU_IntDisMeasMaxCurReset();	
#endif
	
#if	OS_CFG_SCHED_ROUND_ROBIN_EN  //??'??????????????
	 //'????????????????,?????????1?????????g???1*5=5ms
	OSSchedRoundRobinCfg(DEF_ENABLED,1,&err);  
#endif		
	
	OS_CRITICAL_ENTER();	//?????????
	//????LED0????
								 
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

			 						 
	OS_TaskSuspend((OS_TCB*)&StartTaskTCB,&err);		//????'????			 
	OS_CRITICAL_EXIT();	//?????????
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
		OSTimeDlyHMSM(0,0,0,200,OS_OPT_TIME_HMSM_STRICT,&err); //???500ms    
		if(flag){
		OLED_DISPLAY_8x8(rand_y,rand_x*8,2);
		switch(vector){
		case 0:
			x = q_x.rear;
			if(x - 1 < 0){x = q_x.capacity;} else {x = x-1;}
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
			if(x - 1 < 0){x = q_x.capacity;} else {x = x-1;}
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
			if(x - 1 < 0){x = q_x.capacity;} else {x = x-1;}
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
			if(x - 1 < 0){x = q_x.capacity;} else {x = x-1;}
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
		OSTimeDlyHMSM(0,0,0,50,OS_OPT_TIME_HMSM_STRICT,&err); //???500ms
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

