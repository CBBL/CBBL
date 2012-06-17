/**
  ******************************************************************************
  * @file    CBBL_usart/src/commands.h
  * @author  Marco Zavatta, Yin Zhining
  * @version V1.0
  * @date    03/04/2012
  * @brief   STM32 embedded bootloader's commands implementation
  ******************************************************************************
  */

#include "includes.h"
#include "cal.h"
#include "hil.h"

typedef  void (*pFunction)(void);

#define BLVERSION (0x10)

/* Exported functions ------------------------------------------------------- */
int32_t receivecommand(void);
int32_t command_receiveinit(void);

/* Function prototypes ------------------------------------------------------ */
uint8_t calculatechecksum(uint8_t *data, uint32_t length);
int32_t checkchecksumword(uint32_t data, uint8_t length, uint8_t checksum);
int32_t checkchecksumbytes(uint8_t *data, uint32_t length, uint8_t checksum);
int32_t jumptoapp(uint32_t addr);

/* Protocol commands handlers ----------------------------------------------- */
int32_t command_get_command();
int32_t command_get_version();
int32_t command_get_id();
int32_t command_read_memory();
int32_t command_write_memory(); //*
int32_t command_go(); //*
int32_t command_erase(); //*
int32_t command_extended_erase(); //not implemented
int32_t command_write_protect();
int32_t command_write_unprotect(); //*
int32_t command_readout_protect(); //not implemented
int32_t command_readout_unprotect(); //not implemented

/* Command header identifier bytes. */
#define STM32_CMD_INIT 						(0x7F)
#define STM32_CMD_GET_COMMAND  				(0x00)
#define STM32_CMD_GETVERSION_READPROTECTION (0x01)
#define STM32_CMD_GET_ID 					(0x02)
#define STM32_CMD_READ_MEMORY 				(0x11)
#define STM32_CMD_WRITE_MEMORY 				(0x31)
#define STM32_CMD_GO 						(0x21)
#define STM32_CMD_ERASE  					(0x43)
#define STM32_CMD_EXTENDED_ERASE 			(0x44)
#define STM32_CMD_WRITE_PROTECT 			(0x63)
#define STM32_CMD_WRITE_UNPROTECT 			(0x73)
#define STM32_CMD_READOUT_PROTECT			(0x82)
#define STM32_CMD_READOUT_UNPROTECT 		(0x92)

/* Communication data. */
#define STM32_COMM_ACK      0x79
#define STM32_COMM_NACK     0x1F
#define STM32_COMM_TIMEOUT  2000000
#define STM32_WRITE_BUFSIZE 256
