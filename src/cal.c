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

/** @addtogroup CBBL
  * @{
  */

/*
 * @brief  Send byte through selected communication mode
 * @param  Byte to be sent
 * @retval 0 if successful, -1 if not successful
 */
int32_t cal_sendbyte(uint8_t b) {

	if (comm_peripheral == USART)
	{
		USART_SendData(USART1, (uint16_t)b);
		while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET) {
		}
		return 0;
	}

	else
	{

	/* Refer to CANinit() for CAN configuration details. */

	uint8_t mailbox;	//mailbox that will transmit the message
	//uint32_t tries = 0, maxTries = 99;

	/* Visual signal if any error has occurred. */
	if (CAN_GetReceiveErrorCounter(CAN1) != 0)
			GPIOA->BSRR |= GPIO_BSRR_BR0 | GPIO_BSRR_BR1 | GPIO_BSRR_BR2 | GPIO_BSRR_BR3;

	/* Set up the packet info. */
	CanTxMsg msg;
	msg.DLC = 1;		//frame length
	msg.RTR = 0;		//data frame (not a remote frame)
	msg.IDE = 0;		//standard identifier (not an extended identifier)
	msg.StdId = 0;		//identifier value 0; must have been allowed for entrance by a filter bank
	msg.ExtId = 0;		//identifier value 0; must have been allowed for entrance by a filter bank
	msg.Data[0] = b;	//data

	/* Fire, blocking until the message has been sent. This will result in the usage of only one mailbox. */
	mailbox = CAN_Transmit(CAN1, &msg);
	while (CAN_TransmitStatus(CAN1, mailbox)!=CAN_TxStatus_Ok) {
	}

	/* If sent byte is ACK, then visual signal, otherwise */
	if (b == 0x79) GPIOA->BSRR |= GPIO_BSRR_BR0 | GPIO_BSRR_BS1 | GPIO_BSRR_BR2 | GPIO_BSRR_BS3;
	else GPIOA->BSRR |= GPIO_BSRR_BS0 | GPIO_BSRR_BR1 | GPIO_BSRR_BR2 | GPIO_BSRR_BR3;

	/* Fire, blocking until a mailbox is found free or too many attempts are done. */
	/*
	do {
		mailbox = CAN_Transmit(CAN1, &msg);
		tries++;
	}
	while (mailbox==CAN_TxStatus_NoMailBox && tries<maxTries);
	*/

	/* Visual signal if any error has occurred. */
	if (CAN_GetReceiveErrorCounter(CAN1) != 0)
			GPIOA->BSRR |= GPIO_BSRR_BR0 | GPIO_BSRR_BR1 | GPIO_BSRR_BR2 | GPIO_BSRR_BR3;

	/* Hard Fault if no mailbox is found empty. */
	if (mailbox==CAN_TxStatus_NoMailBox) HardFault_Handler();

	return 0;
	}

	return -1;
}


/*
 * @brief  Receive byte through selected communication mode.
 * @param  Pointer to received byte container
 * @retval 0 if successful, -1 if not successful/timeout expired
 */
int32_t cal_receivebyte(uint8_t *c, uint32_t timeout) {

	if (comm_peripheral == USART)
	{
		while (timeout-- > 0)	{
			if ( USART_GetFlagStatus(USART1, USART_FLAG_RXNE) != RESET)	{
				*c = USART1->DR;
				return 0;
			}
		}
		return -1;
	}

	else
	{

	/* Refer to CANinit() for CAN configuration details. */

	CanRxMsg msg0;

	/* Visual signal if any error has occurred. */
	if (CAN_GetReceiveErrorCounter(CAN1) != 0)
		GPIOA->BSRR |= GPIO_BSRR_BR0 | GPIO_BSRR_BR1 | GPIO_BSRR_BR2 | GPIO_BSRR_BR3;

	/* Fetch status of receive FIFOs. */
	//s0 = CAN1->RF0R;
	//s1 = CAN1->RF1R;

	/* Block until a message has arrived. */
	while (!CAN_MessagePending(CAN1, CAN_FIFO0));

	/* Receive the message from FIFO0.
	 * FIFO0 is the only FIFO that will get incoming messages
	 * as a pass-all filter is assigned to FIFO0 (CANinit())*/
	CAN_Receive(CAN1, CAN_FIFO0, &msg0);

	/* Visual signal if any error has occurred. */
	if (CAN_GetReceiveErrorCounter(CAN1) != 0)
			GPIOA->BSRR |= GPIO_BSRR_BR0 | GPIO_BSRR_BR1 | GPIO_BSRR_BR2 | GPIO_BSRR_BR3;

	/* Receive the message from FIFO1. */
	//CAN_Receive(CAN1, CAN_FIFO1, &msg1);

	//f0 = msg0.Data[0];
	//f1 = msg1.Data[0];

	/* Extract the data. */
	*c = msg0.Data[0];

	return 0;
	}

	return -1;
}


