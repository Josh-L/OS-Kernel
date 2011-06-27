#ifndef __SYSTEM__
#define __SYSTEM__
#include "rtx_inc.h"
#include "Hex_to_ASCII.h"
#include "dbug.h"

#define KERNEL_STACK_SIZE	2048 // Another temporary value that is suitable for now
#define NUM_PRIORITIES		5

#define MEM_BLK_SIZE 128
#define NUM_MEM_BLKS 32

#define NUM_PROCESSES 7
#define TEST_PROC 6


extern BYTE __end;

//Structs
struct s_message
{
        UINT8 sender_id;
        UINT8 dest_id;
        UINT8 type_message;
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
    UINT8   m_state;       // 0 memory blocked, 1 ready, 2 running, 4 message blocked
    UINT32  m_stack;
    VOID    (*m_entry)();
	struct s_message_queue msg_queue;
	struct s_message_queue_item msg_queue_slots[NUM_PROCESSES];
	UINT8 	i_process;
	
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
VOID    sys_init();
VOID    scheduler( VOID );
SINT8   release_processor();
SINT8   send_message(UINT8 process_ID, VOID * MessageEnvelope);
VOID  * receive_message(UINT8 * sender_ID);
//SINT8 pop(UINT8 priority, struct s_pcb ** catcher);
//SINT8 push(UINT8 priority, struct s_pcb * new_back);
SINT8 push(struct s_pcb_queue * queue, struct s_pcb_queue_item slots[], struct s_pcb * new_back);
SINT8 pop(struct s_pcb_queue * queue, struct s_pcb_queue_item slots[], struct s_pcb ** catcher);
SINT8   set_process_priority(UINT8 process_ID, UINT8 priority);
SINT8 get_process_priority(UINT8 process_ID);
VOID iterate(UINT8 priority);
VOID* request_memory_block();
SINT8 release_memory_block(VOID * memory_block);
SINT8 message_push(struct s_message_queue * queue, struct s_message_queue_item slots[], struct s_message * new_back);
SINT8 message_pop(struct s_message_queue * queue, struct s_message_queue_item slots[], struct s_message ** catcher);

/*Timing Service*/
SINT8 delayed_send(int process_ID, void * MessageEnvelope, int delay);
VOID null_process();

#endif
