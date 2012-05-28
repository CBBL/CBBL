/**
  ******************************************************************************
  * @file    CBBL_usart/src/hil.c
  * @author  Marco Zavatta, Yin Zhining
  * @version V1.0
  * @date    03/04/2012
  * @brief   Hardware interface layer
  ******************************************************************************
  */

#include "hil.h"

/** @addtogroup CBBL
  * @{
  */

/*
 * @brief  Get LSB of the device's PID
 * @param  void
 * @retval LSB of the device's PID
 */
uint8_t hil_getidbyte2(void) {
	return PIDBYTE2;
}

/*
 * @brief  Checks ROP active or not
 * @param  void
 * @retval 1 if active
 * 		   0 if not active
 */
int32_t hil_ropactive(void) {
	if (FLASH_GetReadOutProtectionStatus()!=0) return 1;
	else return 0;
}

/*
 * @brief  Checks if given address is in range of FLASH or SRAM
 * @param  void
 * @retval 1 if FLASH address
 * 		   0 if RAM address
 * 		  -1 if not valid address
 */
int32_t hil_validateaddr(uint32_t addr) {
	if (addr <= FLASHtop && addr >= FLASHbase) return 1;
	else if (addr <= RAMtop && addr >= RAMbase) return 0;
	else return -1;
}

/*
 * @brief  Reads a word from the FLASH memory
 * @param  void
 * @retval the 32-bit word stored at address,address+1,address+2,address+3
 */
uint32_t hil_readFLASH (uint32_t address) {
	return *(uint32_t*)(address);
}

int32_t hil_writeflash(uint32_t startaddr) {
	//NEED TO MAKE SURE IT'S NOT THE BOOTLOADER'S PAGE ITSELF
return 0;
}

int32_t hil_writeram(uint32_t startaddr) {
return 0;
}

/*
 * @brief  Mass Erase except Page 0 (which contains the bootloader)
 * @param  void
 * @retval
 */
int32_t hil_globalerasememory(void) {
	int32_t pageaddr = FLASHbase;
	while (pageaddr<FLASHtop) {
			 FLASH_ErasePage(pageaddr);
			 pageaddr = pageaddr + FLASHPAGESIZE;
		}
	return 0;
}

int32_t hil_erasecorrespondingpage(int32_t addr) {
	 //NEED TO CHECK THAT IT'S NOT THE BOOTLOADER PAGE!!!
	 FLASH_ErasePage(addr);
	 return 0;
}

int32_t hil_erasebank1(void) {
	hil_globalerasememory();
	return 0;
}

int32_t hil_erasebank2(void) {
	return 0;
}

/*
 * @brief  Generate System reset by SW
 * @param  void
 * @retval none
 */
void hil_reset(void) {

	/* Wait for some time; need of this delay discovered while debugging. */
	delay(0xFFF);

	/* Ensure completion of memory access. */
	__DSB();

	/* Generate reset by setting VECTRESETK and SYSRESETREQ, keeping priority group unchanged.
	 * If only SYSRESETREQ used, no reset is triggered, discovered while debugging.
	 * If only VECTRESETK is used, if you want to read the source of the reset afterwards
	 * from (RCC->CSR & RCC_CSR_SFTRSTF),
	 * it won't be possible to see that it was a software-triggered reset.
	 * */
	SCB->AIRCR  = ((0x5FA << SCB_AIRCR_VECTKEY_Pos)      |
	              (SCB->AIRCR & SCB_AIRCR_PRIGROUP_Msk) |
	              SCB_AIRCR_VECTRESET_Msk |
	              SCB_AIRCR_SYSRESETREQ_Msk);

	/* Ensure completion of memory access. */
	__DSB();

	/* Wait for reset. */
	while(1);
}

int32_t hil_removewriteprotectionflashmem(void) {
	FLASH->CR = FLASH_CR_OPTER;
	return  0;
}

int32_t hil_enablewriteprotectionflashmen(uint32_t sector) {
	FLASH_EnableWriteProtection(sector);
	return 0;
}

int32_t hil_disablerop(void) {
	FLASH_ReadOutProtection(DISABLE);
	return 0;
}

int32_t hil_enablerop(void) {
	FLASH_ReadOutProtection(ENABLE);
	return 0;
}

int32_t hil_clearram(void) {
	return 0;
}

void hil_clock_init(void) {
	/* Activate HSE, HSI. */
	RCC->CR |= RCC_CR_HSEON | RCC_CR_HSION;

	/* Wait until HSE is stable. */
	while (!(RCC->CR & RCC_CR_HSERDY));

	/* Wait until HSI is stable. */
	while (!(RCC->CR & RCC_CR_HSIRDY));

	/* PLL setup. */
	RCC->CFGR |= RCC_CFGR_PLLMULL9 | RCC_CFGR_PLLXTPRE_HSE | RCC_CFGR_PLLSRC_HSE;

	/* Activate PLL. */
	RCC->CR   |= RCC_CR_PLLON;

	/* Wait until PLL is stable. */
	while (!(RCC->CR & RCC_CR_PLLRDY));

	/* AHB, APB1 and APB2 prescaler setup. */
	RCC->CFGR |= RCC_CFGR_HPRE_DIV1 | RCC_CFGR_PPRE1_DIV1 | RCC_CFGR_PPRE2_DIV1;

	/* FLASH wait states setup. */
	FLASH->ACR |= FLASH_ACR_LATENCY_2;

	/* Set PLL as system clock */
	RCC->CFGR |= RCC_CFGR_SW_PLL;

	/* Wait until PLL is the system clock. */
	while (!(RCC->CFGR & RCC_CFGR_SWS_PLL ));
}

/*
 * @brief  Initializes HW platform and its Flash memory controller
 * @param  void
 * @retval void
 */
void hil_init(void) {
	hil_clock_init();
	hil_FPECenable();
	//CLEAR OPTION BYTES
}

/*
 * @brief  Unlocks the FLASH memory Program and Erase Controller
 * @param  void
 * @retval void
 */
void hil_FPECenable(void) {
	FLASH_Unlock();
}

/*
 * @brief  Check whether a sw triggered reset. After read, reset source flags are cleared.
 * @param  void
 * @retval 1 if sw-triggered
 *         0 if not
 */
int8_t hil_ISSWRESET(void) {
	if((RCC->CSR & RCC_CSR_SFTRSTF) == 0x10000000) {
		RCC->CSR |= RCC_CSR_RMVF;
		return 1;
	}
	RCC->CSR |= RCC_CSR_RMVF;
	return 0;
}

/*
 * @brief  Helper function for delay. Loops as many times as the delay parameter
 * @param  uint32_t delay
 * @retval none
 */
void delay(uint32_t delay) {
	uint32_t i = 0;
	while(i<delay) i++;
}
/*FPEC unlock. */
