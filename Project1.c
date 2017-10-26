#include<stdio.h>
#include<stdlib.h>
#include <sys/shm.h>
#include <semaphore.h>

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

struct Node* unlink(struct Node** head_ref)
{
    struct Node* temp = (*head_ref);
    
}

void link (struct Node** head_ref, struct Node *a)
{

}

void produce_blah(struct Node* a)
{

}

void calc_blah(struct Node* a, struct Node* b)
{

}

void consume_blah(stuct Node* c)
{

}
