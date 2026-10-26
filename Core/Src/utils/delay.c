#include <stm32f429xx.h>

void systick_delay(uint32_t ms)
{
    for (uint32_t i = 0U; i < ms; i++)
    {
    	SysTick->LOAD = 22499U;
    	SysTick->VAL = 0U;
    	SysTick->CTRL = SysTick_CTRL_ENABLE_Msk;
    	while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0U){}
    	SysTick->CTRL = 0U;
    }
}

