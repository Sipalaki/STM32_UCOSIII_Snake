#ifndef __SYSTEMCLOCK_H
#define __SYSTEMCLOCK_H

#include "stm32f10x.h"
#include <includes.h>

void systemdelay_init(void);
void delay_us(u32 nus);
void delay_ms(u16 nms);
void delay_osschedlock(void);
void delay_osschedunlock(void);
void delay_ostimedly(u32 ticks);


#endif













