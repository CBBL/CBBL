/**
  ******************************************************************************
  * @file    CBBL_usart/src/commands.c
  * @author  Marco Zavatta, Yin Zhining
  * @version V1.0
  * @date    03/04/2012
  * @brief   STM32 embedded bootloader's commands implementation. Refer to ST's
  * 		 AN3155 for the protocol specification.
  ******************************************************************************
  */

#include "commands.h"

/** @addtogroup CBBL
  * @{
  */

/*
 * @brief  Receives the command code from the host side and accordingly runs the command
 * @param  void
 * @retval 0: successful
 * 		  -1: command code not found or timeout expired or incorrect command code complement
 */
int32_t receivecommand(void) {
	uint8_t p;
	uint8_t q; //q must equal !p
	cal_READBYTE(p, TIMEOUT_NACK);
	switch(p) {
		case STM32_CMD_GET_COMMAND :
			cal_SENDLOG("-> first byte: get command \r\n");
			cal_READBYTE(q, TIMEOUT_NACK);
			uint8_t t = (~(uint8_t)(STM32_CMD_GET_COMMAND));
			if (q==t) {
				return command_get_command();
			}
			else cal_SENDNACK();
		case STM32_CMD_ERASE :
			cal_READBYTE(q, TIMEOUT_NACK);
			if (q== (uint8_t)~STM32_CMD_ERASE) {
				return command_erase();
			}
			else cal_SENDNACK();
		case STM32_CMD_GET_ID :
			cal_READBYTE(q, TIMEOUT_NACK);
			if (q== (uint8_t)~STM32_CMD_GET_ID) {
				return command_get_id();
			}
			else cal_SENDNACK();
		case STM32_CMD_WRITE_MEMORY :
			cal_READBYTE(q, TIMEOUT_NACK);
			if (q== (uint8_t)~STM32_CMD_WRITE_MEMORY) {
				return command_write_memory();
			}
			else cal_SENDNACK();
		case STM32_CMD_WRITE_UNPROTECT :
			cal_READBYTE(q, TIMEOUT_NACK);
			if (q== (uint8_t)~STM32_CMD_WRITE_UNPROTECT) {
				return command_write_unprotect();
			}
			else cal_SENDNACK();
		case STM32_CMD_READ_MEMORY :
			cal_READBYTE(q, TIMEOUT_NACK);
			if (q== (uint8_t)~STM32_CMD_READ_MEMORY) {
				return command_read_memory();
			}
			else cal_SENDNACK();
		case STM32_CMD_GETVERSION_READPROTECTION :
			cal_READBYTE(q, TIMEOUT_NACK);
			if (q== (uint8_t)~STM32_CMD_GETVERSION_READPROTECTION) {
				return command_get_version();
			}
			else cal_SENDNACK();
		case STM32_CMD_GO :
			cal_READBYTE(q, TIMEOUT_NACK);
			if (q== (uint8_t)~STM32_CMD_GO) {
				return command_go();
			}
			else cal_SENDNACK();
		case STM32_CMD_EXTENDED_ERASE :
			cal_READBYTE(q, TIMEOUT_NACK);
			if (q== (uint8_t)~STM32_CMD_EXTENDED_ERASE) {
				return command_extended_erase();
			}
			else cal_SENDNACK();
		case STM32_CMD_WRITE_PROTECT :
			cal_READBYTE(q, TIMEOUT_NACK);
			if (q== (uint8_t)~STM32_CMD_WRITE_PROTECT) {
				return command_write_protect();
			}
			else cal_SENDNACK();
		case STM32_CMD_READOUT_PROTECT :
			cal_READBYTE(q, TIMEOUT_NACK);
			if (q == (uint8_t)~STM32_CMD_READOUT_PROTECT) {
				return command_readout_protect();
			}
			else cal_SENDNACK();
		case STM32_CMD_READOUT_UNPROTECT :
			cal_READBYTE(q, TIMEOUT_NACK);
			if (q == (uint8_t)~STM32_CMD_READOUT_UNPROTECT) {
				return command_readout_unprotect();
			}
			else cal_SENDNACK();
		default :
			cal_SENDLOG("-> received command failed \r\n");
			cal_SENDNACK();  //-1 means no command to receive or receive bytes that is not recognized
			break;
	}
	return 0;
}

/*
 * @brief  Receives initialization sequence from the host
 * @param  void
 * @retval 0: successful
 * 		  -1: wrong initialization byte or timeout expired
 */
