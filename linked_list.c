#include "linked_list.h"

int nrOfNodes = 0;
pthread_rwlock_t readWriteLock;

void list_init(Node** head, size_t size) //initialize a list with specified memory capacity
{
    *head = NULL;
    pthread_rwlock_init(&readWriteLock, NULL);

    mem_init(size); //initialize memory
    nrOfNodes = 0;
}

void list_insert(Node** head, uint16_t data) //insert a new node into the very back of the list
{
    pthread_rwlock_wrlock(&readWriteLock);
    Node* newNode = mem_alloc(sizeof(Node)); //creating new node
    newNode->next = NULL;
    newNode->data = data;

    if(*head == NULL) //if there are no nodes, head will point to new node
    {
        *head = newNode;
    }

    else
    {
        Node* walkNode = *head;
        while(walkNode->next != NULL) //looping to find back of the list
        {
            walkNode = walkNode->next;
        }

        walkNode->next = newNode; //inserting newNode at the back
    }

    nrOfNodes++;
    pthread_rwlock_unlock(&readWriteLock);
}

void list_insert_after(Node* prev_node, uint16_t data) //insert a new node after a specified node
{
    pthread_rwlock_wrlock(&readWriteLock);
    if(nrOfNodes == 0)
    {
        pthread_rwlock_unlock(&readWriteLock);
        return;
    }

    Node* newNode = (Node*)mem_alloc(sizeof(Node)); //creating a new node

    newNode->data = data;
    newNode->next = prev_node->next;
    prev_node->next = newNode;
    nrOfNodes++;
    pthread_rwlock_unlock(&readWriteLock);
}

void list_insert_before(Node** head, Node* next_node, uint16_t data) //insert a new node directly before a specified node
{
    pthread_rwlock_wrlock(&readWriteLock);
    Node* newNode = (Node*)mem_alloc(sizeof(Node)); //creating new node
    newNode->data = data;

    if(*head == NULL) //if list is empty, we can't put it in front of anything
    {
        pthread_rwlock_unlock(&readWriteLock);
        return;
    }
    
    if(next_node == *head) //if next_node is head, the newNode becomes the new head
    {
        newNode->next = *head;
        *head = newNode;
        nrOfNodes++;

        pthread_rwlock_unlock(&readWriteLock);
        return;
    }
    
    Node* walkNode = *head;

    while(walkNode->next != NULL && walkNode->next != next_node) //finding the node we want to insert in front of
    {
        walkNode = walkNode->next;
    }

    if(walkNode->next == NULL)
    {
        pthread_rwlock_unlock(&readWriteLock);
        return;
    }

    newNode->next = walkNode->next;
    walkNode->next = newNode;
    nrOfNodes++;

    pthread_rwlock_unlock(&readWriteLock);
}

void list_delete(Node** head, uint16_t data) //deleting the first node found with specified data
{
    pthread_rwlock_wrlock(&readWriteLock);
    if(nrOfNodes == 0 || (nrOfNodes == 1 && ((*head)->data != data)))
    {
        pthread_rwlock_unlock(&readWriteLock);
        return;
    }

    if((*head)->data == data) //if data is in the head, we need to move the head to its next node
    {
        Node* toDelete = *head;
        *head = toDelete->next;
        mem_free(toDelete);
        nrOfNodes--;

        pthread_rwlock_unlock(&readWriteLock);
        return;
    }

    Node* walkNode = *head;
    
    while(walkNode->next != NULL && (walkNode->next)->data != data) //looping to the specified node
    {
        walkNode = walkNode->next;
    }

    if((walkNode->next)->data == data)
    {
        Node* toDelete = walkNode->next;
        walkNode->next = toDelete->next;
        mem_free(toDelete);
        nrOfNodes--;
    }

    pthread_rwlock_unlock(&readWriteLock);
}

Node* list_search(Node** head, uint16_t data) //searches the list for a specified node and returning a pointer to where it is stored in the memory
{
    pthread_rwlock_rdlock(&readWriteLock);
    Node* walkNode = *head;

    if(walkNode->data == data)
    {
        pthread_rwlock_unlock(&readWriteLock);
        return walkNode;
    }

    while(walkNode->next != NULL && (walkNode->next)->data != data) //looping to find the right node
    {
        pthread_rwlock_unlock(&readWriteLock);
        walkNode = walkNode->next;
    }

    if(walkNode->next == NULL) //return if node isn't found
    {
        pthread_rwlock_unlock(&readWriteLock);
        return NULL;
    }

    if((walkNode->next)->data == data)
    {
        pthread_rwlock_unlock(&readWriteLock);
        return walkNode->next;
    }

    pthread_rwlock_unlock(&readWriteLock);
    return NULL;
}

void list_display(Node** head) //prints all the data of the list
{
    pthread_rwlock_rdlock(&readWriteLock);

    Node* walkNode = *head;
    printf("[");

    while(walkNode->next != NULL) //looping through list and printing data
    {
        printf("%d", walkNode->data);
        printf(", ");
        walkNode = walkNode->next;
    }
    printf("%d", walkNode->data);
    printf("]");

    pthread_rwlock_unlock(&readWriteLock);
}

void list_display_range(Node** head, Node* start_node, Node* end_node) //prints data of the list in a range between two specified nodes
{
    pthread_rwlock_rdlock(&readWriteLock);

    if(start_node == NULL)
    {
        start_node = *head;
    }

    if(end_node == NULL) {
        list_display(&start_node); //if end is null, we just want to loop to the end

        pthread_rwlock_unlock(&readWriteLock);
        return;
    }

    Node* walkNode = start_node;
    printf("[");

    while(walkNode->next != NULL && walkNode != end_node) //looping through list and printing data
    {
        printf("%d", walkNode->data);
        printf(", ");
        walkNode = walkNode->next;
    }

    printf("%d", walkNode->data);
    printf("]");

    pthread_rwlock_unlock(&readWriteLock);
}

int list_count_nodes(Node** head) //counts all nodes in the list
{
    pthread_rwlock_rdlock(&readWriteLock);

    Node* walkNode = *head;

    if(*head == NULL) 
    {
        pthread_rwlock_unlock(&readWriteLock);
        return 0;
    };

    int counter = 1;
    while(walkNode->next != NULL) //loop to count nodes
    {
        walkNode = walkNode->next;
        counter++;
    }

    pthread_rwlock_unlock(&readWriteLock);
    return counter;
    //return nrOfNodes;
}

void list_cleanup(Node** head) //frees the memory of each node and de-initializes the list
{
    pthread_rwlock_wrlock(&readWriteLock); //this deadlocks the program at one point

    if(*head != NULL)
    {
        Node* walkNode = *head;
        while(walkNode->next != NULL) //freeing each node
        {
            mem_free(walkNode);
            walkNode = walkNode->next;
            nrOfNodes--;
        }
    }

    *head = NULL;
    mem_deinit(); //de-initialize

    pthread_rwlock_unlock(&readWriteLock);
    pthread_rwlock_destroy(&readWriteLock);
}
