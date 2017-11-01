#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/types.h>
#include <unistd.h>

//https://stackoverflow.com/questions/33293121/how-to-allocate-linked-list-inside-struct-in-shared-memory-c
#define MAX_DLEN 50             // Max. number of list nodes
#define DNULL (MAX_DLEN + 1)    // NULL value
#define MAX_ITER 500            // Number of times each process will run

typedef struct DNode DNode;
typedef struct DList DList;

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
        if (List->npool < MAX_DLEN){
            DNode *node_test = &List->pool[List->npool++];
            return node_test; 
        }
    }
    DNode* test =NULL;

    return test;
}


DNode *dnode_push(DList*List,int num)
{
    size_t *head =&List->head;
    DNode * node = dnode_alloc(List);

    if(node){
        node->data=num;
        node->next=*head;
        *head = node - List->pool;
    }
    return node;
}


void init_list(DList*List,int size){
    List->head=DNULL;
    List->pfree=DNULL;
    List->npool=0;
    int i=0;
    for(i=0;i<size;i++){
        dnode_push(List,0);     
    }
}

void dnode_free(DNode *node, DList* List){
    if(node){
        node->next=List->pfree;
        List->pfree=node-List->pool;
    }
}

DNode *dnode(size_t index,DList* List)
{
    return (index==DNULL)?NULL : List->pool + index;
}
DNode *dnode_next(const DNode* node, DList* List)
{
    return dnode(node->next, List);
}

void link_node ( DNode* node , DList *New_List)
{
    size_t*head=&New_List->head;
    DNode *temp = dnode_alloc(New_List);
    temp->data=node->data;
    temp->next=*head;
    *head = temp - New_List->pool;
    
    printf("end of link\n");
}

