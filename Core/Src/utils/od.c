#include <stm32f429xx.h>

void od_clock_init(void){
	 RCC->APB1ENR |= RCC_APB1ENR_PWREN;

	 PWR->CR &= ~(3UL << 14);
	 PWR->CR |= (3UL << 14);
	 RCC->CR |= RCC_CR_HSEON;

	 while ((RCC->CR & RCC_CR_HSERDY) == 0U){}

	 RCC->PLLCFGR =
			(8UL << RCC_PLLCFGR_PLLM_Pos)
	      | (360UL << RCC_PLLCFGR_PLLN_Pos)
	      | (0UL << RCC_PLLCFGR_PLLP_Pos)
	      | RCC_PLLCFGR_PLLSRC_HSE;

	 RCC->CR |= RCC_CR_PLLON;
	 PWR->CR |= PWR_CR_ODEN;

	 while ((PWR->CSR & PWR_CSR_ODRDY) == 0U){}

	 PWR->CR |= PWR_CR_ODSWEN;

	 while ((PWR->CSR & PWR_CSR_ODSWRDY) == 0U){}

	 FLASH->ACR =
	      FLASH_ACR_ICEN
	    | FLASH_ACR_DCEN
	    | FLASH_ACR_PRFTEN
	    | FLASH_ACR_LATENCY_5WS;
	 RCC->CFGR &= ~(RCC_CFGR_HPRE |
	                RCC_CFGR_PPRE1 |
	                RCC_CFGR_PPRE2);
	 RCC->CFGR |= RCC_CFGR_HPRE_DIV1
	           | RCC_CFGR_PPRE1_DIV4
	           | RCC_CFGR_PPRE2_DIV2;

	 while ((RCC->CR & RCC_CR_PLLRDY) == 0U){}

	 RCC->CFGR &= ~RCC_CFGR_SW;
	 RCC->CFGR |= RCC_CFGR_SW_PLL;

	 while ((RCC->CFGR & RCC_CFGR_SWS)!= RCC_CFGR_SWS_PLL){}
}
