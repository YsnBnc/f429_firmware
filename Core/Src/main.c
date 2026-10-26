#include "od.h"
#include "ltdc.h"
#include "usart.h"
#include "delay.h"
#include "cli.h"
#include <stm32f429xx.h>

char line[64];

int main(void){
	//Do NOT change load order
	od_clock_init();
	LTDC_INIT();
	USART_INIT();

	while(1){
		if(usart_readline(line, 64)){
			cli_init(line);
			usart_write("\r\n");
		}
	}
}


