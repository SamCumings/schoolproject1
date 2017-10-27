#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

sem_t *MxFL, *MxL1, *MxL2, *SCFL,*SCL1,*SCL2,*LastSem;
/*shared memory and semaphore stuff*/
//https://stackoverflow.com/questions/5656530/how-to-use-shared-memory-with-linux-in-c
void* create_shared_memory(size_t size)
{   
    //gives it read and write
    int protection = PROT_READ | PROT_WRITE;
    //memory is shared between processes but only this process and it's children.
    int visibility = MAP_ANONYMOUS | MAP_SHARED;
    //makes the shared memory block
    return mmap(NULL, size, protection, visibility, 0, 0);
}



//http://www.geeksforgeeks.org/generic-linked-list-in-c-2/
/* A linked list node */
struct Node
{
    // Any data type can be stored in this node
    void  *data;

    struct Node *next;
};

/* Function to add a node at the beginning of Linked List.
   This function expects a pointer to the data to be added
   and size of the data type */
void push(struct Node** head_ref, void *new_data, size_t data_size)
{
    // Allocate memory for node
    struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
 
    new_node->data  = malloc(data_size);
    new_node->next = (*head_ref);
 
    // Copy contents of new_data to newly allocated memory.
    // Assumption: char takes 1 byte.
    int i;
    for (i=0; i<data_size; i++)
        *(char *)(new_node->data + i) = *(char *)(new_data + i);
 
    // Change head pointer as new node is added at the beginning
    (*head_ref)    = new_node;
}
 
/* Function to print nodes in a given linked list. fpitr is used
   to access the function to be used for printing current node data.
   Note that different data types need different specifier in printf() */
void printList(struct Node *node, void (*fptr)(void *))
{
    while (node != NULL)
    {
        (*fptr)(node->data);
        node = node->next;
    }
}

struct Node* unlink_node(struct Node** head_ref)
{
    struct Node* temp = (*head_ref);
    
}

void link_node (struct Node** head_ref, struct Node *a)
{

}

void produce_blah(struct Node* a)
{

}

void calc_blah(struct Node* a, struct Node* b)
{

}

void consume_blah(struct Node* c)
{

}

void free_sem(){
    sem_unlink( "Mutex_Free_List"); 
    sem_unlink( "Mutex_List1");
    sem_unlink( "Mutex_List2"); 
    sem_unlink( "Count_Free_List");
    sem_unlink( "Count_List1");
    sem_unlink( "Count_List2");
    sem_unlink( "Last_Sem");
    sem_close(MxFL); 
    sem_close(MxL1);
    sem_close(MxL2); 
    sem_close(SCFL); 
    sem_close(SCL1);       
    sem_close(SCL2);       
    sem_close(LastSem);

}


void produce_pro(){

}
void calc_pro(){

}
void consume_pro(){

}
int main (void)
{
    sem_t *MxFL, *MxL1, *MxL2, *SCFL,*SCL1,*SCL2,*LastSem;
    int n = 10;
    int i =0;
    pid_t pid,pro_id,calc_id,con_id;    

    MxFL = sem_open ("Mutex_Free_List", O_CREAT | O_EXCL, 0644, 1); 
    MxL1 = sem_open ("Mutex_List1", O_CREAT | O_EXCL, 0644, 1); 
    MxL2 = sem_open ("Mutex_List2", O_CREAT | O_EXCL, 0644, 1); 
    SCFL = sem_open ("Count_Free_List", O_CREAT | O_EXCL, 0644, n-1); 
    SCL1 = sem_open ("Count_List1", O_CREAT | O_EXCL, 0644, 0); 
    SCL2 = sem_open ("Count_List2", O_CREAT | O_EXCL, 0644, 0); 
    LastSem = sem_open ("Last_Sem", O_CREAT | O_EXCL, 0644, 1); 
    //make child processes https://stackoverflow.com/questions/16400820/c-how-to-use-posix-semaphores-on-forked-processes
    pid=fork();
    if(pid<0){
        //check for errors and make sure the semphores doen't exist forever.
        free_sem();
        printf("fork_error\n");       
    }
    //child process
    else if(pid == 0){
        produce_pro();
    }else{
        pid=fork();
        if(pid<0){
            free_sem();
            printf("fork_error\n");
        }
        else if(pid==0){
            calc_pro();
        }else{
            pid=fork();
            if(pid<0){
                free_sem();
                 printf("fork_error\n");
            }
            else if(pid==0){
                consume_pro();
            }
            else{
                if(pid!=0){
                    while((pid = waitpid(-1,NULL,0))){
                        if(errno==ECHILD) break;
                    }
                    printf("\n We Done \n");

                    //need to free memory here I think.

                    //free semaphores
                    free_sem();
                    exit(0); 
                }

            }
        }

    }
    return 0;
}
