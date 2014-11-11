////////////////////////////////////////////////////////////////////////////////
//
//  File           : smsa_driver.c
//  Description    : This is the driver for the SMSA simulator.
//
//   Authors        : Dylan Prescott & Dylan Prescott
//   Last Modified : 
//

// Include Files

// Project Include Files
#include <smsa_driver.h>
#include <cmpsc311_log.h>

// Defines

// Functional Prototypes

//
// Global data

// Interfaces
uint32_t getDrum(uint32_t address)
{
	return address >> 16; //Shift address over 16 bits so we get the ID of the drum to perform further operations.
}

uint32_t getBlock(uint32_t address)
{
	return(address&0xff00) >> 8; //returning the last 8 bits of a unint32 address representing the block ID.
}

uint32_t getOPCode(int operationCode, int drumIdentifier, int blockIdentifier)
{
	uint32_t operation;
	operation = (operationCode << 26); //operation is set to the first 6 bits which signify the smsa operation.
	operation = (operation | (drumIdentifier << 22)); //If drumID becomes modified, shift into drum instruction code location
	operation = (operation | blockIdentifier); //Already in the blockIdentifier instruction code location, checking for modifications
	return operation;
}
////////////////////////////////////////////////////////////////////////////////
//
// Function     : smsa_vmount
// Description  : Mount the SMSA disk array virtual address space
//
// Inputs       : none
// Outputs      : -1 if failure or 0 if successful

int smsa_vmount( void ) 
{
	if(smsa_operation(getOPCode(SMSA_MOUNT,0,0),NULL) < 0) //If mounting fails, return negative one and print an error
	{
		logMessage(LOG_ERROR_LEVEL,"Failure to Mount...NEWB!");
		return -1;
	}
	smsa_operation(getOPCode(SMSA_FORMAT_DRUM,0,0),NULL); //Formats the current drum for mounting.
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : smsa_vunmount
// Description  :  Unmount the SMSA disk array virtual address space
//
// Inputs       : none
// Outputs      : -1 if failure or 0 if successful

int smsa_vunmount( void )  
{
	if(smsa_operation(getOPCode(SMSA_UNMOUNT,0,0),NULL) < 0) //If Unmounting fails, return negative one and print an error.
	{
		logMessage(LOG_ERROR_LEVEL, "Failure to Unmount...NEWB!");
		return -1;
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : smsa_vread
// Description  : Read from the SMSA virtual address space
//
// Inputs       : addr - the address to read from
//                len - the number of bytes to read
//                buf - the place to put the read bytes
// Outputs      : -1 if failure or 0 if successful

int smsa_vread( SMSA_VIRTUAL_ADDRESS addr, uint32_t len, unsigned char *buf ) 
{
	if(len == 0)
	{
		return 0;
	}

	unsigned char tmp[SMSA_BLOCK_SIZE];//Temporary variable stores the contents to be read
	int reader = 0, drum, block, bitDiff, firstBlock, counter = 0;
	drum = getDrum(addr);//Get the specific drum address
	block = getBlock(addr);//Get the specific block address
	firstBlock = block;//initalize first block to block address.
	bitDiff = addr & 0xff; //****************************************

	while(reader < len)
	{
		if(drum < 0 || drum > SMSA_DISK_ARRAY_SIZE) //Error checking for drum in range
		{
			logMessage(LOG_ERROR_LEVEL, "Out of bounds drum");
			return -1;
		}
		else if(block < 0 || block > SMSA_MAX_BLOCK_ID) //Error checking for block in range.
		{
			logMessage(LOG_ERROR_LEVEL, "Out of bounds block");
			return -1;
		}
		else if(block == firstBlock) //
		{
			counter = bitDiff; //***********************************************
			smsa_operation(getOPCode(SMSA_SEEK_DRUM, drum, 0),NULL);  //Finding the correct drum to write
			smsa_operation(getOPCode(SMSA_SEEK_BLOCK, 0, block),NULL); //Finding the correct block to write
		}
		else
		{
			counter = 0; //if none of the conditions hold above set counter to 0
		}

		if(block == SMSA_BLOCK_SIZE) //If the block size is 256
		{
			block = 0;	//set block to 0
			drum++; //go to next drum
			smsa_operation(getOPCode(SMSA_SEEK_DRUM, drum, 0),NULL); //Find the next correct drum to write
		}
		smsa_operation(getOPCode(SMSA_DISK_READ, drum, block),tmp); //Read from disk
		block++;	//go to next block

		while((len>reader) && (SMSA_BLOCK_SIZE > counter)) //read the contents of the drum into the temporary buffer.
		{
			buf[reader] = tmp[counter];
			reader++;
			counter++;
		}
	}
	return 0;
}

////////////////////////////////////////////////////////////////////////////////
//
// Function     : smsa_vwrite
// Description  : Write to the SMSA virtual address space
//
// Inputs       : addr - the address to write to
//                len - the number of bytes to write
//                buf - the place to read the read from to write
// Outputs      : -1 if failure or 0 if successful

int smsa_vwrite( SMSA_VIRTUAL_ADDRESS addr, uint32_t len, unsigned char *buf )  
{
	if(len == 0) // If length equals 0 than return without writing to disk.
	{
		return 0;
	}

	int counter, reader = 0;
	unsigned char tmp[SMSA_BLOCK_SIZE]; //Temporary variable stores the contents to be written
	uint32_t firstBlock, drum, block, bitDiff;
	
	drum = getDrum(addr); //Get the specific drum address
	block = getBlock(addr); //Get the specific block address
	firstBlock = block;	//initalize first block to block address.
	bitDiff = addr & 0xff; //masking the address will detect changes in binary operations.

	while(reader < len) //while not at the end of the address
	{

		if(drum < 0 || drum > SMSA_DISK_ARRAY_SIZE) //Error checking for drum in range
		{
			logMessage(LOG_ERROR_LEVEL, "Out of bounds drum");
			return -1;
		}
		else if(block < 0 || block > SMSA_MAX_BLOCK_ID) //Error checking for block in range.
		{
			logMessage(LOG_ERROR_LEVEL, "Out of bounds block");
			return -1;
		}
		else if(block == firstBlock) //
		{
			counter = bitDiff; 
			smsa_operation(getOPCode(SMSA_SEEK_DRUM, drum, 0),NULL);  //Finding the correct drum to write
			smsa_operation(getOPCode(SMSA_SEEK_BLOCK, 0, block),NULL); //Finding the correct block to write
		}
		else
		{
			counter = 0; //if none of the conditions hold above set counter to 0
		}

		if(block == SMSA_BLOCK_SIZE) //If the block size is 256
		{
			block = 0;	//set block to 0
			drum++; //go to next drum
			smsa_operation(getOPCode(SMSA_SEEK_DRUM, drum, 0),NULL); //Find the next correct drum to write
		}
		smsa_operation(getOPCode(SMSA_DISK_READ, drum, block),tmp); //Read from disk
		block++;	//go to next block

		while((len>reader) && (SMSA_BLOCK_SIZE > counter)) //read the contents of the drum into the temporary buffer.
		{
			tmp[counter] = buf[reader];
			reader++;
			counter++;
		}
		smsa_operation(getOPCode(SMSA_SEEK_BLOCK, 0, block-1),NULL);	//go to the correct block
		smsa_operation(getOPCode(SMSA_DISK_WRITE, drum, block),tmp); //write the drum and block addresses to Disk.
	}
	return 0;
}
