#include "rtx_inc.h"

#define PROCESS_STACK_SIZE 64 //Temporary for now, needs to be changed once appropriate value is agreed upon.

//Function prototypes
void  sys_init();
int   release_processor();
void  (*null_process)();
int   send_message(int process_ID, void * MessageEnvelope);
void* receive_message(int * sender_ID);
int   set_process_priority(int process_ID, int priority);
int   get_process_priority(int process_ID)
int   pop(struct s_queue * q, struct s_node * catcher)
int   push(struct s_queue * q, struct s_node * new_back)

//Structs
struct s_app_process{
    UINT32  m_process_ID;
    UINT8   m_priority;
    UINT32  m_stack[PROCESS_STACK_SIZE];
    UINT32  m_start_address;
};

struct s_sys_process{
    UINT32  m_process_ID;
    UINT8   m_priority;
    UINT32  m_stack[PROCESS_STACK_SIZE];
    UINT32  m_start_address;
    BOOLEAN m_is_iprocess; //Non-zero indicates that this process is an i-process.
};

struct s_queue{
    UINT16 length;
    s_node *m_front;
    s_node *m_back;
};

struct s_node{
    UINT32 *m_data;
    UINT32 *m_next;
};

//Function definitions
void sys_init()
{
    //Initialize process queues, process structs etc. in here
}

int release_processor()
{

    return 0;
}

void (*null_process)()
{
    while(1)
    {
        release_processor();
    }
}

int send_message(int process_ID, void * MessageEnvelope)
{
    
    return 0;   
}

void * receive_message(int * sender_ID)
{

}

int set_process_priority(int process_ID, int priority)
{

    return 0;
}

int get_process_priority(int process_ID)
{

    return 0;
}

int pop(struct s_queue * q, struct s_node * catcher)
{
    if(q->length <= 0)
    {
        return -1;
    }
    catcher = q->m_front;
    q->m_front = q->m_front->m_next;
    q->length--;
    return 0;
}

int push(struct s_queue * q, struct s_node * new_back)
{
    if(q->length == 0)
    {
        q->front = new_back;
    }
    q->m_back->m_next = new_back;
    new_back->m_next = NULL;
    q->m_back = new_back;
    q->length++;
    return 0;
}