int32_t command_receiveinit() {
	cal_SENDLOG("-> waiting for init byte \r\n");
	uint8_t p;
	cal_READBYTE(p, TIMEOUT_INIT);
	if(p==STM32_CMD_INIT) {
		GPIOA->BSRR |= GPIO_BSRR_BS0 | GPIO_BSRR_BR1 | GPIO_BSRR_BR2 | GPIO_BSRR_BR3;
		cal_SENDACK();
		cal_SENDLOG("-> init byte received \r\n");
		cal_baudrate();
		//NEED TO WRITE UNPROTECT SECTOR 1 (PAGES 0-3) AS THEY ARE AUTOMATICALLY WRITE PROTECTED
		//START OFF WITH READ PROTECTION ACTIVE BY ERASING OPTION BYTES AS BULK
		// Enter into infinite loop, it will jump away when the command_go is requested
		while (1) {
			receivecommand();
		}
	}
	else {
		GPIOA->BSRR |= GPIO_BSRR_BS0 | GPIO_BSRR_BS1 | GPIO_BSRR_BR2 | GPIO_BSRR_BR3;
		cal_SENDLOG("-> receive init fail \r\n");
		return -1;
	}
}

/*
 * @brief  Checksum calculation according to ST's AN3155 page7
 * @param  array of bytes and its length
 * @retval 8-bit checksum
 */
uint8_t calculatechecksum(uint8_t *data, uint32_t length) {
	uint8_t checksum = 0;
	uint32_t i;
	for( i = 0; i < length; i ++ ) {
	    checksum ^= *data;
		data = data + 1;
	}
	return checksum;
}

/*
 * @brief  Checks if checksum correct according to ST's AN3155 page7
 * @param  32-bit word to compute byte-wise checksum for, length equal to 4, checksum byte to check against
 * @retval 0 if correct
 * 		  -1 if not correct
 *
 * First computes the checksum of "length" bytes by XOR and then checks it against
 * the provided checksum byte
 */
int32_t checkchecksumword(uint32_t data, uint8_t length, uint8_t checksum) {
	uint8_t bytes[4];
	bytes[0]=(data & 0xFF);
	bytes[1]=(data & 0xFF00)>>8;
	bytes[2]=(data & 0xFF0000)>>16;
	bytes[3]=(data & 0xFF000000)>>24;
	if(calculatechecksum(bytes,(uint32_t)length)==checksum) {
		return 0;
	}
	else return -1;
}

/*
 * @brief  Checks if checksum correct according to ST's AN3155 page7
 * @param  array of bytes to compute byte-wise checksum for, length of the array, checksum byte to check against
 * @retval 0 if correct
 * 		  -1 if not correct
 *
 * Similar to checkchecksumword()
 */
int32_t checkchecksumbytes(uint8_t *data, uint32_t length, uint8_t checksum) {
	if(calculatechecksum(data,length)==checksum) {
			return 0;
		}
		else return -1;
}

/*
 * @brief  Helper function to jump to the application. Uses function pointer mechanism.
 * @param  Address to jump to
 * @retval -1 if not successful
 */
int32_t jumptoapp(uint32_t addr) {
	pFunction JumpToApp;
	uint32_t JumpAddress;

	/* The second entry of the vector table contains the reset_handler function. */
	JumpAddress = *(uint32_t*) (addr + 4);

	/* Assign the function pointer. */
	JumpToApp= (pFunction) JumpAddress;

	/* Initialize user application's Stack Pointer. */
	__set_MSP(*(uint32_t*) addr);

	/* Jump!. */
	JumpToApp();

	return 0;
}


/* Protocol commands handlers ----------------------------------------------- */
/* -------------------------------------------------------------------------- */
/*
 * @brief  Get commands implemented in the device side
 * @param  none
 * @retval 0 if successful
 * 		  -1 in unsuccseful
 */
int32_t command_get_command() {
	cal_SENDLOG("-> cmd: get command \r\n");
	cal_SENDACK();
	cal_SENDBYTE(0x0B);
	cal_SENDBYTE(0x10);
	cal_SENDBYTE(STM32_CMD_GET_COMMAND);
	cal_SENDBYTE(STM32_CMD_GETVERSION_READPROTECTION);
	cal_SENDBYTE(STM32_CMD_GET_ID);
	cal_SENDBYTE(STM32_CMD_READ_MEMORY);
	cal_SENDBYTE(STM32_CMD_GO);
	cal_SENDBYTE(STM32_CMD_WRITE_MEMORY);
	cal_SENDBYTE(STM32_CMD_ERASE);
	cal_SENDBYTE(STM32_CMD_WRITE_PROTECT);
	cal_SENDBYTE(STM32_CMD_WRITE_UNPROTECT);
	cal_SENDBYTE(STM32_CMD_READOUT_PROTECT);
	cal_SENDBYTE(STM32_CMD_READOUT_UNPROTECT);
	cal_SENDACK();
	cal_SENDLOG("\r\n-> cmd: get command terminated \r\n");
	return 0;
}

