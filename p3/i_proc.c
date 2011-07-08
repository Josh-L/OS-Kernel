#include "system.h"

volatile BYTE CharIn = '!';
volatile BOOLEAN Caught = TRUE;
volatile BYTE CharOut = '\0';
CHAR StringHack[] = "You Typed a Q\n\r";

void uart(){
	while(1){  
	rtx_dbug_outs("UART :");
    
        if( !Caught )
        {
            Caught = TRUE;
            CharOut = CharIn;
            /* Nasty hack to get a dynamic string format, 
             * grab the character before turning the interrupts back on. 
             */
            StringHack[12] = CharIn;
            /* enable tx interrupts  */
            SERIAL1_IMR = 3;
	        /* Now print the string to debug, 
             * note that interrupts are now back on. 
             */
            rtx_dbug_outs( StringHack );
		}
		Caught = TRUE;
		CharIn = '\0';
		release_processor();

		
	}
}

void timer(){

}

void c_serial_handler(){
	
    BYTE temp;
    temp = SERIAL1_UCSR;
    if( temp & 1 )
    {
        CharIn = SERIAL1_RD;
        Caught = FALSE;
		iProcessInterrupted = 1;
		rtx_dbug_outs("CALLING SCHEDULER \n\r");
		SERIAL1_IMR = 2;
		scheduler();
    }
    else if ( temp & 4 )
    {
		SERIAL1_WD = CharOut;
        SERIAL1_IMR = 2;

    }
	
	return;
}

