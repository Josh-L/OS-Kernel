#ifndef __SYSTEM__
#define __SYSTEM__
#include "rtx_inc.h"
#include "Hex_to_ASCII.h"
#include "dbug.h"

#define KERNEL_STACK_SIZE	2048 // Another temporary value that is suitable for now
#define NUM_PRIORITIES		5

#define MEM_BLK_SIZE 128
#define NUM_MEM_BLKS 32

#define NUM_PROCESSES 11
#define NUM_TEST_PROCS 6

#define NULL_PROC_ID	0
#define KDC_PROC_ID		7
#define CRT_PROC_ID		8
#define UART_IPROC_ID	9
#define TIMER_IPROC_ID	10

extern BYTE __end;

//Structs
struct delayed_send_request
{
	int exp;      //Counter tracking whether the request has expired yet
	int process_ID; //ID of process to send message to
	VOID * envelope; //Pointer to message envelope
};

struct s_char_queue
{
	struct s_char_queue_item * front;
	struct s_char_queue_item * back;
	UINT8 num_slots;
};

struct s_char_queue_item
{
	struct s_char_queue_item * next;
	char data;
};

struct s_message
{
	int sender_id;
	int dest_id;
	int type;
    char * msg_text;
};

struct s_message_queue
{
	struct s_message_queue_item * front;
	struct s_message_queue_item * back;
	UINT8 num_slots;
};

struct s_message_queue_item
{
	struct s_message_queue_item * next;
	struct s_message * data;
};

struct s_pcb
{
    UINT32  m_process_ID;
    UINT8   m_priority;
    UINT8   m_state;       // 0 memory blocked, 1 ready, 2 running, 3 message blocked
    UINT32  m_stack;
    VOID    (*m_entry)();
	struct	s_message_queue msg_queue;
	struct	s_message_queue_item msg_queue_slots[NUM_PROCESSES];
	UINT8	i_process; // 0 means is not, 1 means it is
};

struct s_pcb_queue_item
{
	struct s_pcb_queue_item * next;
	struct s_pcb * data;
};

struct s_pcb_queue
{
	struct s_pcb_queue_item * front;
	struct s_pcb_queue_item * back;
	UINT8 num_slots;
};

//Function prototypes
VOID	sys_init();
VOID	scheduler( VOID );
VOID	schedule_iproc();
int		release_processor();
int		send_message(int process_ID, VOID * MessageEnvelope);
VOID*	receive_message(int * sender_ID);
SINT8 	push(struct s_pcb_queue * queue, struct s_pcb_queue_item slots[], struct s_pcb * new_back);
SINT8	pop(struct s_pcb_queue * queue, struct s_pcb_queue_item slots[], struct s_pcb ** catcher);
int		set_process_priority(int process_ID, int priority);
int		get_process_priority(int process_ID);
VOID	iterate(UINT8 priority);
VOID*	request_memory_block();
int		release_memory_block(VOID * memory_block);
SINT8	message_push(struct s_message_queue * queue, struct s_message_queue_item slots[], struct s_message * new_back);
SINT8	message_pop(struct s_message_queue * queue, struct s_message_queue_item slots[], struct s_message ** catcher);
int		delayed_send(int process_ID, void * MessageEnvelope, int delay);
SINT32	ColdFire_vbr_init( VOID );
VOID	c_serial_handler();

// System Processes
VOID	null_process();
VOID	kcd();
VOID	crt();

// i-processes
VOID	uart();
VOID	timer();

/*
//System Proc Init Array
struct s_pcb g_sysproc_table[] =
{
	{NULL_PROC_ID, 4, 1, 1024, null_process},
	{UART_ID, 0, 1, 1024, test_proc_1},
	{TIMER_ID, 0, 1, 1024, test_proc_2}
};
*/

#endif
