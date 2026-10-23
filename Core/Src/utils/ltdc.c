
#include <stm32f429xx.h>
#include <ili9341.h>

#define LCD_H 240
#define LCD_V 320
#define HBP 29
#define HWS 9
#define HFP 2
#define VSH 1
#define VBP 3
#define VFP 2

#define LCD_CS_PORT  GPIOC
#define LCD_CS_PIN   2
#define LCD_DC_PORT  GPIOD
#define LCD_DC_PIN   13

static void delay_ms(uint32_t ms)
{
    for (volatile uint32_t i = 0; i < ms * 2000U; i++) { }
}


static void clocks_init(void){
	//Enable GPIO clocks
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN
				  | RCC_AHB1ENR_GPIOBEN
				  | RCC_AHB1ENR_GPIOCEN
				  | RCC_AHB1ENR_GPIODEN
				  | RCC_AHB1ENR_GPIOFEN
				  | RCC_AHB1ENR_GPIOGEN;
	RCC->APB2ENR |= RCC_APB2ENR_LTDCEN //Enable LTDC
				  | RCC_APB2ENR_SPI5EN; //ILI9341 interface connected on SPI5
	(void)RCC->AHB1ENR;
	(void)RCC->APB2ENR;
}

static void pllsai_init(void){
	 RCC->CR &= ~RCC_CR_PLLSAION; //Disable PLLSAI

	 while(RCC->CR & RCC_CR_PLLSAIRDY){} //Wait for shutdown

	 RCC->PLLSAICFGR = (192U << RCC_PLLSAICFGR_PLLSAIN_Pos) //PLLSAIN 192MHz
					 | (4U << RCC_PLLSAICFGR_PLLSAIQ_Pos)
					 | (4U << RCC_PLLSAICFGR_PLLSAIR_Pos); //PLLSAIR 192/4=48MHz

	 RCC->DCKCFGR &= ~(3U << RCC_DCKCFGR_PLLSAIDIVR_Pos); //Reset PLLSAI divider
	 RCC->DCKCFGR |= (2U << RCC_DCKCFGR_PLLSAIDIVR_Pos); //Set PLLSAI divider to 48/8=6MHz

	 RCC->CR |= RCC_CR_PLLSAION; //Enable PLLSAI
	 while(!(RCC->CR & RCC_CR_PLLSAIRDY)){} //Wait for wake up
}

static void pins_init(void){
	//Set pins alternate function
	GPIOA->MODER = (GPIOA->MODER & ~0x03C033C0) | 0x02802280;  // PA3,4,6,11,12
	GPIOB->MODER = (GPIOB->MODER & ~0x00FF000F) | 0x00AA000A;  // PB0,1,8,9,10,11
	GPIOC->MODER = (GPIOC->MODER & ~0x0030F000) | 0x0020A000;  // PC6,7,10
	GPIOD->MODER = (GPIOD->MODER & ~0x000030C0) | 0x00002080;  // PD3,6
	GPIOF->MODER = (GPIOF->MODER & ~0x003FC000) | 0x002A8000;  // PF7,8,9,10
	GPIOG->MODER = (GPIOG->MODER & ~0x03F0F000) | 0x02A0A000;  // PG6,7,10,11,12

	//Set pins speed to very high
	GPIOA->OSPEEDR |= 0x03C033C0;
	GPIOB->OSPEEDR |= 0x00FF000F;
	GPIOC->OSPEEDR |= 0x0030F000;
	GPIOD->OSPEEDR |= 0x000030C0;
	GPIOF->OSPEEDR |= 0x003FC000;
	GPIOG->OSPEEDR |= 0x03F0F000;

	//Set pins to no-pull
	GPIOA->PUPDR &= ~0x03C033C0;
	GPIOB->PUPDR &= ~0x00FF000F;
	GPIOC->PUPDR &= ~0x0030F000;
	GPIOD->PUPDR &= ~0x000030C0;
	GPIOF->PUPDR &= ~0x003FC000;
	GPIOG->PUPDR &= ~0x03F0F000;

	//Configure alternate functions
	GPIOA->AFR[0] = (GPIOA->AFR[0] & ~0x0F0FF000) | 0x0E0EE000; // PA3,4,6   -> AF14
	GPIOA->AFR[1] = (GPIOA->AFR[1] & ~0x000FF000) | 0x000EE000; // PA11,12   -> AF14
	GPIOB->AFR[0] = (GPIOB->AFR[0] & ~0x000000FF) | 0x00000099; // PB0,1     -> AF9
	GPIOB->AFR[1] = (GPIOB->AFR[1] & ~0x0000FFFF) | 0x0000EEEE; // PB8-11    -> AF14
	GPIOC->AFR[0] = (GPIOC->AFR[0] & ~0xFF000000) | 0xEE000000; // PC6,7     -> AF14
	GPIOC->AFR[1] = (GPIOC->AFR[1] & ~0x00000F00) | 0x00000E00; // PC10      -> AF14
	GPIOD->AFR[0] = (GPIOD->AFR[0] & ~0x0F00F000) | 0x0E00E000; // PD3,6     -> AF14
	GPIOF->AFR[0] = (GPIOF->AFR[0] & ~0xF0000000) | 0x50000000; // PF7       -> AF5
	GPIOF->AFR[1] = (GPIOF->AFR[1] & ~0x00000FFF) | 0x00000E55; // PF8,9 AF5, PF10 AF14
	GPIOG->AFR[0] = (GPIOG->AFR[0] & ~0xFF000000) | 0xEE000000; // PG6,7     -> AF14
	GPIOG->AFR[1] = (GPIOG->AFR[1] & ~0x000FFF00) | 0x0009E900; // PG10 AF9, PG11 AF14, PG12 AF9

	//LCD CS and CD ports
	GPIOD->MODER &= ~(3U << 26);
	GPIOD->MODER |= (1U << 26);
	GPIOC->MODER &= ~(3U << 4);
	GPIOC->MODER |= (1U << 4);

	GPIOD->OTYPER &= ~(1U << 13);
	GPIOC->OTYPER &= ~(1U << 2);

	GPIOD->OSPEEDR |= (3U << 26);
	GPIOC->OSPEEDR |= (3U << 4);

	GPIOD->PUPDR &= ~(3U << 26);
	GPIOC->PUPDR &= ~(3U << 4);

	GPIOC->BSRR = (1U << 2);
}