/*
 * @brief  Get bootloader version and read protection status
 * @param  none
 * @retval 0 if successful
 * 		  -1 in unsuccessful
 */
int32_t command_get_version() {
	cal_SENDLOG("-> cmd: get version \r\n");
	cal_SENDACK();
	cal_SENDBYTE(BLVERSION);
	cal_SENDBYTE(0x00);
	cal_SENDBYTE(0x00);
	cal_SENDACK();
	cal_SENDLOG("-> cmd: get version terminated\r\n");
	return 0;
}

/*
 * @brief  Get PID
 * @param  none
 * @retval 0 if successful
 * 		  -1 in unsuccessful
 */
int32_t command_get_id() {
	cal_SENDLOG("-> cmd: get ID \r\n");
	cal_SENDACK();
	cal_SENDBYTE(0x01);
	cal_SENDBYTE(0x04);
	cal_SENDBYTE(hil_getidbyte2());
	cal_SENDACK();
	cal_SENDLOG("-> cmd: get ID terminated\r\n");
	return 0;
}

/*
 * @brief  Read Device Memory
 * @param  none
 * @retval 0 if successful
 * 		  -1 in unsuccessful
 */
int32_t command_read_memory() {
	cal_SENDLOG("-> cmd: read memory \r\n");
	uint8_t checksum, number;
	uint32_t addr, i;
	uint32_t temp;

	/* Check ROP. */
	//if (hil_ropactive())  {cal_sendbyte(STM32_COMM_NACK); return -1;}
	cal_SENDACK();

	/* Receive and validate address. */
	if(cal_receiveword((uint32_t *)&addr, TIMEOUT_NACK) == -1) return -1;  //data,address
	if(cal_receivebyte((uint8_t *)&checksum, TIMEOUT_NACK) == -1) return -1;  //checksum
	if(!hil_validateaddr(addr) == 1) {cal_sendbyte(STM32_COMM_NACK); return -1;}
	if(checkchecksumword(addr,4,checksum) == -1) {cal_sendbyte(STM32_COMM_NACK); return -1;}
	cal_SENDACK();

	/* Receive number of bytes to read. */
	if(cal_receivebyte((uint8_t *)&number, TIMEOUT_NACK) == -1) return -1;
	if(cal_receivebyte((uint8_t *)&checksum, TIMEOUT_NACK) == -1) return -1;
	if(checkchecksumbytes(&number,1,checksum) == -1) {cal_sendbyte(STM32_COMM_NACK); return -1;}
	cal_SENDACK();

    /* Send Data. */
	/* Word-by-word as the returned value by hil_readFLASH */
	for (i = 0;i < (number+1);i=i+4) {

		temp = hil_readFLASH(addr+i);

		/*
		 * Need of delays found during debug
		 * but is only needed for CAN transmission
		 * when used with Pike's USB-CAN adapter
		 * because of packet loss on the RX side
		 * (guess is that the adapted does not keep up with such a fast packet burst)
		 * Using Martino's CAN sniffer, no delays are needed
		 */
		cal_SENDBYTE(temp & 0xFF);
		delay(999);
		cal_SENDBYTE((temp>>8) & 0xFF);
		delay(999);
		cal_SENDBYTE((temp>>16) & 0xFF);
		delay(999);
		cal_SENDBYTE(temp>>24);
		delay(999);

    }
	cal_SENDLOG("-> cmd: read memory terminated \r\n");
	return 0;
}

/*
 * @brief  Go executing the application code
 * @param  none
 * @retval 0 if successful
 * 		  -1 in unsuccessful
 * Return 2 ACK or 1 ACK? Refer to ST's AN3155 for this ambiguity
 */
