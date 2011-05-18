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
CHAR g_output_buffer_snapshot[] = "\r23:59:50";
UINT8 g_out_char = 0;

/*
 * gcc expects this function to exist
 */
int __main( void )
{
    return 0;
}

VOID uart1_out_char( VOID )
{
    SERIAL1_WD = g_output_buffer_snapshot[g_out_char];
    if(g_out_char >= 9)
    {
        g_out_char = 0;
        SERIAL1_IMR = 0x02;
    }
    else
    {
        g_out_char++;
    }
}

/*
 * This function is called by the assembly STUB function
 */
VOID c_timer_handler( VOID )
{
    Counter++;
    if (Counter >= 1)
    {
        Counter = 0;
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
        
        //Take snapshot of output buffer
        UINT8 i;
        for(i = 0; i < 10; i++)
        {
            g_output_buffer_snapshot[i] = g_output_buffer[i];
        }
        SERIAL1_IMR = 0x01;
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
     * Set the reference counts, ~1s
     */
    TIMER0_TRR = 0xFFFF;

    /*
     * Setup the timer prescaler and stuff
     */
    TIMER0_TMR = 0x2A1D;
    
    /*
	* Store the timer output ISR at user vector #64
	*/
    asm( "move.l #asm_timer_output_char,%d0" );
    asm( "move.l %d0,0x10000100" );
    /* Reset the entire UART */
    SERIAL1_UCR = 0x10;
    /* Reset the receiver */
    SERIAL1_UCR = 0x20;
    /* Reset the transmitter */
    SERIAL1_UCR = 0x30;
    /* Reset the error condition */
    SERIAL1_UCR = 0x40;
    /* Install the interupt */
    SERIAL1_ICR = 0x17;
    SERIAL1_IVR = 64;
    /* enable interrupts on tx only */
    SERIAL1_IMR = 0x01;
	
    /* Setup for transmit only */
    SERIAL1_UCR = 0x06;
    
    /*
     * Set the interupt mask
     */
    mask = SIM_IMR;
    mask &= 0x0003fdff;
    SIM_IMR = mask;
    
    /* Enable all interupts */
    asm( "move.w #0x2000,%sr" );
    
    while(TRUE);
    return 0;
}