static void ltdc_setup(void){

	//Set timing registers for vertical and horizontal
	LTDC->SSCR = ((HWS - 1) << 16) | (VSH - 1);
	LTDC->BPCR = ((HWS + HBP - 1) << 16) | (VSH + VBP - 1);
	LTDC->AWCR = ((HWS + HBP + LCD_H - 1) << 16) | (VSH + VBP + LCD_V - 1);
	LTDC->TWCR = ((HWS + HBP + LCD_H + HFP - 1) << 16) | (VSH + VBP + LCD_V + VFP -1);

	//Background color
	LTDC->BCCR = 0x000000;

	//Load shadow registers
	LTDC->SRCR = LTDC_SRCR_IMR;
	while(LTDC->SRCR & LTDC_SRCR_IMR){};

	//LTDC sync + clock start
	LTDC->GCR |= LTDC_GCR_LTDCEN;
}

/* CLAUDE CODE AREA START*/

static void spi5_init(void)
{
    /* APB2 = 16 MHz, /4 = 4 MHz (safe for ILI9341 writes and reads),
       mode 0, 8-bit, MSB first, software NSS */
    SPI5->CR1 = SPI_CR1_BR_0 | SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI;
    SPI5->CR1 |= SPI_CR1_SPE;
}

static uint8_t spi5_xfer(uint8_t b)
{
    while (!(SPI5->SR & SPI_SR_TXE)) { }
    *(__IO uint8_t *)&SPI5->DR = b;
    while (!(SPI5->SR & SPI_SR_RXNE)) { }
    uint8_t r = *(__IO uint8_t *)&SPI5->DR;
    while (SPI5->SR & SPI_SR_BSY) { }
    return r;
}

#define CS_LOW()   (LCD_CS_PORT->BSRR = (1U << (LCD_CS_PIN + 16)))
#define CS_HIGH()  (LCD_CS_PORT->BSRR = (1U << LCD_CS_PIN))
#define DC_LOW()   (LCD_DC_PORT->BSRR = (1U << (LCD_DC_PIN + 16)))   // command
#define DC_HIGH()  (LCD_DC_PORT->BSRR = (1U << LCD_DC_PIN))          // data

void LCD_IO_Init(void)
{
    spi5_init();
}

void LCD_IO_WriteReg(uint8_t Reg)
{
    DC_LOW();
    CS_LOW();
    spi5_xfer(Reg);
    CS_HIGH();
}

void LCD_IO_WriteData(uint16_t RegValue)
{
    DC_HIGH();
    CS_LOW();
    spi5_xfer((uint8_t)RegValue);
    CS_HIGH();
}

uint32_t LCD_IO_ReadData(uint16_t RegValue, uint8_t ReadSize)
{
    uint32_t value = 0;
    DC_LOW();
    CS_LOW();
    spi5_xfer((uint8_t)RegValue);
    DC_HIGH();
    for (uint8_t i = 0; i < ReadSize; i++)
        value = (value << 8) | spi5_xfer(0xFF);
    CS_HIGH();
    return value;
}

void LCD_Delay(uint32_t delay)
{
    delay_ms(delay);
}

static uint16_t framebuffer[LCD_H * LCD_V] __attribute__((aligned(4)));

static void fill(uint16_t color)
{
    for (uint32_t i = 0; i < LCD_H * LCD_V; i++) framebuffer[i] = color;
}

static void lcd_cmd(uint8_t cmd, const uint8_t *params, uint8_t n)
{
    LCD_IO_WriteReg(cmd);
    for (uint8_t i = 0; i < n; i++) LCD_IO_WriteData(params[i]);
}

/* CLAUDE CODE AREA END*/

void LTDC_INIT(void){
    clocks_init();
    pllsai_init();
    pins_init();
    ltdc_setup();
    ili9341_Init();
}
