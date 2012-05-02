/**
  ******************************************************************************
  * @file    CBBL/src/main.c
  * @author  Marco Zavatta, Yin Zhining
  * @version V1.0
  * @date    03/04/2012
  * @brief   Main program body
  ******************************************************************************
  */ 

/** @addtogroup IAP
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "includes.h"
#include "hil.h"
#include "cal.h"
#include "commands.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/

/* Private function prototypes -----------------------------------------------*/
//static void IAP_Init(void);
//static void IO_Init(void);

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program.
  * @param  None
  * @retval None
  */
int main(void)
{
  /* Initialize. */
  hil_init();
  cal_init();

  /* Which kind of reset was it? */
  int8_t resettype;
  resettype = hil_ISSWRESET();

  cal_SENDLOG("\r\n");
  cal_SENDLOG("=========CBBL Log=========\r\n");
  cal_SENDLOG("Marco Zavatta, Yin Zhining\r\n");
  cal_SENDLOG("POLIMI, 2011/2012\r\n");
  cal_SENDLOG("==========================\r\n");
  cal_SENDLOG("\r\n");

  /* Test if button on the board is pressed during reset or if it was a sw-triggered reset. */
  if (((GPIOB->IDR & GPIO_IDR_IDR1) == 0x00 && resettype == 0) || resettype == 1)
  { 
	if (resettype==1) {
		GPIOA->BSRR |= GPIO_BSRR_BS0 | GPIO_BSRR_BS1 | GPIO_BSRR_BS2 | GPIO_BSRR_BR3;
		cal_SENDLOG("-> software reset occured \r\n");
	}
	else	{
		GPIOA->BSRR |= GPIO_BSRR_BS0 | GPIO_BSRR_BS1 | GPIO_BSRR_BR2 | GPIO_BSRR_BR3;
		cal_SENDLOG("-> button pressed \r\n");
	}

	command_receiveinit();
  }
  /* Keep the user application running */
  else
  {
	cal_SENDLOG("-> button not pressed, jumping to app\r\n");
	/* Jump to pre-loaded application. */
	jumptoapp(FLASHbase);
  }
  while (1)
  {
	cal_SENDLOG("-> !!! main function fail !!!\r\n");
	uint32_t i;
	while (1)
	{
		i=0;
		while (i<0xFFFFF) i++;
		GPIOA->BSRR |= GPIO_BSRR_BS0 | GPIO_BSRR_BR1 | GPIO_BSRR_BR2 | GPIO_BSRR_BR3;
		i=0;
		while (i<0xFFFFF) i++;
		GPIOA->BSRR |= GPIO_BSRR_BR0 | GPIO_BSRR_BR1 | GPIO_BSRR_BR2 | GPIO_BSRR_BS3;
	}
  }
}


/**************************** Politecnico di Milano ************END OF FILE****/


#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/******************* (C) COPYRIGHT 2010 STMicroelectronics *****END OF FILE****/
