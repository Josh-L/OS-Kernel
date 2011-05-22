#include "Hex_to_ASCII.h"
#include "memory/dbug.h"

VOID printHexAddress(UINT32 hexAddress)
{
	/***************************************************************
	* This function will take a UINT32 hex address and will output *
	* it as an ASCII string. Each character is printed within each *
	* loop cycle so that a buffer is not required. The syntax may  *
	* need to be cleaned up, but the logic is verified.            *
	****************************************************************/
	
	UINT32 divider = 0x10000000;
	CHAR ascii = 0x0; // This might work as a UINT8

	// Output the 0x part of the address
	rtx_dbug_outs((CHAR *)"0x");

	UINT8 i = 0;
	for (i = 0; i < 8; i++)
	{
		ascii = hexAddress / divider;
		hexAddress -= (ascii * divider);
		divider /= 0x10;
		
		// Now output the ASCII character
		switch (ascii)
		{
			case 0xf:
				rtx_dbug_out_char('F');
				break;
			case 0xe:
				rtx_dbug_out_char('E');
				break;
			case 0xd:
				rtx_dbug_out_char('D');
				break;
			case 0xc:
				rtx_dbug_out_char('C');
				break;
			case 0xb:
				rtx_dbug_out_char('B');
				break;
			case 0xa:
				rtx_dbug_out_char('A');
				break;
			default:
				ascii += 0x30;
				rtx_dbug_out_char(ascii);
				break;
		}
	}
}
