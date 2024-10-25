#include "memory_manager.h"

void* memoryStart;
memoryBlock* head;

size_t memorySize;
size_t  usedMemory;
int totalNrOfBlocks;

pthread_mutexattr_t recursiveMutexAttributes;
pthread_mutex_t lock1;

void mem_init(size_t size) //initialize memory of a certain size and storing the pointer to it in memory
{
    memoryStart = malloc(size);
    memoryBlock* newBlock = malloc(sizeof(memoryBlock));

    pthread_mutexattr_init(&recursiveMutexAttributes);
    pthread_mutexattr_settype(&recursiveMutexAttributes, PTHREAD_MUTEX_RECURSIVE);
    pthread_mutex_init(&lock1, &recursiveMutexAttributes);

    newBlock->start = memoryStart;
    newBlock->next = NULL;
    newBlock->occupied = false;
    newBlock->blockSize = size;

    head = newBlock;
    totalNrOfBlocks = 1;
    usedMemory = 0;
    memorySize = size;
}



void* mem_alloc(size_t size)
{
    pthread_mutex_lock(&lock1);
    
    if(usedMemory + size > memorySize)
    {
        pthread_mutex_unlock(&lock1);
        return NULL;
    }

    if(totalNrOfBlocks >= 2 && usedMemory == 0) //merging when memory is split up but empty
    {
        memoryBlock* walker = head->next;
        while(walker != NULL)
        {
            memoryBlock* toDelete = walker;
            walker = walker->next;
            free(toDelete);
        }

        head->next = NULL;
        head->occupied = false;
        head->blockSize = memorySize;
        totalNrOfBlocks = 1;
    }

    if(!head->occupied && head->blockSize == size)
    {
        //pthread_mutex_lock(&lock1);
        head->occupied = true;
        usedMemory += size;
        
        pthread_mutex_unlock(&lock1);

        return head->start;
    }

    if(!head->occupied && head->blockSize >= size)
    {
        memoryBlock* newBlock = malloc(sizeof(memoryBlock));

        newBlock->next = head->next;
        head->next = newBlock;
        head->blockSize = size;
        head->occupied = true;

        newBlock->start = (char*)head->start + size;
        newBlock->occupied = false;
        newBlock->blockSize = memorySize - size;
        //pthread_mutex_unlock(&lock1);

        usedMemory += size;
        totalNrOfBlocks++;

        pthread_mutex_unlock(&lock1);

        return head->start;
    }

    //pthread_mutex_lock(&lock1);
    memoryBlock* walker = head;

    while(walker != NULL && (walker->occupied || walker->blockSize < size))
    {
        walker = walker->next;
    }

    //pthread_mutex_unlock(&lock1);
    if(walker == NULL)
    {
        pthread_mutex_unlock(&lock1);

        return NULL;
    }

    if(walker->blockSize == size)
    {
        walker->occupied = true;
        usedMemory += size;
        pthread_mutex_unlock(&lock1);

        return walker->start;
    }

    memoryBlock* newBlock = malloc(sizeof(memoryBlock));

    size_t theBlockSize = walker->blockSize;

    newBlock->next = walker->next;
    walker->next = newBlock;
    walker->occupied = true;
    walker->blockSize = size;

    newBlock->start = (char*)walker->start + size;
    newBlock->occupied = false;
    newBlock->blockSize = theBlockSize - size;

    usedMemory += size;
    totalNrOfBlocks++;

    pthread_mutex_unlock(&lock1);
    return walker->start;
}

void mem_free(void* block) //free a memory block by finding the block and marking it as not occupied
{
    pthread_mutex_lock(&lock1);
    if(head == NULL)
    {
        pthread_mutex_unlock(&lock1);
        return;
    }

    if(head->start == block)
    {
        head->occupied = false;
        usedMemory -= head->blockSize;
    }

    else
    {
        memoryBlock* walker = head;

        while(walker->next != NULL && walker->next->start != block)
        {
            walker = walker->next;
        }

        if(walker->next == NULL)
        {
            pthread_mutex_unlock(&lock1);
            return;
        }

        if(walker->next->start == block)
        {
            walker->next->occupied = false;
            usedMemory -= walker->next->blockSize;
        }
    }
    
    pthread_mutex_unlock(&lock1);
}

void* mem_resize(void* block, size_t size) //set a new size for a block
{
    pthread_mutex_lock(&lock1);
    memoryBlock* resizedBlock = mem_alloc(size);

    memcpy(resizedBlock, block, size); //copy the data to the new place

    mem_free(block);
    pthread_mutex_unlock(&lock1);

    return resizedBlock;
}

void mem_deinit() //freeing the pointers of the memory-pointer and the memoryBlocks-pointer and makes the memory available for other usage
{
    if(totalNrOfBlocks < 1) return;

    if(totalNrOfBlocks == 1) free(head);

    else
    {
        memoryBlock* walker = head;

        while(walker != NULL)
        {
            memoryBlock* toDelete = walker;

            walker = walker->next;
            free(toDelete);
        }
    }
    
    free(memoryStart);
    totalNrOfBlocks = 0;
    usedMemory = 0;
    head = NULL;
    pthread_mutex_destroy(&lock1);
}