DNode* unlink_node(DList *List) 
{

    size_t * head = &List->head;
    if(*head !=DNULL){
        size_t next = List->pool[*head].next;
        DNode* temp = &List->pool[*head];
        dnode_free(&List->pool[*head],List);
        
        *head = next;
        printf("end of unlink in if\n");
        return temp;
    }
    printf("end of unlink out of if\n");
    printf("NULLLLLLLIIIIINNNNUNLINK\n");
    return NULL; 
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
    sem_t* MxFL = sem_open ("Mutex_Free_List", O_CREAT ); 
    sem_t* MxL1 = sem_open ("Mutex_List1", O_CREAT ); 
    sem_t* MxL2 = sem_open ("Mutex_List2", O_CREAT); 
    sem_t* SCFL = sem_open ("Count_Free_List", O_CREAT ); 
    sem_t* SCL1 = sem_open ("Count_List1", O_CREAT ); 
    sem_t* SCL2 = sem_open ("Count_List2", O_CREAT ); 
    sem_t* LastSem = sem_open ("Last_Sem", O_CREAT );


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
    sem_t* MxFL = sem_open ("Mutex_Free_List", O_CREAT ); 
    sem_t* MxL1 = sem_open ("Mutex_List1", O_CREAT ); 
    sem_t* MxL2 = sem_open ("Mutex_List2", O_CREAT); 
    sem_t* SCFL = sem_open ("Count_Free_List", O_CREAT ); 
    sem_t* SCL1 = sem_open ("Count_List1", O_CREAT ); 
    sem_t* SCL2 = sem_open ("Count_List2", O_CREAT ); 
    sem_t* LastSem = sem_open ("Last_Sem", O_CREAT );


     DNode* b_node;
    int i=0;
    //we could make this while(1) but I want the program to end.
    while(1){
        printf("produce proccess: %d\n",i);
        sem_wait(LastSem);
        sem_wait(SCFL);
        sem_wait(MxFL);  
        b_node = unlink_node(FreeList);    
        sem_post(MxFL);
        produce_blah(b_node);
        sem_wait(MxL1);
        link_node(b_node,List1);
        sem_post(MxL1);
        sem_post(SCL1);
        sem_post(LastSem);
        i++;
    } 

}
void calc_pro(){
    DNode *x_node,*y_node;
    sem_t* MxFL = sem_open ("Mutex_Free_List", O_CREAT ); 
    sem_t* MxL1 = sem_open ("Mutex_List1", O_CREAT ); 
    sem_t* MxL2 = sem_open ("Mutex_List2", O_CREAT); 
    sem_t* SCFL = sem_open ("Count_Free_List", O_CREAT ); 
    sem_t* SCL1 = sem_open ("Count_List1", O_CREAT ); 
    sem_t* SCL2 = sem_open ("Count_List2", O_CREAT ); 
    sem_t* LastSem = sem_open ("Last_Sem", O_CREAT );


    int i=0;
    while(1){
        printf("calc process %d \n",i);
        sem_wait(SCL1);
        sem_wait(MxL1);
        x_node=unlink_node(List1);
        sem_post(MxL1);
        sem_wait(SCFL);
        sem_wait(MxFL);
        y_node=unlink_node(FreeList);
        sem_post(MxFL);
        calc_blah(x_node,y_node);
        sem_wait(MxFL);
        link_node(x_node,FreeList);
        sem_post(MxFL);
        sem_post(SCFL);
        sem_post(LastSem);
        sem_wait(MxL2);
        link_node(y_node,List2);
        sem_post(MxL2);
        sem_post(SCL2);
        i++;
    }
}
void consume_pro(){
    sem_t* MxFL = sem_open ("Mutex_Free_List", O_CREAT ); 
    sem_t* MxL1 = sem_open ("Mutex_List1", O_CREAT ); 
    sem_t* MxL2 = sem_open ("Mutex_List2", O_CREAT); 
    sem_t* SCFL = sem_open ("Count_Free_List", O_CREAT ); 
    sem_t* SCL1 = sem_open ("Count_List1", O_CREAT ); 
    sem_t* SCL2 = sem_open ("Count_List2", O_CREAT ); 
    sem_t* LastSem = sem_open ("Last_Sem", O_CREAT );


    DNode *c_node;
    int i=0;
    while(1){
        printf("consume process: %d\n",i);
        sem_wait(SCL2);
        sem_wait(MxL2);
        c_node = unlink_node(List2);
        sem_post(MxL2);
        consume_blah(c_node);
        sem_wait(MxFL);
        link_node(c_node, FreeList);
        sem_post(MxFL);
        sem_post(SCFL);
        sem_post(LastSem);
        i++;
    }
}
int main (void)
{
    int n = MAX_DLEN;
    int i =0;
    pid_t pid,pro_id,calc_id,con_id;    

    sem_t *MxFL, *MxL1, *MxL2, *SCFL,*SCL1,*SCL2,*LastSem;
    MxFL = sem_open ("Mutex_Free_List", O_CREAT , 0777, 1); 
    MxL1 = sem_open ("Mutex_List1", O_CREAT , 0777, 1); 
    MxL2 = sem_open ("Mutex_List2", O_CREAT,  0777, 1); 
    SCFL = sem_open ("Count_Free_List", O_CREAT , 0777, n); 
    SCL1 = sem_open ("Count_List1", O_CREAT , 0777, 0); 
    SCL2 = sem_open ("Count_List2", O_CREAT , 0777, 0); 
    LastSem = sem_open ("Last_Sem", O_CREAT , 0777, n-1);

    FreeList=create_shared_memory(sizeof(DList));
    init_list(FreeList,n);
    List1=create_shared_memory(sizeof(DList)); 
    init_list(List1,0);
    List2=create_shared_memory(sizeof(DList)); 
    init_list(List2,0);
    
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
                    munmap(FreeList, sizeof(DList));
                    munmap(List1, sizeof(DList));
                    munmap(List2, sizeof(DList));
                    //free semaphores
                    free_sem();
                    exit(0); 
                }

            }
        }

    }
    return 0;
}
