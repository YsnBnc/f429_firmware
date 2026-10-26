#include <stm32f429xx.h>

/*
 * Make sure terminal baud rate is 115200
 */

static void usart_gpio_init(){
	//Enable clock
	RCC->APB2ENR |= RCC_APB2ENR_USART1EN;
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

	//Set TX and RX to alternate function mode and configure
	GPIOA->MODER |= 0x280000; //PA9 PA10
	GPIOA->AFR[1] = (GPIOA->AFR[1] & ~0xFF0) | 0x770; //PA9,10 AF7
	GPIOA->OTYPER &= ~0x600; //PA9,10 push-pull
	GPIOA->OSPEEDR |= (1U << 18) | (1U << 20); //PA9,10 medium speed
	GPIOA->PUPDR |= (1U << 18) | (1U << 20); //PA9,10 pull-up

	/*
	### USARTDIV calculation
	### APB2 = 90MHz BAUD = 115200
	### f_clk / baud = 90,000,000 / 115,200 = 781.25
	*/
	USART1->BRR = 781;

	//Enable transmitter, receiver
	USART1->CR1 |= 0xC;

	//Enable USART1
	USART1->CR1 |= (1U << 13);
}
void usart_put_char(char c){
	while(!(USART1->SR & (1U << 7)));
	USART1->DR = (uint8_t)c;
}

char usart_get_char(void){
	while(!(USART1->SR & (1U << 6))){}
	return (char)USART1->DR;
}

void usart_write(const char* s){
	while(*s){
		usart_put_char(*s++);
	}
	while (!(USART1->SR & (1U << 6)));
}

int usart_readline(char *buf, int max){
	static int id_x = 0;

	if(!(USART1->SR & (1U << 5))) return 0;

	char c = usart_get_char();

	//Check "ENTER" is pressed
	//Enter is \r
	if(c == '\r' || c == '\n'){
		buf[id_x] = '\0';
		id_x = 0;
		usart_write("\r\n"); //Go to next line
		return 1;
	}
	//Backspace
	if(c == '\b'|| c == 0x7F){
		if(id_x > 0){
			id_x--;
			usart_write("\b \b");
		}
		return 0;
	}

	if(id_x < max - 1){
		buf[id_x++] = c;
		usart_put_char(c); //echo
	}
	return 0;

}

void USART_INIT(void){
	usart_gpio_init();
}