int32_t command_go() {
	uint8_t checksum;
	uint32_t addr;
	//if (hil_ropactive())  {cal_sendbyte(STM32_COMM_NACK); return -1;}
	cal_SENDACK();

	/* Receive address and its checksum and validate it. */
	cal_READWORD(addr,TIMEOUT_NACK);
	cal_READBYTE(checksum,TIMEOUT_NACK); //checksum
	if(checkchecksumword(addr,4,checksum) == -1) {cal_SENDNACK();}
	if(!hil_validateaddr(addr) == 1) {cal_SENDNACK();}
	cal_SENDACK();

	//cal_SENDACK();

	/* Jump! */
	jumptoapp(addr);

	return 0;
}

/*
 * @brief  Write device memory
 * @param  none
 * @retval 0 if successful
 * 		  -1 in unsuccseful
 */
int32_t command_write_memory() {

	uint32_t addr;
	uint8_t number, checksum;
	uint8_t bytes[4] = {0, 0, 0, 0};
	uint32_t i, j;

	//if (hil_ropactive())  {cal_sendbyte(STM32_COMM_NACK); return -1;}
	cal_SENDACK();

	/* Receive address. */
	cal_READWORD(addr,TIMEOUT_NACK);
	cal_READBYTE(checksum,TIMEOUT_NACK);
	if(checkchecksumword(addr,4,checksum) == -1) {cal_SENDNACK();}
	cal_SENDACK();

	/* Receive data packet. */
	/* it is actually number + 1 to be received according to the docs
     * expecting nummber+1 bytes, and add one place to append number itself at the end
     * range: 0 - number+1, places available: number+2
     */
	cal_READBYTE(number, TIMEOUT_NACK);
	uint8_t databuffer[number+2];
	for(i=0;i<number+1;i++) {
		if(cal_receivebyte(databuffer+i, TIMEOUT_NACK)) return -1; //41 41 a
	}
	databuffer[number+1]=number;
	cal_READBYTE(checksum,TIMEOUT_NACK);
	if(checkchecksumbytes(databuffer,number+2,checksum)==-1) {cal_SENDNACK();}
	else {
		switch (hil_validateaddr(addr)) {
			case 1:  //case FLASH
				for (j=0;j<(number+1);j=j+4) {
					for (i=0; (i<4 && (j+i)<=number); i++) {
						bytes[i]=databuffer[j+i];
					}
				FLASH_ProgramWord(addr+j,*(uint32_t*)bytes);
				}
				cal_SENDACK();
				break;
			case 0:  //case RAM
				//UNTESTED
				for (j=0;j<number+1;j++) {
				*(&addr+j)=*(databuffer+j);
				}
				break;
			default: //case option bytes
				//UNTESTED
				if(addr==0x1FFFF800) {
					FLASH_ProgramOptionByteData(addr, *databuffer);
					if(cal_sendbyte(STM32_COMM_ACK)==-1) return -1;
					hil_reset();
					break;
				}
			else {
				cal_SENDNACK();
				return -1;
			}
		}
	}
	return 0;
}

/*
 * @brief  Erase device memory
 * @param  none
 * @retval 0 if successful
 * 		  -1 in unsuccseful
 */
int32_t command_erase() {
	uint8_t number, checksum;
	uint32_t pageaddr;
	uint8_t i;

	cal_SENDLOG("-> cmd: erase memory started, acking \r\n");
	//if (hil_ropactive())  {cal_sendbyte(STM32_COMM_NACK); return -1;}
	cal_SENDACK();

	/* Read number of pages to be erased. */
	cal_READBYTE(number, TIMEOUT_NACK);

	/* If global erase. */
	if (number==0xFF) {
		cal_SENDLOG("-> cmd: global erase requested, starting global erase \r\n");
		hil_globalerasememory();
		cal_SENDLOG("-> cmd: global erase terminated, acking \r\n");
		cal_SENDACK();
		else return 0;
	}

	/* If pagewise erase. */
	else {

			//UNTESTED!

			cal_SENDLOG("-> cmd: pagewise erase requested \r\n");
			uint8_t databuffer[number+2];
			for(i=0;i<number+1;i++) {
				if(cal_receivebyte(databuffer+i, TIMEOUT_NACK)) return -1;//receive page codes
			}
			databuffer[number+1]=number;
			cal_READBYTE(checksum, TIMEOUT_NACK);
			if(checkchecksumbytes(databuffer,number+2,checksum)==-1) cal_SENDNACK();
			cal_SENDLOG("-> cmd: checksum correct, starting pagewise erase \r\n");
			for (i=0;i<number+1;i++) {
			   pageaddr = (databuffer[i]-1)*FLASHPAGESIZE+FLASHbase;
			   FLASH_ErasePage(pageaddr);
			}
			cal_SENDLOG("-> cmd: pagewise erase terminated, acking \r\n");
			cal_SENDACK();
		}
	return 0;
}