/*
int32_t cal_receiveword(uint32_t *c, uint32_t timeout) {
	uint8_t bytes[4], i;
	for (i=0; i<4; i++) if (cal_receivebyte(bytes+i, timeout)==-1) return -1;
	c = (uint32_t*)bytes;
	return 0;
}
*/

/*
 * @brief  Receive word(32bit) through selected communication mode
 * @param  Pointer to received word container
 * @retval 0 if successful, -1 if not successful/timeout expired
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

	//#ifdef USART
	USARTinit();
	//#elif defined CAN
	CANinit();
	//#endif

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
 * @brief  Initializes CAN peripheral
 * @param  void
 * @retval void
 */
void CANinit() {

	//uint32_t stdmsgid = 0;

	/* This id must be between 0-13 as there are 14 filter banks available in MD devices. */
	uint32_t filterbankid = 0;

	/* bxCAN on APB1 bus clock enable. */
	RCC->APB1ENR |= RCC_APB1ENR_CAN1EN;

	/* Enable clock on GPIOB. */
	RCC->APB2ENR |= RCC_APB2ENR_IOPBEN;

	/* Remap CAN pinouts to GPIOB's PB8, PB9: bits 13=0 14=1. */
	RCC->APB2ENR |= RCC_APB2ENR_AFIOEN;
	AFIO->MAPR |= AFIO_MAPR_CAN_REMAP_REMAP2;
	//AFIO->MAPR &= ~0x00002000;

	/*Configure GPIOB (PB9, PB10) output and input mode for CAN. */
	/*CAN TX as alternate function push-pull: bits 4-7 to 1011=B. */
	/*CAN RX as input floating: bits 0-3 to 0100=4. */
	GPIOB->CRH &= 0xFFFFFF00;
	GPIOB->CRH |= 0x000000B4;

	/* Enter CAN initialization mode. */
	CAN1->MCR |= CAN_MCR_INRQ;

	/* Wait until init mode entered. */
	while (((CAN1->MSR & CAN_MSR_INAK) != CAN_MSR_INAK));

	/* CAN module still working during debug. */
	//CAN1->MCR &= ~0x00010000;

	/* Use automatic wakeup mode. */
	//CAN1->MCR |= CAN_MCR_AWUM;

	/* Use automatic retransmission mode. */
	CAN1->MCR &= ~(uint32_t) CAN_MCR_NART;
	//  |=

	/* Receive FIFO locked against overrun; incoming messages when FIFO full will be discarded. */
	//CAN1->MCR |= CAN_MCR_RFLM;

	/* When many transmit Mailboxes are ready, transmit in request chronological order. */
	CAN1->MCR |= CAN_MCR_TXFP;

	uint32_t btr = 0;

	/* Use hot self-test mode (silent+loop back). */
	//btr |= (CAN_BTR_SILM | CAN_BTR_LBKM);

	/* Set CAN baud rate prescaler. */
	btr |= CAN_BRP;

	/* Set TS1, TS2 to achieve 0.7*BitTime=SampleTime @36MHz. */
	btr |= (CAN_SJW << 24) | (CAN_TS2 << 20) | (CAN_TS1 << 16);

	CAN1->BTR = btr;

	//uint32_t btr = CAN1->BTR;
	//btr = 0;

	/* ID of the messages that are allowed to enter receive FIFOs
	 * adapted for standard id (not extended) format. */
	//stdmsgid  |= (0) | CAN_ID_STD;

	/* Enter initialization mode for filter banks and in particular for the specified bank. */
	CAN1->FMR |= CAN_FMR_FINIT;
	CAN1->FA1R &= ~(uint32_t)(1 << filterbankid);

	/* Set 32-bit scale filtering. */
	CAN1->FS1R |= (uint32_t)(1 << filterbankid);

	/* Set mask mode. */
	CAN1->FM1R &= ~(uint32_t)(1 << filterbankid);

	/* Assign allowed message ids to the selected filter bank. */
	CAN1->sFilterRegister[filterbankid].FR1 = (uint32_t)MSGID;
	CAN1->sFilterRegister[filterbankid].FR2 = (uint32_t)MSGID;

	/* Messages passing the selected bank will flow into FIFO0. */
	CAN1->FFA1R &= ~(uint32_t)(1 << filterbankid);

	/* Activate the selected bank. */
	CAN1->FA1R |= (uint32_t)(1 << filterbankid);

	/* Leave initialization mode for filter banks. */
	CAN1->FMR &= ~CAN_FMR_FINIT;

	/*Enter CAN normal mode. */
	CAN1->MCR &= ~CAN_MCR_INRQ;

	/* Wait until normal mode entered. */
	while (((CAN1->MSR & CAN_MSR_INAK) == CAN_MSR_INAK));

	/* Exit sleep mode (need discovered while debugging). */
	CAN1->MCR &= ~CAN_MCR_SLEEP;

	/* Wait transmit mailbox empty. */
	//while ((CAN->TSR & CAN_TSR_TME0) == 0);
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

/**************************** Politecnico di Milano ************END OF FILE****/
