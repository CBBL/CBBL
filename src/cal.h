/**
  ******************************************************************************
  * @file    CBBL_usart/src/cal.h
  * @author  Marco Zavatta, Yin Zhining
  * @version V1.0
  * @date    03/04/2012
  * @brief   Communication abstraction layer
  ******************************************************************************
  */

#include "includes.h"
#include "hil.h"

/* Communication peripheral selection --------------------------------------- */
/* Uncomment the intended device -------------------------------------------- */
#define USART 1
#define CAN 2

/* Global variables --------------------------------------------------- */
uint8_t comm_peripheral;

//#ifdef USART
#include "stm32f10x_usart.h"
//#elif defined CAN
#include "stm32f10x_can.h"
//#endif

/* ------------------------------------------------------------------------- */

/* ------------------------------------------------------------------------- */
/* Defines ----------------------------------------------------------------- */
#define USART_BAUD 		(115200)
#define CAN_BAUD		(115200)
#define CAN_TS1			(0x4)
#define CAN_TS2			(0x2)
#define CAN_BRP			(0x3)
#define CAN_SJW			(0x1)
#define TIMEOUT_NACK 	(0xFFFFFF)
#define TIMEOUT_INIT 	(0xFFFFFFFF)
#define MSGID			(0x00);


/* Exported functions ------------------------------------------------------- */
int32_t cal_init(void);
void cal_baudrate(void); //Don't need any parameters, default value is 115200
int32_t cal_sendbyte(uint8_t b);    //want to return value to say whether sending succeed or not, within sendbyte, there is a mechanism that will do checksum
int32_t cal_receivebyte(uint8_t *c, uint32_t timeout);  // if it receives sth,return exact byte, otherwise return -1;remember to cast from 1 byte to 4 bytes
int32_t cal_receiveword(uint32_t *c, uint32_t timeout);
int32_t cal_sendword(uint32_t b);
int32_t cal_sendstring(uint8_t *s);

/* Private function prototypes --------------------------------------------- */
void GPIOinit(void);
void USARTinit(void);
void CANinit(void);

/* Useful macros ----------------------------------------------------------- */
#define cal_READBYTE(x, timeout)\
  if(cal_receivebyte((uint8_t *)&x, timeout) == -1 )\
    return -1

#define cal_READWORD(x, timeout)\
  if(cal_receiveword((uint32_t *)&x, timeout) == -1 )\
    return -1

#define cal_SENDBYTE(x)\
  if(cal_sendbyte(x)==-1)\
	return -1

#define cal_SENDNACK()\
  cal_sendbyte(STM32_COMM_NACK);\
    return -1

#define cal_SENDACK()\
  if(cal_sendbyte(STM32_COMM_ACK)==-1)\
    return -1

/* LOG macros ------------------------------------------------------------------ */

/* Swap desired commented define statement in order to enable-disable logging info. */
/* Trick is to replace the sending of a string with empty code at compile time. */
#define cal_SENDLOG(x)
/*
#define cal_SENDLOG(x)\
  if(cal_sendstring((uint8_t*)x))\
  	return -1
*/




/**************************** Politecnico di Milano ************END OF FILE****/
