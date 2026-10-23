#include "od.h"
#include "ltdc.h"
#include <stm32f429xx.h>


int main(void){
	od_clock_init();
	LTDC_INIT();
}


