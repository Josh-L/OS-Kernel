#include "rtx_inc.h"
#include "dbug.h"

/*
 * Global Variables
 */
SINT32 Counter = 0;
UINT8 g_hours = 23;
UINT8 g_minutes = 59;
UINT8 g_seconds = 50;
CHAR g_output_buffer[] = "\r23:59:50";

/*
 * gcc expects this function to exist
 */
int __main( void )
{
    return 0;
}

VOID uart1_print_char( CHAR c )
{
    /*
	* Acknowledge the interrupt
	SERIAL1_UCSR is called USR for reading, UCSR for writing
	*/
    
	/* See if port is ready to accept data */
    while ((SERIAL1_UCSR & 4) == 0);
    SERIAL1_WD = c;
}

SINT32 uart1_print_chars( CHAR* s )
{
	if ( s == NULL )
	{
		return RTX_ERROR;
	}
	else
	{
		while ( *s != '\0' )
		{
			uart1_print_char( *s++ );
		}
	}
    return RTX_SUCCESS;
}

/*
 * This function is called by the assembly STUB function
 */
VOID c_timer_handler( VOID )
{
    Counter++;
    if (Counter >= 100)
    {
        Counter = Counter - 100;
        g_seconds++;
        if (g_seconds >= 60)
        {
            g_seconds = 0;
            g_minutes++;
            if (g_minutes >= 60)
            {
                g_minutes = 0;
                g_hours++;
                if (g_hours >= 24)
                {
                    g_hours = 0;
                }
            }
        }
        
        // Int to string function
        UINT8 s = (g_seconds % 0xa);
        g_output_buffer[8] = (BYTE)(s + 0x30);
        g_output_buffer[7] = (BYTE)(((g_seconds - s)/0xa) + 0x30);
        s = (g_minutes % 0xa);
        g_output_buffer[5] = (BYTE)(s + 0x30);
        g_output_buffer[4] = (BYTE)(((g_minutes - s)/0xa) + 0x30);
        s = (g_hours % 0xa);
        g_output_buffer[2] = (BYTE)(s + 0x30);
        g_output_buffer[1] = (BYTE)(((g_hours - s)/0xa) + 0x30);
        
        uart1_print_chars(g_output_buffer);
    }
    
    TIMER0_TER = 2;
}




SINT32 coldfire_vbr_init( VOID )
{
    /*
     * Move the VBR into real memory
     */
    asm( "move.l %a0, -(%a7)" );
    asm( "move.l #0x10000000, %a0 " );
    asm( "movec.l %a0, %vbr" );
    asm( "move.l (%a7)+, %a0" );
    
    return RTX_SUCCESS;
}



/*
 * Entry point, check with m68k-coff-nm
 */
int main( void )
{
    UINT32 mask;
    
    /* Disable all interupts */
    asm( "move.w #0x2700,%sr" );

    coldfire_vbr_init();

    /*
     * Store the timer ISR at auto-vector #6
     */
    asm( "move.l #asm_timer_entry,%d0" );
    asm( "move.l %d0,0x10000078" );

    /*
     * Setup to use auto-vectored interupt level 6, priority 3
     */
    TIMER0_ICR = 0x9B;

    /*
     * Set the reference counts, ~10ms
     */
    TIMER0_TRR = 1758;

    /*
     * Setup the timer prescaler and stuff
     */
    TIMER0_TMR = 0xFF1B;
    
    /*
     * Set the interupt mask
     */
    mask = SIM_IMR;
    mask &= 0x0003fdff;
    SIM_IMR = mask;
    
    /* Enable all interupts */
    asm( "move.w #0x2000,%sr" );
    
    uart1_print_chars(g_output_buffer);
    
    /* Let the timer interrupt fire, lower running priority */
    asm( "move.w #0x2000,%sr" );
    
    Counter = 0;
    while(TRUE);
    return 0;
}