//DOES NOT HAVE TO BE IMPLEMENTED MANDATORILY. Erase and Extended Erase commands are mutually exclusive
//UNTESTED
int32_t command_extended_erase() {
	uint16_t *q = 0x0000;
	if (hil_ropactive())  {cal_sendbyte(STM32_COMM_NACK); return -1;}
	if(cal_sendbyte(STM32_COMM_ACK)==-1) return -1;
	//receive 2 bytes
	switch (*q) {
	case 0xFFFF:
		//receive checksum 2 bytes
		//check it
		//yes
		hil_globalerasememory();
		if(cal_sendbyte(STM32_COMM_ACK)==-1) return -1;
		//NO
		if(cal_sendbyte(STM32_COMM_NACK)==-1) return -1;
		    break;
	case 0xFFFE:
		//receive checksum 2 bytes
		//check it
		//yes
		hil_erasebank1();
		if(cal_sendbyte(STM32_COMM_ACK)==-1) return -1;
		//NO
		if(cal_sendbyte(STM32_COMM_NACK)==-1) return -1;
			break;
	case 0xFFFD:
		//receive checksum 2 bytes
		//check it
		//yes
		hil_erasebank2();
		if(cal_sendbyte(STM32_COMM_ACK)==-1) return -1;
		//NO
		if(cal_sendbyte(STM32_COMM_NACK)==-1) return -1;
			break;
	default:
		//receive page codes and checksum
		//checksum ok
		//no
		if(cal_sendbyte(STM32_COMM_NACK)==-1) return -1;
		//yes
		hil_erasecorrespondingpage(0xFFFF);
		if(cal_sendbyte(STM32_COMM_ACK)==-1) return -1;
		break;
	}
	return 0;
}

// UNTESTED
int32_t command_write_protect() {
	uint8_t number;
	uint8_t checksum ;
	uint8_t i;
	uint32_t sector;
	if (hil_ropactive())  {cal_sendbyte(STM32_COMM_NACK); return -1;}
	if(cal_sendbyte(STM32_COMM_ACK)==-1) return -1;
	cal_receivebyte((uint8_t *)&number, TIMEOUT_NACK); //number of sectors to be protected (1 byte)
	uint8_t databuffer[number+2];  //sector code
	for(i=0;i<number+1;i++) {
		if(cal_receivebyte(databuffer+i, TIMEOUT_NACK)) return -1;//receive sector codes
	}
	databuffer[number+1]=number;
	if(cal_receivebyte((uint8_t *)&checksum, TIMEOUT_NACK) == -1) return -1;
	if(checkchecksumbytes(databuffer,number+2,checksum)==-1) {cal_sendbyte(STM32_COMM_NACK); return -1;};
	for (i=0;i<number+1;i++) {
    sector = (databuffer[i]-1)*SECTORSIZE+FLASHbase;   //what is sector codes received. this calculation might be wrong
    hil_enablewriteprotectionflashmen(sector);
	}
	if(cal_sendbyte(STM32_COMM_ACK)==-1) return -1;
	hil_reset();
	return 0;
}

//UNTESTED
int32_t command_write_unprotect() {
	//if (hil_ropactive())  {cal_sendbyte(STM32_COMM_NACK); return -1;}
	cal_SENDLOG("-> cmd: write unprotect entered, acking \r\n");
	cal_SENDACK();
	hil_removewriteprotectionflashmem();
	cal_SENDLOG("-> cmd: write protection removed, acking \r\n");
	cal_SENDACK();
	cal_SENDLOG("-> cmd: write unprotect ended, generating system reset \r\n");
	hil_reset();
	return 0;
}

//UNTESTED
int32_t command_readout_protect() {
	if (hil_ropactive())  {cal_sendbyte(STM32_COMM_NACK); return -1;}
	if(cal_sendbyte(STM32_COMM_ACK)==-1) return -1;
	hil_enablerop();
	if(cal_sendbyte(STM32_COMM_ACK)==-1) return -1;
	hil_reset();
	return 0;
}

//UNTESTED
int32_t command_readout_unprotect() {
	if(cal_sendbyte(STM32_COMM_ACK)==-1) return -1;
	hil_disablerop();//Flash Mass Erased :(
	if(cal_sendbyte(STM32_COMM_ACK)==-1) return -1;
	hil_clearram();
	hil_reset();
	return 0;
}

/**************************** Politecnico di Milano ************END OF FILE****/
