#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

//https://stackoverflow.com/questions/33293121/how-to-allocate-linked-list-inside-struct-in-shared-memory-c
#define MAX_DLEN 10             // Max. number of list nodes
#define DNULL (MAX_DLEN + 1)    // NULL value

typedef struct DNode DNode;
typedef struct DList DList;

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

struct DNode {
    int data;
    size_t next;
};
struct DList {
    DNode pool[MAX_DLEN]; //our nodes hold an int
    size_t npool;
    size_t pfree;
    size_t head;
};
DList *FreeList,*List1,*List2;

DNode *dnode_alloc(DList* List)
{
    if(List->pfree != DNULL){
        DNode *node = List->pool + List->pfree;

        List->pfree = List->pool[List->pfree].next;
        return node;
    } else {
        if (List->npool < MAX_DLEN) return &List->pool[List->npool++];
    }

    return NULL;
}

void dnode_free(DNode *node, DList* List){
    if(node){
        node->next=List->pfree;
        List->pfree=node-List->pool;

    }
}

DNode *dnode(size_t index,DList*  List)
{
    return (index==DNULL)?NULL : List->pool + index;
}
DNode *dnode_next(const DNode* node, DList* List)
{
    return dnode(node->next, List);    
}

void link_node (size_t *head, DNode* node , DList *New_List)
{
    DNode *temp = dnode_alloc(New_List);
    temp = node;
    node->next=*head;
    *head = node - New_List->pool;

}

DNode* unlink_node(size_t *head,DList *List) 
{
    if(*head !=DNULL){
        size_t next = List->pool[*head].next;
        size_t temp = *head;
        dnode_free(&List->pool[*head].next,List);
        
        *head = next;
        return temp;
    } 
}


void produce_blah( DNode* node)
{
    node->data=1;
}

void calc_blah(DNode* node_a, DNode* node_b) 
{
    node_b->data=node_a->data+1;
}

void consume_blah(DNode* node)
{
    node->data=0;
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
