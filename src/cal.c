/**
  ******************************************************************************
  * @file    CBBL_usart/src/cal.c
  * @author  Marco Zavatta, Yin Zhining
  * @version V1.0
  * @date    03/04/2012
  * @brief   Communication abstraction layer
  ******************************************************************************
  */

#include "cal.h"

/*
 * @brief  Send byte through selected communication mode
 * @param  Byte to be sent
 * @retval 0 if successful, -1 if not successful
 */
int32_t cal_sendbyte(uint8_t b) {
	#ifdef USART
	USART_SendData(USART1, (uint16_t)b);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) {
	}
	return 0;
	#elif defined CAN
	#endif
	return -1;
}


/*
 * @brief  Receive byte through selected communication mode
 * @param  Pointer to received byte container
 * @retval 0 if successful, -1 if not successful/timeout expired
 */
int32_t cal_receivebyte(uint8_t *c, uint32_t timeout) {

	#ifdef USART
	while (timeout-- > 0)	{
		if ( USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)	{
			*c = USART1->DR;
			return 0;
		}
	}
	return -1;

	#elif defined CAN

	#endif
	return -1;
}

/*
 * @brief  Receive word(32bit) through selected communication mode
 * @param  Pointer to received word container
 * @retval 0 if successful, -1 if not successful/timeout expired
 */
/*
int32_t cal_receiveword(uint32_t *c, uint32_t timeout) {
	uint8_t bytes[4], i;
	for (i=0; i<4; i++) if (cal_receivebyte(bytes+i, timeout)==-1) return -1;
	c = (uint32_t*)bytes;
	return 0;
}
*/

int32_t cal_receiveword(uint32_t *c, uint32_t timeout) {
	uint32_t bytes = 0;
    uint8_t a1,a2,a3,a4;
    cal_READBYTE(a1,TIMEOUT_NACK);
    cal_READBYTE(a2,TIMEOUT_NACK);
    cal_READBYTE(a3,TIMEOUT_NACK);
    cal_READBYTE(a4,TIMEOUT_NACK);
    bytes |= a1<<24;
    bytes |= a2<<16;
    bytes |= a3<<8;
    bytes |= a4;
    /*
    addr_buf[ 0 ] = address >> 24;
      addr_buf[ 1 ] = ( address >> 16 ) & 0xFF;
      addr_buf[ 2 ] = ( address >> 8 ) & 0xFF;
      addr_buf[ 3 ] = address & 0xFF;
      */
    *c = bytes;
    return 0;
}

/*
 * @brief  Send word through selected communication mode
 * @param  Word to be sent
 * @retval 0 if successful, -1 if not successful
 */
int32_t cal_sendword(uint32_t b) {
	uint8_t *p, i;
	p = (uint8_t*)&b;
	for (i=0; i<4; i++) cal_sendbyte(*(p+i));
	return 0;
}

/**
  * @brief  Send a string string through selected communication mode
  * @param  s: The string to be sent
  * @retval 0 if successful
  * 	   -1 if unsuccessful
  */
int32_t cal_sendstring(uint8_t *s)	{
	while (*s != '\0')  {
		if(cal_sendbyte(*s)==-1) return -1;
		s++;
	}
	return 0;
}


/*
 * @brief  Initialize communication layer for the desired mode
 * @param  void
 * @retval 0 if successful, -1 if not successful
 */
int32_t cal_init(void) {

	GPIOinit();

	#ifdef USART
	USARTinit();
	//USARTinitALT();
	#elif defined CAN
	CANinit();
	#endif

	return 0;
}

/*
 * @brief  Initialize USART1 peripheral
 * @param  void
 * @retval void
 */
//remember that to be STM32 embedded bl compliant even parity bits must be active!
void USARTinit(void) {

		  /*Baud Rate at 115200. */
		  uint32_t BRR=0x00000271;

		  /*GPIOA, GPIOB on APB2 bus clock enable. */
		  /* DONE IN GPIOinit. */
		  //RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;

		  /*USART1 on APB2 bus clock enable. */
		  RCC->APB2ENR |= RCC_APB2ENR_USART1EN;

		  /*Configure GPIOA output and input mode for UART. */
		  /*USART1 TX as alternate function push-pull: bits 4-7 to 1011=B. */
		  /*USART1 RX as input floating: bits 8-11 to 0100=4. */
		  GPIOA->CRH = 0x444444B4;

		  /*Ensure no remap, keep PA9,PA10. */
		  AFIO->MAPR &= ~AFIO_MAPR_USART1_REMAP;

		  /*USART Enable. */
		  USART1->CR1 |= USART_CR1_UE;

		  /*Use 1 stop bit. */
		  USART1->CR2 &= ~0x00003000;

		  /*Set BaudRate=115.2Ksymb/sec @72MHz CLK, 1 Symb=1 Bit. */
		  USART1->BRR = BRR;

		  /*Set word length to 8 bits. */
		  USART1->CR1 &= ~USART_CR1_M;

		  /*Ensure Hardware flow control disabled. */
		  USART1->CR3 &= 0x000009FF;

		  //!!!!!!!!!!!!!! /*Ensure no parity bit. */
		  USART1->CR1 &= 0xFFFFFBFF;

		  /*Set TE RE (transmission and receive enable) bits. */
		  USART1->CR1 |= USART_CR1_TE | USART_CR1_RE;
}

/*
 * @brief  Initialize GPIO peripheral
 * @param  void
 * @retval void
 */
void GPIOinit(void) {
	  /*GPIO Initialization. */

	  /*GPIOA, GPIOB on APB2 bus clock enable. */
	  RCC->APB2ENR |= RCC_APB2ENR_IOPAEN | RCC_APB2ENR_IOPBEN | RCC_APB2ENR_AFIOEN;

	  /*PA0, PA1, PA2, PA3 in output mode general purpose push-pull. */
	  GPIOA->CRL =(0x03 << (3 * 4)) | (0x03 << (2 * 4)) | (0x03 << (1 * 4)) | (0x03 << (0 * 4));

	  /*PB0, PB1 in floating input mode. */
	  GPIOB->CRL = (0x08 << (1 * 4)) | (0x08 << (0 * 4));

	  /*why do we do this?. */
	  GPIOB->ODR |= GPIO_ODR_ODR0 | GPIO_ODR_ODR1;

	  /*Light up selected leds by resetting the output bits. */
	  GPIOA->BSRR |= GPIO_BSRR_BR0 | GPIO_BSRR_BR1 | GPIO_BSRR_BS2 | GPIO_BSRR_BS3;
}

void cal_baudrate(void) {
	return;
}

void USARTinitALT() {

	  USART_InitTypeDef USART_InitStructure;

	  /* USART resources configuration (Clock, GPIO pins and USART registers) ----*/
	  /* USART configured as follow:
	        - BaudRate = 115200 baud
	        - Word Length = 8 Bits
	        - One Stop Bit
	        - No parity
	        - Hardware flow control disabled (RTS and CTS signals)
	        - Receive and transmit enabled
	  */

	  USART_InitStructure.USART_BaudRate = 115200;
	  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	  USART_InitStructure.USART_StopBits = USART_StopBits_1;
	  USART_InitStructure.USART_Parity = USART_Parity_No;
	  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	  USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;

	  USART_Init(USART1, &USART_InitStructure);
}

/**************************** Politecnico di Milano ************END OF FILE****/
