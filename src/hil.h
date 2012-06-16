/**
  ******************************************************************************
  * @file    CBBL_usart/src/hil.h
  * @author  Marco Zavatta, Yin Zhining
  * @version V1.0
  * @date    03/04/2012
  * @brief   Hardware interface layer
  ******************************************************************************
  */
#include "includes.h"

#define STM32F10X_MD
#define BOARD 07301A-15
#define PCLK1 (0x112A880) //value given in Hz: 18MHz

#define PIDBYTE2				(0x10)
#define FLASHbase				(0x08006000)
#define FLASHtop				(0x0801FFFF)
#define FLASHPAGESIZE   		(0x400)
#define SECTORSIZE      		(0x1000)
#define RAMbase         		(0x20000200)
#define RAMtop          		(0x20005000)
#define SCBAIRCR_SYSRESETVALUE  (0xF5FA0004)

/* Exported functions ------------------------------------------------------- */
void hil_init(void);

uint32_t hil_readFLASH (uint32_t address);
uint8_t hil_getidbyte2(void);
int32_t hil_ropactive(void);
int32_t hil_validateaddr(uint32_t addr);
int32_t hil_writeflash(uint32_t startaddr);
int32_t hil_writeram(uint32_t startaddr);
int32_t hil_globalerasememory(void);
int32_t hil_erasecorrespondingpage(int32_t addr);
int32_t hil_erasebank1(void);
int32_t hil_erasebank2(void);
void hil_reset(void);
int32_t hil_removewriteprotectionflashmem(void);
int32_t hil_enablewriteprotectionflashmen(uint32_t sector);
int32_t hil_disablerop(void);
int32_t hil_enablerop(void);
int32_t hil_clearram(void);//clear whole ram , should be different from globalerasememory()


void hil_clock_init(void);
void hil_FPECenable(void);
int8_t hil_isSWreset();
void delay(uint32_t delay);
